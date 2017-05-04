#include "rtl_utils.h"  // used for memDump
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include "hsModel.h"
#include "rtl8672_asicregs.h"
#include "rtl8672_Proto.h"
#include "icModel_Packet.h"
#include "icModel_ringController.h"
#include "icTest_ringController.h"
#include "icModel_pktParser.h"
#include "icTest_ALE.h"
#include "rtl8672_tblAsicDrv.h"
#include "icTest.h"
#include "virtualMac.h"
#include "modelTrace.h"
#include "rtl8672_tblDrv.h"
#include "rtl8672PacketProcessor.h"
#include <linux/delay.h>
#include "rtl8672pp_extsw.h"
#include "../../867x_sar/ra867x_pp.h"
#include "../re830x.h"  //xl_yue:20100204
#ifdef CONFIG_USB_RTL8192SU_SOFTAP//CONFIG_RTL8185
#define __WLAN_ATTRIB_PACK__		__attribute__ ((packed))
#define __PACK
#ifndef _TYPE_DEF
#define _TYPE_DEF
typedef unsigned char	UINT8;
typedef unsigned short	UINT16;
typedef unsigned long	UINT32;

typedef signed char		INT8;
typedef signed short	INT16;
typedef signed long		INT32;

typedef unsigned int	UINT;
typedef signed int		INT;

typedef unsigned long long	UINT64;
typedef signed long long	INT64;
#endif
//#include "../../../wireless/rtl8185/wifi.h"
#endif
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
#include "../../usb/net/8192su/wifi.h"
#endif
static int rtl8139rx_drop=0;
// Tx to ext device 
struct net_device *netdev_for8672_tx[3]={NULL,NULL,NULL};
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
struct tasklet_struct ppsar_rx_tasklets;
#endif
#ifdef CONFIG_RTL8681_PTM
extern struct tasklet_struct ppptm_rx_tasklets;
#endif
unsigned int tmpmacisr=0;
#define MAC_USE_TASKLET
#ifdef MAC_USE_TASKLET
struct tasklet_struct ppmac_rx_tasklets;
void ppmac_rx_bh(unsigned long cp);
#endif

#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
//struct sar_private sarcp;
void ppsar_rx_bh(unsigned long cp);
#endif
#ifdef CONFIG_RTL8681_PTM
void ppptm_rx_bh(unsigned long cp);
#endif

static inline void mac_l2learning(struct sk_buff *skb,int intfidx,int vlanid);

#include <linux/netdevice.h>
#include <linux/interrupt.h>
#include <linux/etherdevice.h>
#include <asm/io.h> /* for dma_cache_wback_inv */
#ifdef CONFIG_RTK_VOIP
#define EXT_PHY
#endif
struct ext_private {
	int rsv;
};

struct sar_private {
	int rsv;
};

struct net_device *ext_dev,*mac_dev[2];
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
struct net_device sarPP_dev;
#endif
#ifdef CONFIG_RTL8681_PTM
struct net_device *ptmPP_dev;
#endif

//int first_time=TRUE;


#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
#define SAR_FIRST
#endif
/* Special patch to make SAR->NIC using Hi pirority queue (based on MAC) */



int cpu_ext_vrx_idx[EXT_INTFS]={0};
int cpu_ext_ptx_idx[EXT_INTFS]={0};
int cpu_ext_vtx_idx[EXT_INTFS]={0};
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
int cpu_sar_vrx_idx[SAR_INTFS]={0};
#endif
#ifdef CONFIG_RTL8681_PTM
int cpu_ptm_vrx_idx[PTM_INTFS]={0};
#endif
int cpu_mac_vrx_idx[MAC_RX_INTFS]={0};
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
int cpu_sar_vtx_idx[SAR_INTFS]={0};
int cpu_sar_vtx_finish_idx[SAR_INTFS]={0}; // record the index of finished sar vtx 
#endif
#ifdef CONFIG_RTL8681_PTM
int cpu_ptm_vtx_idx[PTM_INTFS]={0};
int cpu_ptm_vtx_finish_idx[PTM_INTFS]={0}; // record the index of finished ptm vtx 
#endif
int cpu_mac_vtx_idx[MAC_TX_INTFS]={0};
uint32 cpu_mac_vtx_skb[MAC_TX_INTFS][MAC_VTX_RING_SIZE]; // for vtx resource recycle
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
uint32 cpu_sar_vtx_skb[SAR_INTFS][SAR_VTX_RING_SIZE]; // for vtx resource recycle
#endif
#ifdef CONFIG_RTL8681_PTM
uint32 cpu_ptm_vtx_skb[PTM_INTFS][PTM_VTX_RING_SIZE]; // for vtx resource recycle
#endif

//struct sk_buff *last_skb;
#define NEW_ARCH

#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
#define MAX_SRAM_MAP_ID ((MAC_PTX_RING0_SIZE+MAC_PTX_RING1_SIZE+MAC_PTX_RING2_SIZE+MAC_PTX_RING3_SIZE+MAC_PTX_RING4_SIZE)  /* share pool to mac */  \
						+SAR_INTFS*SAR_PTX_RING_SIZE  /* share pool to sar ptx */ \
						+EXT_PTX_RING0_SIZE+EXT_PTX_RING1_SIZE+EXT_PTX_RING2_SIZE  /* share pool to ext ptx  */ \
						+MAC_RX_INTFS*MAC_PRX_RING_SIZE /* mac prx */ \
						+SAR_INTFS*SAR_PRX_RING_SIZE) /* sar prx */ \

#define MAX_SKB_ID		(MAC_PTX_RING0_SIZE+MAC_PTX_RING1_SIZE+MAC_PTX_RING2_SIZE+MAC_PTX_RING3_SIZE+MAC_PTX_RING4_SIZE \
						+SAR_INTFS*SAR_PTX_RING_SIZE \
						+EXT_PTX_RING0_SIZE+EXT_PTX_RING1_SIZE+EXT_PTX_RING2_SIZE \
						+MAC_RX_INTFS*(MAC_PRX_RING_SIZE+MAC_VRX_RING_SIZE) \
						+SAR_INTFS*(SAR_PRX_RING_SIZE+SAR_VRX_RING_SIZE) \
						+EXT_INTFS*(EXT_PRX_RING_SIZE+EXT_VRX_RING_SIZE)) \

#endif

#ifdef CONFIG_RTL8681_PTM
#define MAX_SRAM_MAP_ID ((MAC_PTX_RING0_SIZE+MAC_PTX_RING1_SIZE+MAC_PTX_RING2_SIZE+MAC_PTX_RING3_SIZE+MAC_PTX_RING4_SIZE)  /* share pool to mac */  \
						+(PTM_INTFS-1)*PTM_PTX_RINGx_SIZE + PTM_PTX_RING0_SIZE/* share pool to ptm ptx */ \
						+EXT_PTX_RING0_SIZE+EXT_PTX_RING1_SIZE+EXT_PTX_RING2_SIZE  /* share pool to ext ptx  */ \
						+MAC_RX_INTFS*MAC_PRX_RING_SIZE /* mac prx */ \
						+(PTM_INTFS-1)*PTM_PRX_RINGx_SIZE + PTM_PRX_RING0_SIZE) /* ptm prx */ \

#define MAX_SKB_ID		(MAC_PTX_RING0_SIZE+MAC_PTX_RING1_SIZE+MAC_PTX_RING2_SIZE+MAC_PTX_RING3_SIZE+MAC_PTX_RING4_SIZE \
						+(PTM_INTFS-1)*PTM_PTX_RINGx_SIZE + PTM_PTX_RING0_SIZE \
						+EXT_PTX_RING0_SIZE+EXT_PTX_RING1_SIZE+EXT_PTX_RING2_SIZE \
						+MAC_RX_INTFS*(MAC_PRX_RING_SIZE+MAC_VRX_RING_SIZE) \
						+(PTM_INTFS-1)*PTM_PRX_RINGx_SIZE+ PTM_PRX_RING0_SIZE +(PTM_INTFS*PTM_VRX_RING_SIZE) \
						+EXT_INTFS*(EXT_PRX_RING_SIZE+EXT_VRX_RING_SIZE)) \

#endif

#define MAX_SKB_POOL	(MAX_SKB_ID+128*2+4)

struct sk_buff *skbs[MAX_SKB_ID]={0};

// all descriptors


struct mac_pRx *user_mac_prx=NULL;
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
struct sar_pRx *user_sar_prx=NULL;
#endif
#ifdef CONFIG_RTL8681_PTM
struct mac_pRx *user_ptm_prx[PTM_INTFS]={0};
#endif
struct ext_Rx *user_ext_prx=NULL;


struct mac_pTx *user_mac_ptxs[MAC_TX_INTFS]={0};
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
struct sar_pTx *user_sar_ptx=NULL;
#endif
#ifdef CONFIG_RTL8681_PTM
struct mac_pTx *user_ptm_ptx[PTM_INTFS]={0};
#endif
struct ext_Tx *user_ext_ptxs[EXT_INTFS]={0};

struct mac_vRx *user_mac_vrx=NULL;
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
struct sar_vRx *user_sar_vrx=NULL;
#endif
#ifdef CONFIG_RTL8681_PTM
struct mac_vRx *user_ptm_vrx=NULL;
#endif
struct ext_Rx *user_ext_vrx=NULL;

struct mac_vTx *user_mac_vtx=NULL;
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
struct sar_vTx *user_sar_vtx=NULL;
#endif
#ifdef CONFIG_RTL8681_PTM
struct mac_vTx *user_ptm_vtx=NULL;
#endif
struct ext_Tx *user_ext_vtx=NULL;


struct sp_pRx *user_sp_to_macs[MAC_TX_INTFS]={0};
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
struct sp_pRx *user_sp_to_sar=NULL;
#endif
#ifdef CONFIG_RTL8681_PTM
struct sp_pRx *user_sp_to_ptm[PTM_INTFS]={0};
#endif
struct sp_pRx *user_sp_to_exts[EXT_INTFS]={0};

/* 0: means disable, 1: means enable*/
int pp_nic_enable = 0;   
int pp_ptm_enable = 0;

/*xl_yue:20100204 start: add for bug that eth2wifi blocking*/
static int eth2WifiBlockingFlag = 0;  /*1: block all; 2: block eth; 3: block wan*/
void ppSetEth2WifiBlockingFlag(int flag)
{
	eth2WifiBlockingFlag = (flag==2)?1:0;
}

static int isEthernetDomainMac(unsigned char *mac)
{
	int ret = 0;
	uint32 i,port=0,find=0;
	rtl8672_tblAsicDrv_intfParam_t rxintf;	
	rtl8672_tblAsicDrv_l2Param_t l2pt;

#ifdef NEW_ARCH
#ifdef EXT_PHY
	i=0;		
#else
	i=1;
#endif //EXT_PHY
#else
	for(i=0;i<2;i++)
#endif	
	{
 		rtl8672_getAsicNetInterface(i+SAR_INTFS, &rxintf);
		if(rtl8672_L2EntryLookup(mac,rxintf.PortVlanID,&l2pt)==SUCCESS){ 
			find = 1;
#ifndef NEW_ARCH
			break;
#endif
		}
	}
	if(find){
		port = l2pt.Port;
	}
#ifdef CONFIG_EXT_SWITCH
	else if(enable_port_mapping){
		for(i=0;i<SW_PORT_NUM;i++){
			if(rtl8672_L2EntryLookup(mac,rtl8305_info.vlan[rtl8305_info.phy[virt2phy[i]].vlanIndex].vid,&l2pt)==SUCCESS){ 
				port = l2pt.Port;
				break;
			}
		}	
	}
#endif

	if(port >= SAR_INTFS && port < (SAR_INTFS+MAC_TX_INTFS))
		ret = 1;

	return ret;
}
/*xl_yue:20100204 end*/


__inline__ struct mac_pRx *user_mac_prx_idx(int intf,int ring)
{
	return user_mac_prx+(MAC_PRX_RING_SIZE*intf)+ring;
}

#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
__inline__ struct sar_pRx *user_sar_prx_idx(int intf,int ring)
{
	return user_sar_prx+(SAR_PRX_RING_SIZE*intf)+ring;
}
#endif
#ifdef CONFIG_RTL8681_PTM
__inline__ struct mac_pRx *user_ptm_prx_idx(int intf,int ring)
{
	return user_ptm_prx[intf]+ring;
}
#endif


__inline__ struct ext_Rx *user_ext_prx_idx(int intf,int ring)
{
	return user_ext_prx+(EXT_PRX_RING_SIZE*intf)+ring;
}

__inline__ struct mac_pTx *user_mac_ptx_idx(int intf,int ring)
{
	return user_mac_ptxs[intf]+ring;
}

#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
__inline__ struct sar_pTx *user_sar_ptx_idx(int intf,int ring)
{
	return user_sar_ptx+(SAR_PTX_RING_SIZE*intf)+ring;
}
#endif
#ifdef CONFIG_RTL8681_PTM
__inline__ struct mac_pTx *user_ptm_ptx_idx(int intf,int ring)
{
	return user_ptm_ptx[intf]+ring;
}
#endif

__inline__ struct ext_Tx *user_ext_ptx_idx(int intf,int ring)
{
	return user_ext_ptxs[intf]+ring;
}

__inline__ struct mac_vRx *user_mac_vrx_idx(int intf,int ring)
{
	return user_mac_vrx+(MAC_VRX_RING_SIZE*intf)+ring;
}

#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
__inline__ struct sar_vRx *user_sar_vrx_idx(int intf,int ring)
{
	return user_sar_vrx+(SAR_VRX_RING_SIZE*intf)+ring;
}
#endif
#ifdef CONFIG_RTL8681_PTM
__inline__ struct mac_vRx *user_ptm_vrx_idx(int intf,int ring)
{
	return user_ptm_vrx+(PTM_VRX_RING_SIZE*intf)+ring;
}
#endif

__inline__ struct ext_Rx *user_ext_vrx_idx(int intf,int ring)
{
	return user_ext_vrx+(EXT_VRX_RING_SIZE*intf)+ring;
}

__inline__ struct mac_vTx *user_mac_vtx_idx(int intf,int ring)
{
	return user_mac_vtx+(MAC_VTX_RING_SIZE*intf)+ring;
}

#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
__inline__ struct sar_vTx *user_sar_vtx_idx(int intf,int ring)
{
	return user_sar_vtx+(SAR_VTX_RING_SIZE*intf)+ring;
}
#endif
#ifdef CONFIG_RTL8681_PTM
__inline__ struct mac_vTx *user_ptm_vtx_idx(int intf,int ring)
{
	return user_ptm_vtx+(PTM_VTX_RING_SIZE*intf)+ring;
}
#endif

__inline__ struct ext_Tx *user_ext_vtx_idx(int intf,int ring)
{
	return user_ext_vtx+(EXT_VTX_RING_SIZE*intf)+ring;
}

__inline__ struct sp_pRx *user_sp_to_mac_idx(int intf,int ring)
{
	return user_sp_to_macs[intf]+ring;
}

#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
__inline__ struct sp_pRx *user_sp_to_sar_idx(int intf,int ring)
{
	return user_sp_to_sar+(SAR_PTX_RING_SIZE*intf)+ring;
}
#endif
#ifdef CONFIG_RTL8681_PTM
__inline__ struct sp_pRx *user_sp_to_ptm_idx(int intf,int ring)
{
	return user_sp_to_ptm[intf]+ring;
}
#endif

__inline__ struct sp_pRx *user_sp_to_ext_idx(int intf,int ring)
{
	return user_sp_to_exts[intf]+ring;
}

#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
struct sar_vTx *user_current_sar_vtx[SAR_INTFS];
#endif
#ifdef CONFIG_RTL8681_PTM
struct mac_vTx *user_current_ptm_vtx[PTM_INTFS];
#endif
struct mac_vTx *user_current_mac_vtx[MAC_TX_INTFS];
struct ext_Tx *user_current_ext_vtx[EXT_INTFS];


#ifdef CONFIG_RTK_VOIP
extern void set_write_allocate( void );
#endif
int rtl8672_pp_set_mac_addr(struct net_device *dev, void *p)
{

	int i;
	rtl8672_tblAsicDrv_intfParam_t intf;
	struct sockaddr *addr = p;
	

	if (netif_running(dev))
		return -EBUSY;
	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);

#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	int ch_no;
	ch_no = index_dev2ch(dev);

	//sar device
	if(ch_no != -1){
		rtl8672_getAsicNetInterface(ch_no, &intf);
		memcpy(intf.GMAC, addr->sa_data, dev->addr_len);
		rtl8672_setAsicNetInterface(ch_no, &intf);
		return 0;
	}
#endif

	//mac device
	for(i=SAR_INTFS;i<(MAC_TX_INTFS+SAR_INTFS);i++)
	{
		rtl8672_getAsicNetInterface(i, &intf);
		memcpy(intf.GMAC, addr->sa_data, dev->addr_len);
		rtl8672_setAsicNetInterface(i, &intf);
	}
	return 0;
}

int init_ale(void)
{
	int i;
	//uint32 dmac_idx;
	//int32 retval;
	rtl8672_tblAsicDrv_l2Param_t l2p;

	/* ALE Control Registers */
	rtl8672_setAsicL2Ability(TRUE);	/* Layer2 Table Enable */
#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)  //0437 doesn't Support Layer 4
	rtl8672_setAsicL4Ability(TRUE);	/* Layer4 Table Enable */
#endif
	
	/* icTest Initialize */
	rtl8672_setAsicL2HashIdxBits(7);
	rtl8672_setAsicL2Way(4);
#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	rtl8672_setAsicL4HashIdxBits(7);
	rtl8672_setAsicL4Way(4);
#endif
	testALE_Init();

#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	WRITE_MEM32(ALECR_REASONACT,0x10800800);
#else
	WRITE_MEM32(ALECR_REASONACT,0x10000800);
#endif

	/* set each mac mapping to a unique port */
	bzero(&l2p, sizeof(rtl8672_tblAsicDrv_l2Param_t));
	l2p.MAC[0] = 0x0;
	l2p.MAC[1] = 0x0;
	l2p.MAC[2] = 0x0;
	l2p.MAC[3] = 0x0;
	l2p.MAC[4] = 0x1;
	
	l2p.APMAC[0] = 0x0;
	l2p.APMAC[1] = 0x0;
	l2p.APMAC[2] = 0x0;
	l2p.APMAC[3] = 0x0;
//	l2p.APMAC[4] = 0x2;
	l2p.APMAC[4] = 0x0;
	l2p.APMAC[5] = 0x0;
	
	l2p.VlanID=8;
	i=0;
#if 0
	for(i=0;i<SAR_INTFS+MAC_TX_INTFS+EXT_INTFS;i++)
	{
		l2p.MAC[5] = i;
//		l2p.APMAC[5] = i;
		l2p.Port=i;
		l2p.Valid=1;
		dmac_idx = rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID);
//		memDump((void *)L2TABLE_BASE + (dmac_idx * ALE_L2TABLE_WIDTH * ALE_L2TABLE_SRAM_WAY), ALE_L2TABLE_WIDTH * ALE_L2TABLE_SRAM_WAY, "L2 Table Before");
		rtl8672_setAsicL2Table_Sram(dmac_idx, 3, &l2p);
//		memDump((void *)L2TABLE_BASE + (dmac_idx * ALE_L2TABLE_WIDTH * ALE_L2TABLE_SRAM_WAY), ALE_L2TABLE_WIDTH * ALE_L2TABLE_SRAM_WAY, "L2 Table After");
	}
retval = 0;
#endif
	return SUCCESS;

}

int sram_map_offset[SAR_INTFS+MAC_TX_INTFS+EXT_INTFS]={0};

#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
extern int sendToPort0;
int set_sar_intf(int channel, int mode, int encap){
	rtl8672_tblAsicDrv_intfParam_t outintf;
	rtl8672_getAsicNetInterface(channel, &outintf);
	printk("set ch:%d  mode:%d  %s\n",channel,mode,encap?"LLC":"VCMUX");
	outintf.SARhdr = mode;
	outintf.L2Encap=encap;
	if(sendToPort0)
		outintf.ATMPORT=1;
	else
		outintf.ATMPORT=0;
	//outintf.RXshift=DMA_SHIFT;
	rtl8672_setAsicNetInterface(channel, &outintf);

	return 0;
}
int init_sar_ring(void)
{
	int i,j;
	rtl8672_tblAsicDrv_intfParam_t outintf;	

	for(i=0;i<SAR_INTFS;i++)
	{
		rtl8672_getAsicNetInterface(i, &outintf);
		outintf.SARhdr = BRIDGED;
		outintf.L2Encap=LLC_ENCAPSULATION;
		outintf.RXshift=DMA_SHIFT;
		sram_map_offset[i]=96;
		outintf.SrcPortFilter=0;
		rtl8672_setAsicNetInterface(i, &outintf);		

		for(j=0;j<SAR_PRX_RING_SIZE;j++)
		{
			struct sk_buff *skb;
			struct sar_pRx *sarRx;
			sarRx=user_sar_prx_idx(i,j);
			sarRx->rx_shift=DMA_SHIFT;
			sarRx->sram_map_of=96;
			skb=(struct sk_buff *)sarRx->skb_header_addr;
			skb->data=skb->head+DMA_SHIFT;
			skb->tail=skb->data;
		}

		for(j=0;j<SAR_VRX_RING_SIZE;j++)
		{
			struct sk_buff *skb;
			struct sar_vRx *sarRx;
			sarRx=user_sar_vrx_idx(i,j);
			sarRx->rx_shift=DMA_SHIFT;
			skb=(struct sk_buff *)sarRx->skb_header_addr;
			skb->data=skb->head+DMA_SHIFT;
			skb->tail=skb->data;			
		}
		
	}
	return SUCCESS;
}
#endif 	//end RTL8672_SAR


struct free_skb_list
{
	struct sk_buff *skb;	
	struct free_skb_list *next;
#ifdef SKB_POOL_DEBUG		
	char free_func[32];
	int free_line;
#endif	
} free_head;

//int insert_skb_pool_lock=0;
int get_free_skb_lock=0;
volatile unsigned int lock;
#define SAVE_INT_AND_CLI(x)		spin_lock_irqsave(lock, x)
#define RESTORE_INT(x)			spin_unlock_irqrestore(lock, x)

#if 0
__IRAM_PP_HIGH void init_skb(struct sk_buff *skb,int allocsize)
{
	struct skb_shared_info *shinfo;
	skb->head=(unsigned char *)((uint32)skb->head | 0xa0000000);
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
	skb->data=skb->head;
	skb->tail=skb->data;
	skb->end=(unsigned char *)((uint32)skb->head+allocsize+NET_SKB_PAD);
#else
	skb->end=(unsigned char *)((uint32)skb->head+BUFFER_SIZE-sizeof(struct skb_shared_info));
#endif
	atomic_set(&(skb_shinfo(skb)->dataref), 1);
	atomic_set(&(skb->users), 1);	
	shinfo=(struct skb_shared_info *)skb->end;
	shinfo->nr_frags=0;
	shinfo->frag_list=NULL;
	shinfo->gso_size = 0;
	shinfo->gso_segs = 0;
	shinfo->gso_type = 0;
	shinfo->ip6_frag_id = 0;
#ifndef CONFIG_USB_RTL8192SU_SOFTAP
	skb->data=skb->head;
	skb->tail=skb->head;
#endif
	skb->cloned = 0;
	skb->len=0;
	skb->data_len=0;
	skb->next=NULL;
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
	skb->protocol=0;
	skb->fcpu=0;
#endif
	skb->pptx=0;
}


#define __DRAM_8672		__attribute__ ((section(".dram")))
//#define __DRAM_8672		

__DRAM_8672 int skb_pool_alloc_index=0;
__DRAM_8672 int skb_pool_free_index=0;
__DRAM_8672 int free_cnt=0;
__DRAM_8672 struct sk_buff *skb_pool_ring[MAX_SKB_POOL];

__IRAM_PP_HIGH int insert_skb_pool(struct sk_buff *skb) 
{
	unsigned long flags;
	SAVE_INT_AND_CLI(flags);

#if 0 //verify only
{
	int i;
	int end;
//	printk("free skb=%x idx=%d skb_pool_alloc_index=%d\n",(u32)skb,skb_pool_free_index,skb_pool_alloc_index);
	if(skb_pool_free_index<skb_pool_alloc_index)
		end=skb_pool_free_index+MAX_SKB_POOL;
	else
		end=skb_pool_free_index;
	for(i=skb_pool_alloc_index;i<end;i++)
	{
		if((u32)skb_pool_ring[i%MAX_SKB_POOL]==(u32)skb)
		{
			printk("skb=%x already in pool, idx=%d(%d~%d)\n",(u32)skb, i,skb_pool_alloc_index,end);
			memDump(skb->data,128,"skb data");
			BUG();
		}
	}
}

	if (skb == 0) {
		printk("%s(%d)\n",__FUNCTION__, __LINE__);
		while(1);
	}

	/*if (skb->pp_use!=1) {
		printk("%s(%d)\n",__FUNCTION__, __LINE__);
		while(1);
	}*/
#endif

	//boundary check
	if(skb_pool_free_index==MAX_SKB_POOL-1)
	{
		if(skb_pool_alloc_index==0)
		{
			goto error;
		}
	}
	else
	{
		if(skb_pool_free_index+1==skb_pool_alloc_index)
		{	
			goto error;
		}		
	}
		
	init_skb(skb,BUFFER_SIZE);
	
	skb_pool_ring[skb_pool_free_index]=skb;
	
	++free_cnt;	

	if(skb_pool_free_index==MAX_SKB_POOL-1)
		skb_pool_free_index=0;
	else
		++skb_pool_free_index;
	
	RESTORE_INT(flags);
	if(free_cnt>=MAX_SKB_POOL) {goto error;}
	return SUCCESS;
error:
	printk("skb ring full, free cnt=%d, free=%d, alloc=%d\n",free_cnt, 
		skb_pool_free_index,skb_pool_alloc_index);
	while(1);
	
}


__IRAM_PP_HIGH struct sk_buff *get_free_skb(void)
{
	struct sk_buff *ret;
	unsigned long	flags;
	if(free_cnt<=0) {printk("free_cnt=%d\n",free_cnt); BUG(); }
	
	SAVE_INT_AND_CLI(flags);
	ret=skb_pool_ring[skb_pool_alloc_index];

//	printk("alloc skb=%x idx=%d free_cnt=%d\n",(u32)ret,skb_pool_alloc_index,free_cnt);

	if(skb_pool_alloc_index==MAX_SKB_POOL-1)
		skb_pool_alloc_index=0;
	else
		++skb_pool_alloc_index;
		--free_cnt;	
	RESTORE_INT(flags);

	return ret;
}

static int print_l2entry(void *ptr, void *user_data) {
	rtl8672_tblAsicDrv_l2Param_t *l2pt = ptr;
	printk("%02x%02x.%02x%02x.%02x%02x port%d vlan%d\n", l2pt->MAC[0],l2pt->MAC[1],l2pt->MAC[2],
		l2pt->MAC[3],l2pt->MAC[4],l2pt->MAC[5],l2pt->Port, l2pt->VlanID);
	
	return 0;
}

int shrink_skb_pool(void) {
	rtl8672_L2EntrySearch(print_l2entry,0);
	return 0;
	/*
	unsigned long	flags;
	struct sk_buff *skb;
	printk("%s(%d) free %d\n", __FUNCTION__, __LINE__, free_cnt);
	
	SAVE_INT_AND_CLI(flags);
	while (free_cnt > 10) {
		skb = get_free_skb();
		if (skb) {
			skb->head = (unsigned char *) ((uint32)skb->head & (~0x20000000));
			dev_kfree_skb(skb);
		}
	}
	RESTORE_INT(flags);
	return 0;
	*/

	
}


int alloc_skb_pool(int num)
{
	int i;
	printk("alloc %d skb\n", num);
	for(i=0;i<num;i++)
	{
		skb_pool_ring[i]=dev_alloc_skb(BUFFER_SIZE);		
		ASSERT((u32 *)skb_pool_ring[i]!=NULL);
		init_skb(skb_pool_ring[i],BUFFER_SIZE);
//		printk("init skb[%d]=%x\n",i,skb_pool_ring[i]);
	}
	free_cnt=MAX_SKB_POOL;
	skb_pool_alloc_index=0;
	skb_pool_free_index=0;
	return SUCCESS;
}

void set_sar_vid(int ch,int vlanid){
	rtl8672_tblAsicDrv_intfParam_t outintf;
	printk("set channel:%d  vlanid:%d\n",ch,vlanid);
	rtl8672_getAsicNetInterface(ch, &outintf);
	outintf.PortVlanID=vlanid;
	rtl8672_setAsicNetInterface(ch, &outintf);	
}
#endif

int Dump_ppTrapCPUpkt_debug=0;	//Dump the packet content being trapped to CPU
int Dump_ppTrapCPU_hsab_debug=0;	//Dump the HSA and HSB content being trapped to CPU

void pp_memDump (void *start, u32 size, char * strHeader, char type)
{
	int row, column, index, index2, max;
//	uint32 buffer[5];
	u8 *buf, *line, ascii[17];
	char empty = ' ';

	if(!start ||(size==0))
		return;
	line = (u8*)start;

	/*
	16 bytes per line
	*/

	if (strHeader && (type==0))  {
		/* MAC */
		printk("\033[41;37m%s\033[0m", strHeader);
	}else if (strHeader && (type==1))  {
		/* PTM */
		printk("\033[44;37m%s\033[0m", strHeader);
	}else {
		printk("\033[42;37m%s\033[0m", strHeader);
	}
	
	column = size % 16;
	row = (size / 16) + 1;
	for (index = 0; index < row; index++, line += 16) 
	{
		buf = line;

		memset (ascii, 0, 17);

		max = (index == row - 1) ? column : 16;
		if ( max==0 ) break; /* If we need not dump this line, break it. */

		printk("\n%08x ", (u32) line);
		
		//Hex
		for (index2 = 0; index2 < max; index2++)
		{
			if (index2 == 8)
			printk("  ");
			printk("%02x ", (u8) buf[index2]);
			ascii[index2] = ((u8) buf[index2] < 32) ? empty : buf[index2];
		}

		if (max != 16)
		{
			if (max < 8)
				printk("  ");
			for (index2 = 16 - max; index2 > 0; index2--)
				printk("   ");
		}

		//ASCII
		printk("  %s", ascii);
	}
	printk("\n");
	return;
}


void set_intf_vid(int ch,int vlanid){
	rtl8672_tblAsicDrv_intfParam_t outintf;
	printk("set channel:%d  vlanid:%d\n",ch,vlanid);
	rtl8672_getAsicNetInterface(ch, &outintf);
	outintf.PortVlanID=vlanid;
	rtl8672_setAsicNetInterface(ch, &outintf);	
}

#ifdef CONFIG_RTL8681_PTM
void init_ptm(void)
{
	(*(volatile unsigned char*)(PTM_BASE+PTM_CR)) = 0x01;  //ptm sw reset
	(*(volatile unsigned char*)(PTM_BASE+PTM_CR)) = 0x02;  //ptm checksum offload

	/* QoS setting */
	WRITE_MEM32(PTM_BASE+PTM_QoS_MIS,0x8);

	/* Bondiing setting */
	WRITE_MEM32(PTM_BASE+PTM_BD_SLV_NUM, 0x00000002);
	WRITE_MEM32(PTM_BASE+PTM_BOND_FRAG_LF, 0x000000b4);
	WRITE_MEM32(PTM_BASE+PTM_BOND_FRAG_HF, 0x00000058);
	WRITE_MEM32(PTM_BASE+PTM_BOND_FRAG_LS, 0x000000ac);
	WRITE_MEM32(PTM_BASE+PTM_BOND_FRAG_HS, 0x00000050);
	WRITE_MEM32(PTM_BASE+PTM_BD_TIMEOUT, 0x000061a8);

	(*(volatile unsigned short*)(PTM_BASE+PTM_ISR1))=0x0;
	WRITE_MEM32(PTM_BASE+PTM_ISR0,0xffffffff);

	// PTM PTx ring allocated
	WRITE_MEM32(PTM_BASE+PTM_TxFDP0_F, PHYSICAL_ADDRESS(user_ptm_ptx_idx(0,0)));
	printk("PTM_TxFDP0_F = 0x%08X\n",(uint32)(user_ptm_ptx_idx(0,0)));
	WRITE_MEM32(PTM_BASE+PTM_TxFDP1_F, PHYSICAL_ADDRESS(user_ptm_ptx_idx(1,0)));
	printk("PTM_TxFDP1_F = 0x%08X\n",(uint32)(user_ptm_ptx_idx(1,0)));
	WRITE_MEM32(PTM_BASE+PTM_TxFDP2_F, PHYSICAL_ADDRESS(user_ptm_ptx_idx(4,0)));
	printk("PTM_TxFDP2_F = 0x%08X\n",(uint32)(user_ptm_ptx_idx(4,0)));
	WRITE_MEM32(PTM_BASE+PTM_TxFDP3_F, PHYSICAL_ADDRESS(user_ptm_ptx_idx(5,0)));
	printk("PTM_TxFDP3_F = 0x%08X\n",(uint32)(user_ptm_ptx_idx(5,0)));
	WRITE_MEM32(PTM_BASE+PTM_TxFDP0_S, PHYSICAL_ADDRESS(user_ptm_ptx_idx(2,0)));
	printk("PTM_TxFDP0_S = 0x%08X\n",(uint32)(user_ptm_ptx_idx(2,0)));
	WRITE_MEM32(PTM_BASE+PTM_TxFDP1_S, PHYSICAL_ADDRESS(user_ptm_ptx_idx(3,0)));
	printk("PTM_TxFDP1_S = 0x%08X\n",(uint32)(user_ptm_ptx_idx(3,0)));
	WRITE_MEM32(PTM_BASE+PTM_TxFDP2_S, PHYSICAL_ADDRESS(user_ptm_ptx_idx(6,0)));
	printk("PTM_TxFDP2_S = 0x%08X\n",(uint32)(user_ptm_ptx_idx(6,0)));
	WRITE_MEM32(PTM_BASE+PTM_TxFDP3_S, PHYSICAL_ADDRESS(user_ptm_ptx_idx(7,0)));
	printk("PTM_TxFDP3_S = 0x%08X\n",(uint32)(user_ptm_ptx_idx(7,0)));

	// PTM PRx ring allocated

#if 0  //0437 setting
	WRITE_MEM32(PTM_BASE+PTM_RxFDP_H_S, PHYSICAL_ADDRESS(user_ptm_prx_idx(0,0)));
	WRITE_MEM32(PTM_BASE+PTM_RxFDP_L_S, PHYSICAL_ADDRESS(user_ptm_prx_idx(1,0)));
	WRITE_MEM32(PTM_BASE+PTM_RxFDP_H_F, PHYSICAL_ADDRESS(user_ptm_prx_idx(2,0)));
	WRITE_MEM32(PTM_BASE+PTM_RxFDP_L_F, PHYSICAL_ADDRESS(user_ptm_prx_idx(3,0)));
	WRITE_MEM32(PTM_BASE+PTM_RxFDP_HMQ_H_S, PHYSICAL_ADDRESS(user_ptm_prx_idx(4,0)));
	WRITE_MEM32(PTM_BASE+PTM_RxFDP_HMQ_L_S, PHYSICAL_ADDRESS(user_ptm_prx_idx(5,0)));
	WRITE_MEM32(PTM_BASE+PTM_RxFDP_HMQ_H_F, PHYSICAL_ADDRESS(user_ptm_prx_idx(6,0)));
	WRITE_MEM32(PTM_BASE+PTM_RxFDP_HMQ_L_F, PHYSICAL_ADDRESS(user_ptm_prx_idx(7,0)));
#else //0513 setting, Bic changs the PTM queue mapping
	WRITE_MEM32(PTM_BASE+PTM_RxFDP_L_F, PHYSICAL_ADDRESS(user_ptm_prx_idx(0,0)));
	printk("PTM_RxFDP_L_F = 0x%08X\n", (uint32)(user_ptm_prx_idx(1,0)));
	WRITE_MEM32(PTM_BASE+PTM_RxFDP_H_F, PHYSICAL_ADDRESS(user_ptm_prx_idx(1,0)));
	WRITE_MEM32(PTM_BASE+PTM_RxFDP_HMQ_L_F, PHYSICAL_ADDRESS(user_ptm_prx_idx(2,0))); // ori :2
	WRITE_MEM32(PTM_BASE+PTM_RxFDP_HMQ_H_F, PHYSICAL_ADDRESS(user_ptm_prx_idx(3,0))); // ori : 3
	WRITE_MEM32(PTM_BASE+PTM_RxFDP_L_S, PHYSICAL_ADDRESS(user_ptm_prx_idx(4,0)));
	WRITE_MEM32(PTM_BASE+PTM_RxFDP_H_S, PHYSICAL_ADDRESS(user_ptm_prx_idx(5,0)));
	WRITE_MEM32(PTM_BASE+PTM_RxFDP_HMQ_L_S, PHYSICAL_ADDRESS(user_ptm_prx_idx(6,0)));
	WRITE_MEM32(PTM_BASE+PTM_RxFDP_HMQ_H_S, PHYSICAL_ADDRESS(user_ptm_prx_idx(7,0)));

#endif

	WRITE_MEM32(PTM_BASE+PTM_SMSA, PHYSICAL_ADDRESS(sram_buf));
	//WRITE_MEM32(PTM_BASE+PTM_IOCMD, 0x38091030);
	WRITE_MEM32(PTM_BASE+PTM_IOCMD, 0x38011800);

	return;
}

int start_pp_ptm_rx(struct net_device* dev)
{
	pp_ptm_enable = 1;
	
	(*(volatile unsigned short*)(PTM_BASE+PTM_RCR))=0x1f;  // enable AE/AR/AB/AM/APM/AAP packet receive.	
	return 0;
}

int stop_pp_ptm_rx (struct net_device *dev)
{
	pp_ptm_enable = 0;
	
	(*(volatile unsigned short*)(PTM_BASE+PTM_RCR))=0x0;
	return 0;
}

#endif

void init_nic(void)
{ 


//tylo, for external phy (MII)
#ifdef EXT_PHY
	/* have 1 tx port */
	WRITE_MEM32(NIC100MII_BASE+MAR0TO3,0xffffffff);  // allow multicast recv
	WRITE_MEM32(NIC100MII_BASE+MAR4TO7,0xffffffff);	
	(*(volatile unsigned char*)(NIC100MII_BASE+CR))=1;  //RESET
	(*(volatile unsigned char*)(NIC100MII_BASE+CR))=0x2;  //CRC offload enable
	WRITE_MEM32(NIC100MII_BASE+TCR,0xc00);	
	WRITE_MEM32(NIC100MII_BASE+RCR,0xf); // enable AB/AM/APM/AAP packet receive.
	WRITE_MEM32(NIC100MII_BASE+TXFDP4,(uint32)user_mac_ptx_idx(0,0));
	WRITE_MEM32(NIC100MII_BASE+RXFDP,(uint32)user_mac_prx_idx(0,0));
	if((((uint32)user_mac_prx_idx(0,0))&0xff)!=0) rtlglue_printf("ERROR: init_nic() error, NIC100MII_RXFDP must 256 bytes alignment!\n");
	WRITE_MEM32(NIC100MII_BASE+SMSA,(uint32)sram_buf);
	WRITE_MEM32(NIC100MII_BASE+ETHER_IO_CMD,READ_MEM32(NIC100MII_BASE+ETHER_IO_CMD)|0x1830);
#else
	/* have 4 tx port */
	WRITE_MEM32(NIC100_BASE+MAR0TO3,0xffffffff);  // allow multicast recv
	WRITE_MEM32(NIC100_BASE+MAR4TO7,0xffffffff);
	(*(volatile unsigned char*)(NIC100_BASE+CR))=1;  //RESET
	(*(volatile unsigned char*)(NIC100_BASE+CR))=0x2;  //CRC offload enable
	WRITE_MEM32(NIC100_BASE+TCR,0xc00);
	WRITE_MEM32(NIC100_BASE+RCR,0xf); // enable AB/AM/APM/AAP packet receive.
#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	WRITE_MEM32(NIC100_BASE+TXFDP1,(uint32)user_mac_ptx_idx(1,0));
//	printk("nic ptx ring 1 start address=%x\n",(uint32)user_mac_ptx_idx(1,0));	
	WRITE_MEM32(NIC100_BASE+TXFDP2,(uint32)user_mac_ptx_idx(2,0));
//	printk("nic ptx ring 2 start address=%x\n",(uint32)user_mac_ptx_idx(2,0));

	WRITE_MEM32(NIC100_BASE+TXFDP3,(uint32)user_mac_ptx_idx(3,0));
//	printk("nic ptx ring 3 start address=%x\n",(uint32)user_mac_ptx_idx(3,0));	

	WRITE_MEM32(NIC100_BASE+TXFDP4,(uint32)user_mac_ptx_idx(4,0));	
//	printk("nic ptx ring 4 start address=%x\n",(uint32)user_mac_ptx_idx(4,0));	
#else //0437 mapping
	WRITE_MEM32(NIC100_BASE+TXFDP2,((uint32)user_mac_ptx_idx(1,0)));
	WRITE_MEM32(NIC100_BASE+TXFDP3,((uint32)user_mac_ptx_idx(2,0)));
	WRITE_MEM32(NIC100_BASE+TXFDP4,((uint32)user_mac_ptx_idx(3,0)));
	WRITE_MEM32(NIC100_BASE+TXFDP5,((uint32)user_mac_ptx_idx(4,0)));
	
	WRITE_MEM32(NIC100_BASE+TXFDP1,((uint32)user_mac_ptx_idx(0,0)));
#endif
	WRITE_MEM32(NIC100_BASE+RXFDP,((uint32)user_mac_prx_idx(1,0)));

//	printk("nic prx start address=%x\n",(uint32)user_mac_prx_idx(1,0));
//	if((((uint32)user_mac_prx_idx(1,0))&0xff)!=0) rtlglue_printf("ERROR: init_nic() error, NIC100_RXFDP must 256 bytes alignment!\n");	
//	if((((uint32)user_mac_ptx_idx(1,0))&0xff)!=0) rtlglue_printf("ERROR: init_nic() error, NIC100_TXFDP must 256 bytes alignment!\n");
	
	WRITE_MEM32(NIC100_BASE+SMSA,(uint32)PHYSICAL_ADDRESS(sram_buf));
	WRITE_MEM32(NIC100_BASE+ETHER_IO_CMD1, 0x10000000);  //czyao, adds for RLE0437
	WRITE_MEM32(NIC100_BASE+ETHER_IO_CMD,READ_MEM32(NIC100_BASE+ETHER_IO_CMD)|0x1830);
#endif



}

int start_pp_nic_rx(struct net_device* dev)
{
	pp_nic_enable = 1;
//tylo, for external phy (MII)
#ifdef EXT_PHY
	/* have 1 tx port */
	WRITE_MEM32(NIC100MII_BASE+RCR,0xf); // enable AB/AM/APM/AAP packet receive.
#else
	/* have 4 tx port */
	WRITE_MEM32(NIC100_BASE+RCR,0xf); // enable AB/AM/APM/AAP packet receive.
#endif
	return 0;
}


//#define SRAM_BUFFER_OVERLAP_DEBUG
#ifdef SRAM_BUFFER_OVERLAP_DEBUG
#define check_mem(x) \
do{ \
	int i; \
	printf("==== %s %d\n",__FUNCTION__,__LINE__); \
	for(i=0;i<140;i++) \
	{ \
		unsigned char b[128]; \
		memset(&b,0xff,128); \
		if(memcmp(sram_buf+(128*(372+i)),b,128)!=0) \
		{ \
 			printf("error at i=%d...\n",i); \
			memDump((sram_buf+(128*(372+i))),128,"data"); \
			exit(1); \
		} \
	} \
} while(0);


#define check_sar_ring(x) \
	do { \
		int i,j; \
		struct sar_pRx *prx; \
		for(i=0;i<SAR_INTFS;i++) \
		{ \
		for(j=0;j<SAR_PRX_RING_SIZE;j++) \
		{ \
		prx=user_sar_prx_idx(i,j); \
		if(prx->skb_header_addr&0xf0000000) \
			{ \
			printf("error at %s %d\n",__FUNCTION__,__LINE__);		 \
			rtl8672_dumpRing(0,i); \
			return FAILED; \
			} \
		} \
		} \
	}while(0);

#endif

int assert_sram_size(int total_used_sram_size)
{
		if(total_used_sram_size>32*1024)
		{
			rtlglue_printf("Out of SRAM size!!\n"); 
			while(1);
		}
		return SUCCESS;
}
		


#if 1 //8672 mac driver

struct net_device_stats *rtl8672_mac_get_stats (struct net_device *dev)
{
	struct mac_private *tp = dev->priv;
	return &tp->stats;
}

#endif


void skb_debug(struct sk_buff* skb, int len){
	int i;
	for(i=0;i<len;i++){
		if(i%0x10==0 && i!=0)
			printk("\n");
		printk("%2x ",skb->data[i]);
	}
	printk("\n");
}
int stop_pp_nic_rx (struct net_device *dev)
{
	pp_nic_enable = 0;
	
#ifdef EXT_PHY
	WRITE_MEM32(NIC100MII_BASE+RCR,0x0); 
#else
	/* have 4 tx port */
	WRITE_MEM32(NIC100_BASE+RCR,0x0);
#endif

	return 0;
}

void pktfree(void){
			int idx;
#ifdef EXT_PHY
			int intf=0;
#else
			int intf=1;
#endif
			for(idx=0; idx<MAC_VTX_RING_SIZE;idx++){
				struct mac_vTx *tmpvtx;
				tmpvtx=user_mac_vtx_idx(intf,(cpu_mac_vtx_idx[intf]+idx)%MAC_VTX_RING_SIZE);
				if(tmpvtx->own==0 && cpu_mac_vtx_skb[intf][(cpu_mac_vtx_idx[intf]+idx)%MAC_VTX_RING_SIZE]!=0){
					dev_kfree_skb((struct sk_buff *)cpu_mac_vtx_skb[intf][(cpu_mac_vtx_idx[intf]+idx)%MAC_VTX_RING_SIZE]);
					cpu_mac_vtx_skb[intf][(cpu_mac_vtx_idx[intf]+idx)%MAC_VTX_RING_SIZE]=(uint32)NULL;
				}
			}
}

//for l2 and l4 entry expire
struct timer_list l2_timeout_timer;
struct timer_list l4_timeout_timer;
unsigned long l2timeout_arg;
unsigned long l4timeout_arg;
#define L2_TIMEOUT	300	//sec
#define L4_TIMEOUT	600	//sec
void l2_timeout_check(unsigned long arg){
	rtl8672_L2EntryTimeout();
	mod_timer(&l2_timeout_timer, jiffies + L2_TIMEOUT*HZ);
}
void l4_timeout_check(unsigned long arg){
	rtl8672_L4EntryTimeout();
	mod_timer(&l4_timeout_timer, jiffies + L4_TIMEOUT*HZ);
}
void init_table_timeout_timer(void){
	init_timer(&l2_timeout_timer);
	l2_timeout_timer.expires = jiffies + L2_TIMEOUT*HZ;
	l2_timeout_timer.function = l2_timeout_check;
	l2_timeout_timer.data = (unsigned long)l2timeout_arg;
	add_timer(&l2_timeout_timer);

	init_timer(&l4_timeout_timer);
	l4_timeout_timer.expires = jiffies + L4_TIMEOUT*HZ;
	l4_timeout_timer.function = l4_timeout_check;
	l4_timeout_timer.data = (unsigned long)l4timeout_arg;
	add_timer(&l4_timeout_timer);
}

static __init int init_ring(void)
{
       struct rtl8672_tblAsicDrv_intfParam_s *intftbl=NULL;
	rtl8672_tblAsicDrv_intfParam_t intf;
	int i,j,skb_id=0,sram_id=0;
//	memDump(0x80370000,128,"init_ring1");


#if defined(SRAM_MAPPING_ENABLED) && defined(RTL867X_MODEL_KERNEL)
	int total_used_sram_size=0;
#endif


	int max_sram_id=MAX_SRAM_MAP_ID;
	
	printk("Packet processor INIT!! MAX_SRAM_MAP_ID=%d NAX_SKB_ID=%d MAX_SKB_POOL=%d\n",MAX_SRAM_MAP_ID,MAX_SKB_ID,MAX_SKB_POOL);
	// Kaohj
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	pp_enabled = 1;
#endif
	//reset packet processor
	WRITE_MEM32(PPCR,PPCR_RESET);
	//enable packet processor
#if defined(CONFIG_RLE0437) || defined(CONFIG_RTL8681)
	*(volatile unsigned int*)0xb800010c |= 0x10;
#ifdef CONFIG_RTL8681_PTM
	*(volatile unsigned int*)0xb800010c |= 0x202;
#endif
#else
	*(volatile unsigned int*)0xb800330c=0x800008f0;
#endif

	MT_RINGCTRL("call");
	rtl8139rx_drop=0;


	testALE_Reset();

#ifdef CONFIG_RTL8672_FPGA
#ifdef MAC_LLIP_SIM
#else
// force 8139 speed at 10 Mbps on FPGA

if((READ_MEM32(0xb9000060)&0xff)==0x0011) 
	WRITE_MEM32(0xb9000060,READ_MEM32(0xb9000060)&0xffffffef);
#endif
#endif

	if(sram_buf==NULL) 
	{

#ifdef SRAM_MAPPING_ENABLED		
		sram_buf=(unsigned char *)rtlglue_malloc(64*1024);		
		printk("sram_buf=%x~%x\n",(u32)sram_buf,(u32)sram_buf+64*1024);
#ifdef BUFFER_HDR_IN_SRAM		
		total_used_sram_size+=128*(max_sram_id);  
		if(assert_sram_size(total_used_sram_size)!=SUCCESS) return FAILED;
#endif		
#else
		sram_buf=(unsigned char *)rtlglue_malloc(64*1024);	
#endif
		
		if(sram_buf==NULL)
		{
			rtlglue_printf("Out of memory!!\n"); 
			return FAILED;
		}

		if((u32)sram_buf & 0x7fff) //is 32k alignment
		sram_buf=(unsigned char *)(((((uint32)sram_buf)&(0xffff8000))|(0xa0000000))+32*1024);
		else
			sram_buf=(unsigned char *)(((uint32)sram_buf)|(0xa0000000));
		
		printk("align 32k sram_buf=%x\n",(u32)sram_buf);		

#ifdef SRAM_MAPPING_ENABLED	
		//SRAM Mapping enable
		WRITE_MEM32(0xb8001300,(((uint32)sram_buf)&(0x1ffffffe))|1);
		WRITE_MEM32(0xb8001304,8);  // 32K  DRAM unmap
		WRITE_MEM32(0xb8004000,(((uint32)sram_buf)&(0x1ffffffe))|1);
		WRITE_MEM32(0xb8004004,8);  // 32K  SRAM mapping
#endif
	}


#ifdef SRAM_BUFFER_OVERLAP_DEBUG	
	memset(sram_buf,0xff,128*512);
#endif


	//if(first_time==TRUE)
	{
		int rc;

		prealloc_skb_init(MAX_SKB_POOL);					

		ext_dev = alloc_etherdev(sizeof(struct ext_private));
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
		sarPP_dev = alloc_etherdev(sizeof(struct sar_private));
#endif
#ifdef CONFIG_USB_RTL8192SU_SOFTAP		
		rc = request_irq(PKT_IRQ, ext_intHandler, IRQF_SHARED, "EXT", ext_dev);
		if (rc)
		{
			rtlglue_printf("request irq error\n");
			return FAILED;
		}
#endif	
#ifdef CONFIG_RTL8681_PTM
		rc = request_irq(PKT_SAR_IRQ, ptm_intHandler, IRQF_SHARED, "PP_PTM", ptmPP_dev);
		if (rc)
		{
			rtlglue_printf("request irq error\n");
			return FAILED;
		}
#endif
		

#ifdef EXT_PHY
		rc = request_irq(PKT_NIC100MII_IRQ, mac_intHandler, IRQF_SHARED, "MAC0", mac_dev[0]);
#else
		rc = request_irq(PKT_NIC100_IRQ, mac_intHandler, IRQF_SHARED, "MAC0", mac_dev[1]);
#endif
		if (rc)
		{
			rtlglue_printf("request irq error\n");
			return FAILED;
		}
		//rc = request_irq(mac_dev[1]->irq, mac_intHandler, SA_SHIRQ, "MAC1", mac_dev[1]);
		//if (rc)
		//{
		//	rtlglue_printf("request irq error\n");
		//	return FAILED;
		//}

#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
		ppsar_rx_tasklets.func=(void (*)(unsigned long))ppsar_rx_bh;
		//ppsar_rx_tasklets.data=(unsigned long)sarcp;//(unsigned long)cp;
#endif

#ifdef CONFIG_RTL8681_PTM
		ppptm_rx_tasklets.func=(void (*)(unsigned long))ppptm_rx_bh;
#endif
		
#ifdef MAC_USE_TASKLET
		ppmac_rx_tasklets.func=(void (*)(unsigned long))ppmac_rx_bh;
#endif

#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
		rc = request_irq(PKT_SAR_IRQ, sar_intHandler, IRQF_SHARED, "SAR", sarPP_dev);
		if (rc)
		{
			rtlglue_printf("request irq error\n");
			return FAILED;
		}
#endif
	

		/* enable GIMR - for packet processor - EXT/MAC/SAR  */
#ifdef EXT_PHY
		WRITE_MEM32(GIMR,READ_MEM32(GIMR)|0x10440000); 
#else
		WRITE_MEM32(GIMR,READ_MEM32(GIMR)|0x10840000); 		
#endif
		//first_time=FALSE;
		
	}

	/* using uncache address */

/* ----------------------------------------------------------------------------------------------------------- */
/* ------------------------ MAC PRx---------------------------- */
	if(user_mac_prx==NULL) 	
	{
#ifdef SRAM_MAPPING_ENABLED
		user_mac_prx=(struct mac_pRx *)(sram_buf+total_used_sram_size);
		total_used_sram_size+=MAC_RX_INTFS*MAC_PRX_RING_SIZE*sizeof(struct mac_pRx);
		if(assert_sram_size(total_used_sram_size)!=SUCCESS) return FAILED;
#else
		user_mac_prx=(struct mac_pRx *)((uint32)(rtlglue_malloc(MAC_RX_INTFS*MAC_PRX_RING_SIZE*sizeof(struct mac_pRx)))|0xa0000000);
		memset(user_mac_prx, 0, (MAC_RX_INTFS*MAC_PRX_RING_SIZE*sizeof(struct mac_pRx)));
#endif
	}
	
	if(user_mac_prx==NULL) 	{ rtlglue_printf("Out of memory!!\n");  return FAILED; }	

/* ----------------------------------------------------------------------------------------------------------- */
/* ------------------------ SAR PRx---------------------------- */
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	if(user_sar_prx==NULL) 	
	{
#ifdef SRAM_MAPPING_ENABLED
		user_sar_prx=(struct sar_pRx *)(sram_buf+total_used_sram_size);
		total_used_sram_size+=SAR_INTFS*SAR_PRX_RING_SIZE*sizeof(struct sar_pRx);
		if(assert_sram_size(total_used_sram_size)!=SUCCESS) return FAILED;
#else
		user_sar_prx=(struct sar_pRx *)((uint32)(rtlglue_malloc(SAR_INTFS*SAR_PRX_RING_SIZE*sizeof(struct sar_pRx)))|0xa0000000);
		memset(user_sar_prx, 0, (SAR_INTFS*SAR_PRX_RING_SIZE*sizeof(struct sar_pRx)));
#endif	
		
	}	
	if(user_sar_prx==NULL) 	{ rtlglue_printf("Out of memory!!\n");  return FAILED; }
#endif
/* ----------------------------------------------------------------------------------------------------------- */
/* ------------------------ PTM PRx---------------------------- */
#ifdef CONFIG_RTL8681_PTM

	if(user_ptm_prx[PTM_INTFS-1]==NULL)
	{
		int i;
		for(i=0;i<PTM_INTFS;i++)
		{
			int ringsize=0;
			switch(i)
			{
				case 0:
					ringsize=PTM_PRX_RING0_SIZE;
					break;
				default:
					ringsize=PTM_PRX_RINGx_SIZE;
					break;
			}

			if(ringsize!=0){
#ifdef SRAM_MAPPING_ENABLED
				user_ptm_prx[i]=(struct mac_pRx *)(sram_buf+total_used_sram_size);
				total_used_sram_size+=ringsize*sizeof(struct mac_pRx);
				if(assert_sram_size(total_used_sram_size)!=SUCCESS) return FAILED;
#else
				user_ptm_prx[i]=(struct mac_pRx *)((uint32)(rtlglue_malloc(ringsize*sizeof(struct mac_pRx)))|0xa0000000);
				if(user_ptm_prx[i]==NULL) 	{ rtlglue_printf("Out of memory!!\n");  return FAILED; }

				memset(user_ptm_prx[i],0, (ringsize*sizeof(struct mac_pRx)));
#endif
			}

		}


	}

#endif
/* ----------------------------------------------------------------------------------------------------------- */
/* ------------------------ EXT PRx---------------------------- */
	if(user_ext_prx==NULL) 	
	{
#ifdef SRAM_MAPPING_ENABLED
		user_ext_prx=(struct ext_Rx *)(sram_buf+total_used_sram_size);
		total_used_sram_size+=EXT_INTFS*EXT_PRX_RING_SIZE*sizeof(struct ext_Rx);
		if(assert_sram_size(total_used_sram_size)!=SUCCESS) return FAILED;
#else
		user_ext_prx=(struct ext_Rx *)((uint32)(rtlglue_malloc(EXT_INTFS*EXT_PRX_RING_SIZE*sizeof(struct ext_Rx)))|0xa0000000);
#endif		
	}
	if(user_ext_prx==NULL) 	{ rtlglue_printf("Out of memory!!\n");  return FAILED; }	
/* ----------------------------------------------------------------------------------------------------------- */	
/* ------------------------ MAC PTx---------------------------- */

	if(user_mac_ptxs[1]==NULL)
	{
		int i;
		for(i=0;i<MAC_TX_INTFS;i++)
		{
			int ringsize=0;
			switch(i)
			{
				case 0:
					ringsize=MAC_PTX_RING0_SIZE;
					break;
				case 1:
					ringsize=MAC_PTX_RING1_SIZE;
					break;						
				case 2:
					ringsize=MAC_PTX_RING2_SIZE;
					break;
				case 3:
					ringsize=MAC_PTX_RING3_SIZE;
					break;
				case 4:
					ringsize=MAC_PTX_RING4_SIZE;
					break;

			}

			if(ringsize!=0)
			{
#ifdef SRAM_MAPPING_ENABLED			
				user_mac_ptxs[i]=(struct mac_pTx *)(sram_buf+total_used_sram_size);
				total_used_sram_size+=ringsize*sizeof(struct mac_pTx);
				if(assert_sram_size(total_used_sram_size)!=SUCCESS) return FAILED;
#else
				user_mac_ptxs[i]=(struct mac_pTx *)((uint32)(rtlglue_malloc(ringsize*sizeof(struct mac_pTx)))|0xa0000000);
				if(user_mac_ptxs[i]==NULL) 	{ rtlglue_printf("Out of memory!!\n");  return FAILED; }	

				memset(user_mac_ptxs[i],0, (ringsize*sizeof(struct mac_pTx)));
#endif		
	}
		}
	}


/* ----------------------------------------------------------------------------------------------------------- */	
/* ------------------------ SAR PTx---------------------------- */
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	if(user_sar_ptx==NULL) 	
	{
#ifdef SRAM_MAPPING_ENABLED
		user_sar_ptx=(struct sar_pTx *)(sram_buf+total_used_sram_size);
		total_used_sram_size+=SAR_INTFS*SAR_PTX_RING_SIZE*sizeof(struct sar_pTx);
		if(assert_sram_size(total_used_sram_size)!=SUCCESS) return FAILED;
#else
		user_sar_ptx=(struct sar_pTx *)((uint32)(rtlglue_malloc(SAR_INTFS*SAR_PTX_RING_SIZE*sizeof(struct sar_pTx)))|0xa0000000);
#endif
		
	}
	if(user_sar_ptx==NULL) 	{ rtlglue_printf("Out of memory!!\n");  return FAILED; }
#endif

/* ----------------------------------------------------------------------------------------------------------- */	
/* ------------------------ PTM PTx---------------------------- */
#ifdef CONFIG_RTL8681_PTM

	if(user_ptm_ptx[PTM_INTFS-1]==NULL) 	
	{
		int i;
		for(i=0; i< PTM_INTFS; i++)
		{
			int ringsize = 0;
			if(i==0)
				ringsize = PTM_PTX_RING0_SIZE;
			else
				ringsize = PTM_PTX_RINGx_SIZE;

			if(ringsize!=0){
//#ifdef SRAM_MAPPING_ENABLED 
#if defined(SRAM_MAPPING_ENABLED) && defined(SRAM_PROTECTION)
				user_ptm_ptx[i]=(struct mac_pTx *)(sram_buf+total_used_sram_size);
				total_used_sram_size+=ringsize*sizeof(struct mac_pTx);
				if(assert_sram_size(total_used_sram_size)!=SUCCESS) return FAILED;
#else
				user_ptm_ptx[i]=(struct mac_pTx *)((uint32)(rtlglue_malloc(ringsize*sizeof(struct mac_pTx)))|0xa0000000);
				if(user_ptm_ptx[i]==NULL) 	{ rtlglue_printf("Out of memory!!\n");  return FAILED; }

				memset(user_ptm_ptx[i],0, (ringsize*sizeof(struct mac_pTx)));
#endif
			}
		}
	}
#endif


/* ----------------------------------------------------------------------------------------------------------- */	
/* ------------------------ EXT PTx---------------------------- */

	if(user_ext_ptxs[2]==NULL) 	
	{
		int i;
		for(i=0;i<EXT_INTFS;i++)
		{
			int ringsize=0;
			switch(i)
			{
				case 0:
					ringsize=EXT_PTX_RING0_SIZE;
					break;
				case 1:
					ringsize=EXT_PTX_RING1_SIZE;
					break;						
				case 2:
					ringsize=EXT_PTX_RING2_SIZE;
					break;
			}

			if(ringsize!=0)
			{

#ifdef SRAM_MAPPING_ENABLED			
				user_ext_ptxs[i]=(struct ext_Tx *)(sram_buf+total_used_sram_size);
//printk("user_ext_ptxs[%d]=%x ringsize=%d\n",i,(u32)user_ext_ptxs[i],ringsize);		
				total_used_sram_size+=ringsize*sizeof(struct ext_Tx);
				if(assert_sram_size(total_used_sram_size)!=SUCCESS) return FAILED;
#else
				user_ext_ptxs[i]=(struct ext_Tx *)((uint32)(rtlglue_malloc(ringsize*sizeof(struct ext_Tx)))|0xa0000000);
				if(user_ext_ptxs[i]==NULL) 	{ rtlglue_printf("Out of memory!!\n");  return FAILED; }	
#endif
	}
		}		
	}

	
	
/* ----------------------------------------------------------------------------------------------------------- */
	/* ------------------------ MAC VRx---------------------------- */
	if(user_mac_vrx==NULL) 	user_mac_vrx=(struct mac_vRx *)((uint32)rtlglue_malloc(MAC_RX_INTFS*MAC_VRX_RING_SIZE*sizeof(struct mac_vRx))|0xa0000000);
	if(user_mac_vrx==NULL) 	{ rtlglue_printf("Out of memory!!\n");  return FAILED; }	

	memset(user_mac_vrx, 0, (MAC_RX_INTFS*MAC_VRX_RING_SIZE*sizeof(struct mac_vRx)));

	/* ------------------------ SAR VRx---------------------------- */
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	if(user_sar_vrx==NULL) 	user_sar_vrx=(struct sar_vRx *)((uint32)rtlglue_malloc(SAR_INTFS*SAR_VRX_RING_SIZE*sizeof(struct sar_vRx))|0xa0000000);
	if(user_sar_vrx==NULL) 	{ rtlglue_printf("Out of memory!!\n");  return FAILED; }		
#endif

	/* ------------------------ PTM VRx---------------------------- */
#ifdef CONFIG_RTL8681_PTM
	if(user_ptm_vrx==NULL)	user_ptm_vrx=(struct mac_vRx *)((uint32)rtlglue_malloc(PTM_INTFS*PTM_VRX_RING_SIZE*sizeof(struct mac_vRx))|0xa0000000);
	if(user_ptm_vrx==NULL)	{ rtlglue_printf("Out of memory!!\n");	return FAILED; }		

	memset(user_ptm_vrx,0, (PTM_INTFS*PTM_VRX_RING_SIZE*sizeof(struct mac_vRx)));
#endif

	/* ------------------------ EXT VRx---------------------------- */
	if(user_ext_vrx==NULL) 	user_ext_vrx=(struct ext_Rx *)((uint32)rtlglue_malloc(EXT_INTFS*EXT_VRX_RING_SIZE*sizeof(struct ext_Rx))|0xa0000000);
	if(user_ext_vrx==NULL) 	{ rtlglue_printf("Out of memory!!\n");  return FAILED; }		

	/* ------------------------ MAC VTx---------------------------- */
	if(user_mac_vtx==NULL) 	user_mac_vtx=(struct mac_vTx *)((uint32)rtlglue_malloc(MAC_TX_INTFS*MAC_VTX_RING_SIZE*sizeof(struct mac_vTx))|0xa0000000);
	if(user_mac_vtx==NULL) 	{ rtlglue_printf("Out of memory!!\n");  return FAILED; }

	memset(user_mac_vtx,0, (MAC_TX_INTFS*MAC_VTX_RING_SIZE*sizeof(struct mac_vTx)));

	/* ------------------------ SAR VTx---------------------------- */
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	if(user_sar_vtx==NULL) 	user_sar_vtx=(struct sar_vTx *)((uint32)rtlglue_malloc(SAR_INTFS*SAR_VTX_RING_SIZE*sizeof(struct sar_vTx))|0xa0000000);
	if(user_sar_vtx==NULL) 	{ rtlglue_printf("Out of memory!!\n");  return FAILED; }
#endif

	/* ------------------------ PTM VTx---------------------------- */
#ifdef CONFIG_RTL8681_PTM
	if(user_ptm_vtx==NULL) 	user_ptm_vtx=(struct mac_vTx *)((uint32)rtlglue_malloc(PTM_INTFS*PTM_VTX_RING_SIZE*sizeof(struct mac_vTx))|0xa0000000);
	if(user_ptm_vtx==NULL) 	{ rtlglue_printf("Out of memory!!\n");  return FAILED; }

	memset(user_ptm_vtx, 0, (PTM_INTFS*PTM_VTX_RING_SIZE*sizeof(struct mac_vTx)));
#endif

	/* ------------------------ EXT VTx---------------------------- */
	if(user_ext_vtx==NULL) 	user_ext_vtx=(struct ext_Tx *)((uint32)rtlglue_malloc(EXT_INTFS*EXT_VTX_RING_SIZE*sizeof(struct ext_Tx))|0xa0000000);
	if(user_ext_vtx==NULL) 	{ rtlglue_printf("Out of memory!!\n");  return FAILED; }		


/* ----------------------------------------------------------------------------------------------------------- */	
	/* ------------------------ MAC SP Ring---------------------------- */
	if(user_sp_to_macs[1]==NULL) 	
	{
		int i;
		for(i=0;i<MAC_TX_INTFS;i++)
		{
			int ringsize=0;
			switch(i)
			{
				case 0:
					ringsize=MAC_PTX_RING0_SIZE;
					break;
				case 1:
					ringsize=MAC_PTX_RING1_SIZE;
					break;
				case 2:
					ringsize=MAC_PTX_RING2_SIZE;
					break;
				case 3:
					ringsize=MAC_PTX_RING3_SIZE;
					break;
				case 4:
					ringsize=MAC_PTX_RING4_SIZE;
					break;						
			}

			if(ringsize!=0)
			{
#ifdef SRAM_MAPPING_ENABLED			
				user_sp_to_macs[i]=(struct sp_pRx *)(sram_buf+total_used_sram_size);
				total_used_sram_size+=ringsize*sizeof(struct sp_pRx);
				if(assert_sram_size(total_used_sram_size)!=SUCCESS) return FAILED;
#else
				user_sp_to_macs[i]=(struct sp_pRx *)((uint32)rtlglue_malloc(ringsize*sizeof(struct sp_pRx))|0xa0000000);
				if(user_sp_to_macs[i]==NULL) 	{ rtlglue_printf("Out of memory!!\n");  return FAILED; }	
#endif		
	}
		}		
	}
	
/* ----------------------------------------------------------------------------------------------------------- */	
	/* ------------------------ SAR SP Ring---------------------------- */
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	if(user_sp_to_sar==NULL) 	
	{
#ifdef SRAM_MAPPING_ENABLED
		user_sp_to_sar=(struct sp_pRx *)(sram_buf+total_used_sram_size);
		total_used_sram_size+=SAR_INTFS*SAR_PTX_RING_SIZE*sizeof(struct sp_pRx);
		if(assert_sram_size(total_used_sram_size)!=SUCCESS) return FAILED;
#else
		user_sp_to_sar=(struct sp_pRx *)((uint32)rtlglue_malloc(SAR_INTFS*SAR_PTX_RING_SIZE*sizeof(struct sp_pRx))|0xa0000000);
#endif		
	}
	if(user_sp_to_sar==NULL) 	{ rtlglue_printf("Out of memory!!\n");  return FAILED; }	
#endif

	/* ------------------------ PTM PRx Ring---------------------------- */
#ifdef CONFIG_RTL8681_PTM

	if(user_sp_to_ptm[PTM_INTFS-1]==NULL) 	
	{
		int i;
		for(i=0; i<PTM_INTFS;i++)
		{
			int ringsize = 0;
			if(i==0)
				ringsize = PTM_PTX_RING0_SIZE;
			else
				ringsize = PTM_PTX_RINGx_SIZE;

			if(ringsize!=0)
			{
#ifdef SRAM_MAPPING_ENABLED
				user_sp_to_ptm[i]=(struct sp_pRx *)(sram_buf+total_used_sram_size);
				total_used_sram_size+=ringsize*sizeof(struct sp_pRx);
				if(assert_sram_size(total_used_sram_size)!=SUCCESS) return FAILED;
#else
				user_sp_to_ptm[i]=(struct sp_pRx *)((uint32)rtlglue_malloc(ringsize*sizeof(struct sp_pRx))|0xa0000000);
				if(user_sp_to_ptm[i]==NULL) 	{ rtlglue_printf("Out of memory!!\n");  return FAILED; }	
#endif
			}
		}

	}
#endif

	
/* ----------------------------------------------------------------------------------------------------------- */
	/* ------------------------ EXT PRx Ring---------------------------- */

	if(user_sp_to_exts[2]==NULL) 	
	{
		int i;
		for(i=0;i<EXT_INTFS;i++)
		{
			int ringsize=0;
			switch(i)
			{
				case 0:
					ringsize=EXT_PTX_RING0_SIZE;
					break;
				case 1:
					ringsize=EXT_PTX_RING1_SIZE;
					break;
				case 2:
					ringsize=EXT_PTX_RING2_SIZE;
					break;					
			}

			if(ringsize!=0)
			{
#ifdef SRAM_MAPPING_ENABLED			
				user_sp_to_exts[i]=(struct sp_pRx *)(sram_buf+total_used_sram_size);
//printk("user_sp_to_exts[%d]=%x ringsize=%d\n",i,(u32)user_sp_to_exts[i],ringsize);
				total_used_sram_size+=ringsize*sizeof(struct sp_pRx);
				if(assert_sram_size(total_used_sram_size)!=SUCCESS) return FAILED;
#else
				user_sp_to_exts[i]=(struct sp_pRx *)((uint32)rtlglue_malloc(ringsize*sizeof(struct sp_pRx))|0xa0000000);
				if(user_sp_to_exts[i]==NULL) 	{ rtlglue_printf("Out of memory!!\n");  return FAILED; }	
#endif		
	}
		}
	}

/* ----------------------------------------------------------------------------------------------------------- */	
		   
	//init interface table, here PTM_INTFS = SAR_INTFS
	for(i=0;i<(MAC_TX_INTFS+SAR_INTFS+EXT_INTFS);i++)
	{
		intftbl = &intf;
		
		intftbl->AcceptTagged=1;
		intftbl->AcceptUntagged=1;
		intftbl->PortVlanID = 9;
		intftbl->SrcPortFilter = 0;
		intftbl->L2BridgeEnable=1;
		intftbl->Dot1QRemr=0;
		intftbl->VlanIDRemr=9;
		intftbl->LogicalID= i;


		intftbl->MTU = 1500;
		//tylo, for gmac
		intftbl->GMAC[0] = 0; intftbl->GMAC[1] = 0; intftbl->GMAC[2] = 0; intftbl->GMAC[3] = 0; intftbl->GMAC[4] = 0xff; intftbl->GMAC[5] = i;			
		//intftbl->GMAC[0] = 0x00; intftbl->GMAC[1] = 0xe0; intftbl->GMAC[2] = 0x4c; intftbl->GMAC[3] = 0x86; intftbl->GMAC[4] = 0x70; intftbl->GMAC[5] = 0x01;			
				
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
		if(i<SAR_INTFS)
		{
			intftbl->IfType = IF_SAR;

			intftbl->ATMPORT=1;
			intftbl->TRLREN=1;
			intftbl->CLP=0;
			intftbl->PTI=0x0;
			intftbl->TRLR=0;				
			intftbl->LanFCS = 0x0;
			intftbl->GIP = 0x01020300 | i;
			
			switch(i){
				case 0:
					intftbl->SARhdr = BRIDGED;
					intftbl->L2Encap=LLC_ENCAPSULATION;
					intftbl->RXshift = DMA_SHIFT;
					sram_map_offset[i] = 96;

					break;
				case 1:
					intftbl->SARhdr = ROUTED;
					intftbl->L2Encap=LLC_ENCAPSULATION;
					intftbl->RXshift = DMA_SHIFT;
					sram_map_offset[i] = 80;					
					break;
				case 2:
					intftbl->SARhdr = MER;
					intftbl->L2Encap=LLC_ENCAPSULATION;
					intftbl->RXshift = DMA_SHIFT;
					sram_map_offset[i] = 96;					

					break;
				case 3:
					intftbl->SARhdr = PPPOE;
					intftbl->L2Encap=LLC_ENCAPSULATION;					
					intftbl->RXshift = DMA_SHIFT;
					sram_map_offset[i] = 100;					

					break;
				case 4:
					intftbl->SARhdr = PPPOA;
					intftbl->L2Encap=LLC_ENCAPSULATION;					
					intftbl->RXshift = DMA_SHIFT+2;
					sram_map_offset[i] = 76;					

					break;
				case 5:
					intftbl->SARhdr = ROUTED;
					intftbl->L2Encap=VC_MULTIPLEXING;
					intftbl->RXshift = DMA_SHIFT;
					sram_map_offset[i] = 72;					
					break;
				case 6:
					intftbl->SARhdr = BRIDGED;
					intftbl->L2Encap=VC_MULTIPLEXING;
					intftbl->RXshift = DMA_SHIFT;
					sram_map_offset[i] = 88;					
					break;
				case 7:
					intftbl->SARhdr = MER;
					intftbl->L2Encap=VC_MULTIPLEXING;
					intftbl->RXshift = DMA_SHIFT;
					sram_map_offset[i] = 88;					
			}
		}
#elif defined(CONFIG_RTL8681_PTM) 	//ptm intf
		if(i<PTM_INTFS)
		{	
			intftbl->IfType = IF_ETHERNET;

			intftbl->LogicalID = i;
			intftbl->PortVlanID = 9;
			intftbl->RXshift = DMA_SHIFT+2;
			sram_map_offset[i] = 88;			
		}
#endif
		else if(i<(MAC_TX_INTFS+SAR_INTFS)) // mac intf
		{
			if(i>SAR_INTFS)
			{
				intftbl->LogicalID= 9;
				intftbl->PortVlanID = 9; //chihhsin, 2008,06,04
			}
			else
				intftbl->LogicalID= 8;
			
			intftbl->IfType = IF_ETHERNET;

			intftbl->RXshift = DMA_SHIFT+2;
			sram_map_offset[i] = 88;			
		}
		else if(i==(MAC_TX_INTFS+SAR_INTFS)) // ETH0: wlan 802.11 intf
		{		
			intftbl->IfType = IF_WIRELESS;
			intftbl->RXshift = DMA_SHIFT;	//chihhsing		
			intftbl->PortVlanID = 9; //chihhsin, 2008,06,04
			sram_map_offset[i] = 88;		
		}
		else if(i==(MAC_TX_INTFS+SAR_INTFS+1)) // ETH1: wlan 802.11 intf
		{
			intftbl->IfType = IF_ETHERNET; // 802.3 intf
			intftbl->RXshift = DMA_SHIFT+2;
			intftbl->PortVlanID = 9; //chihhsin, 2008,06,04
			sram_map_offset[i] = 88;			
		}
		else //ETH2
		{
			intftbl->IfType = IF_ETHERNET;
			intftbl->RXshift = DMA_SHIFT+2;
			sram_map_offset[i] = 88;	
			intftbl->PortVlanID = 9; //chihhsin, 2008,06,04			
		}

		rtl8672_setAsicNetInterface(i, intftbl);
		rtl8672_getAsicNetInterface(i, intftbl);

		
	}



	// share pool  init
	/* ------------------------ MAC share pool ---------------------------- */
	for(i=0;;i++)
	{

		if((i>=MAC_PTX_RING0_SIZE)&&(i>=MAC_PTX_RING1_SIZE)&&(i>=MAC_PTX_RING2_SIZE)&&(i>=MAC_PTX_RING3_SIZE)&&(i>=MAC_PTX_RING4_SIZE))
			break;
		
		for(j=0;j<MAC_TX_INTFS;j++)
		{
			struct sp_pRx *spRx;

			switch(j)
			{
				case 0:
					if(i>=MAC_PTX_RING0_SIZE) continue;
					break;
				case 1:
					if(i>=MAC_PTX_RING1_SIZE) continue;
					break;
				case 2:
					if(i>=MAC_PTX_RING2_SIZE) continue;
					break;
				case 3:
					if(i>=MAC_PTX_RING3_SIZE) continue;
					break;
				case 4:
					if(i>=MAC_PTX_RING4_SIZE) continue;
					break;
			}

			spRx=user_sp_to_mac_idx(j,i);

			spRx->own=1;
			// share pool do not need to set the rx_shift and buffer_size fields, because the value will fill when pRx and SharePool swap.
//			user_sp_to_mac[j][i].data_length=intf_tbl[j+8].buffer_size<<7;

			spRx->eor=0;
			switch(j)
			{
				case 0:
					if(i==MAC_PTX_RING0_SIZE-1) spRx->eor=1;
					break;
				case 1:
					if(i==MAC_PTX_RING1_SIZE-1) spRx->eor=1;
					break;
				case 2:
					if(i==MAC_PTX_RING2_SIZE-1) spRx->eor=1;
					break;
				case 3:
					if(i==MAC_PTX_RING3_SIZE-1) spRx->eor=1;
					break;
				case 4:
					if(i==MAC_PTX_RING4_SIZE-1) spRx->eor=1;
					break;
			}
		

			
//			user_sp_to_mac[j][i].linkid=0;			
//			user_sp_to_mac[j][i].rx_shift=intf_tbl[j+8].rx_shift;

			if(skbs[skb_id]==NULL) 
			{
//#ifdef CONFIG_USB_RTL8192SU_SOFTAP
#ifdef CONFIG_SKB_POOL_PREALLOC //czyao 0437FPGA
				skbs[skb_id]=prealloc_skb_get();
#else
				skbs[skb_id]=dev_alloc_skb(BUFFER_SIZE);
#endif
				ASSERT(skbs[skb_id]!=NULL);
				init_skb(skbs[skb_id],BUFFER_SIZE);
				skb_reserve(skbs[skb_id],DMA_SHIFT);
#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
				spRx->skb_header_addr=(uint32)(unsigned char *)(skbs[skb_id]);
				spRx->rx_buffer_addr=(uint32)skbs[skb_id]->head;
#else
				spRx->skb_header_addr=PHYSICAL_ADDRESS((unsigned char *)(skbs[skb_id]));
				spRx->rx_buffer_addr=PHYSICAL_ADDRESS(skbs[skb_id]->head);
#endif
			}
						
//			printk("mac intf =%d  idx=%x prx=%x eor=%x\n",j,i,(u32)spRx,spRx->eor);

#ifdef SRAM_MAPPING_ENABLED
#ifdef BUFFER_HDR_IN_SRAM
			spRx->sram_en=1;
#else
			spRx->sram_en=0;
#endif

#else
			spRx->sram_en=0;
#endif
			spRx->sram_map_id=sram_id;
			sram_id++;			
			skb_id++;


			spRx->sram_map_of=sram_map_offset[SAR_INTFS+j];
		}
	}

	/* ------------------------ SAR share pool ---------------------------- */
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	for(i=0;i<SAR_PTX_RING_SIZE;i++)
	{
		for(j=0;j<SAR_INTFS;j++)
		{
			struct sp_pRx *spRx;
			spRx=user_sp_to_sar_idx(j,i);		
			spRx->own=1;
//			user_sp_to_sar[j][i].data_length=intf_tbl[j].buffer_size<<7;
			if(i==SAR_PTX_RING_SIZE-1) 
				spRx->eor=1;
			else
				spRx->eor=0;
			
//			user_sp_to_sar[j][i].linkid=0;					

			if(skbs[skb_id]==NULL) 
			{
//#ifdef CONFIG_USB_RTL8192SU_SOFTAP
#ifdef CONFIG_SKB_POOL_PREALLOC //czyao 0437FPGA
				skbs[skb_id]=prealloc_skb_get();
#else
				skbs[skb_id]=dev_alloc_skb(BUFFER_SIZE);
#endif
				ASSERT(skbs[skb_id]!=NULL);
				init_skb(skbs[skb_id],BUFFER_SIZE);
				skb_reserve(skbs[skb_id],DMA_SHIFT);
				spRx->rx_buffer_addr=(uint32)skbs[skb_id]->head;

				spRx->skb_header_addr=(uint32)(unsigned char *)(skbs[skb_id]);				
			}

#ifdef SRAM_MAPPING_ENABLED
#ifdef BUFFER_HDR_IN_SRAM
			spRx->sram_en=1;
#else
			spRx->sram_en=0;
#endif
#else
			spRx->sram_en=0;
#endif
			spRx->sram_map_id=sram_id;
			skb_id++;
			sram_id++;
			spRx->sram_map_of=sram_map_offset[j];	
		}
	}
#endif

	/* ------------------------ PTM share pool ---------------------------- */
#ifdef CONFIG_RTL8681_PTM

	for(i=0;;i++)
	{
		if((i>=PTM_PTX_RING0_SIZE ) && (i>=PTM_PTX_RINGx_SIZE ))
			break;

		for(j=0;j<PTM_INTFS;j++)
		{
			struct sp_pRx *spRx;
			switch(j)
			{
				case 0:
					if(i>=PTM_PTX_RING0_SIZE) continue;
					break;
				default:
					if(i>=PTM_PTX_RINGx_SIZE) continue;
					break;
			}

			spRx=user_sp_to_ptm_idx(j,i);
			spRx->own=1;
			spRx->eor=0;

			switch(j)
			{
				case 0:
					if(i==PTM_PTX_RING0_SIZE-1) spRx->eor=1;
					break;
				default:
					if(i==PTM_PTX_RINGx_SIZE-1) spRx->eor=1;
					break;
			}

			if(skbs[skb_id]==NULL) 
			{
//#ifdef CONFIG_USB_RTL8192SU_SOFTAP
#ifdef CONFIG_SKB_POOL_PREALLOC //czyao 0437FPGA
				skbs[skb_id]=prealloc_skb_get();
#else
				skbs[skb_id]=dev_alloc_skb(BUFFER_SIZE);
#endif
				ASSERT(skbs[skb_id]!=NULL);
				init_skb(skbs[skb_id],BUFFER_SIZE);
				skb_reserve(skbs[skb_id],DMA_SHIFT);
#if !defined(CONFIG_RTE0437) && !defined(CONFIG_RTL8681)
				spRx->rx_buffer_addr=(uint32)skbs[skb_id]->head;
				spRx->skb_header_addr=(uint32)(unsigned char *)(skbs[skb_id]);
#else
				spRx->rx_buffer_addr=PHYSICAL_ADDRESS(skbs[skb_id]->head);
				spRx->skb_header_addr=PHYSICAL_ADDRESS((unsigned char *)(skbs[skb_id]));
#endif
			}

#ifdef SRAM_MAPPING_ENABLED
#ifdef BUFFER_HDR_IN_SRAM
				spRx->sram_en=1;
#else
				spRx->sram_en=0;
#endif
#else
				spRx->sram_en=0;
#endif
				spRx->sram_map_id=sram_id;
				skb_id++;
				sram_id++;
				spRx->sram_map_of=sram_map_offset[j];	
			
		}
		

	}


#endif


	/* ------------------------ EXT share pool ---------------------------- */
	for(i=0;;i++)
	{

		if((i>=EXT_PTX_RING0_SIZE)&&(i>=EXT_PTX_RING1_SIZE)&&(i>=EXT_PTX_RING2_SIZE))
			break;

	
		for(j=0;j<EXT_INTFS;j++)
		{
			struct sp_pRx *spRx;

			switch(j)
			{
				case 0:
					if(i>=EXT_PTX_RING0_SIZE) continue;
					break;
				case 1:
					if(i>=EXT_PTX_RING1_SIZE) continue;
					break;
				case 2:
					if(i>=EXT_PTX_RING2_SIZE) continue;
					break;
			}

			
			spRx=user_sp_to_ext_idx(j,i);
			spRx->own=1;
//			user_sp_to_ext[j][i].data_length=intf_tbl[j+13].buffer_size<<7;

			spRx->eor=0;
			switch(j)
			{
				case 0:
					if(i==EXT_PTX_RING0_SIZE-1) spRx->eor=1;
					break;
				case 1:
					if(i==EXT_PTX_RING1_SIZE-1) spRx->eor=1;
					break;
				case 2:
					if(i==EXT_PTX_RING2_SIZE-1) spRx->eor=1;
					break;
			}


//			user_sp_to_ext[j][i].linkid=0;
//			user_sp_to_ext[j][i].rx_shift=intf_tbl[j+13].rx_shift;

			if(skbs[skb_id]==NULL) 
			{
//#ifdef CONFIG_USB_RTL8192SU_SOFTAP
#ifdef CONFIG_SKB_POOL_PREALLOC //czyao 0437FPGA
				skbs[skb_id]=prealloc_skb_get();
#else
				skbs[skb_id]=dev_alloc_skb(BUFFER_SIZE);
#endif
				ASSERT(skbs[skb_id]!=NULL);
				init_skb(skbs[skb_id],BUFFER_SIZE);
				skb_reserve(skbs[skb_id],DMA_SHIFT);
				spRx->rx_buffer_addr=(uint32)skbs[skb_id]->head;			
				spRx->skb_header_addr=(uint32)(unsigned char *)(skbs[skb_id]);
			}

#ifdef SRAM_MAPPING_ENABLED
#ifdef BUFFER_HDR_IN_SRAM
			spRx->sram_en=1;
#else
			spRx->sram_en=0;
#endif
#else
			spRx->sram_en=0;
#endif

			spRx->sram_map_id=sram_id;
			sram_id++;
			skb_id++;
			
			spRx->sram_map_of=sram_map_offset[SAR_INTFS+MAC_TX_INTFS+j];			
		}	
	}


	if(BUFFER_SIZE>2048)
	{
		WRITE_MEM32(BUFFERSIZE,2048);	// packet processor max pkt size
		printk("Warning: BUFFER_SIZE>2048 , over packet processor max rx size!\n");
	}
	else
	{
		WRITE_MEM32(BUFFERSIZE,BUFFER_SIZE);
	}
	

	// init pRx ring
	/* ------------------------ MAC PRx ring ---------------------------- */
	for(i=0;i<MAC_PRX_RING_SIZE;i++)
	{
		for(j=0;j<MAC_RX_INTFS;j++)
		{
			struct mac_pRx *macRx;
			
			macRx=user_mac_prx_idx(j,i);
			macRx->own=1;
			macRx->data_length=READ_MEM32(BUFFERSIZE);

			if(i==MAC_PRX_RING_SIZE-1)
			{
				macRx->eor=1;

			}
			else
			{
				macRx->eor=0;
			}

			rtl8672_getAsicNetInterface(j+8, &intf);

			macRx->rx_shift=intf.RXshift;

			if(skbs[skb_id]==NULL) 
			{
//#ifdef CONFIG_USB_RTL8192SU_SOFTAP
#ifdef CONFIG_SKB_POOL_PREALLOC //czyao 0437FPGA
				skbs[skb_id]=prealloc_skb_get();
#else
				skbs[skb_id]=dev_alloc_skb(BUFFER_SIZE);
#endif
				ASSERT(skbs[skb_id]!=NULL);
				init_skb(skbs[skb_id],BUFFER_SIZE);
				skb_reserve(skbs[skb_id],intf.RXshift);
#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
				macRx->rx_buffer_addr=(uint32)skbs[skb_id]->head;
				macRx->skb_header_addr=(uint32)(unsigned char *)(skbs[skb_id]);
#else
				macRx->rx_buffer_addr=PHYSICAL_ADDRESS(skbs[skb_id]->head);
				macRx->skb_header_addr=PHYSICAL_ADDRESS((unsigned char *)(skbs[skb_id]));
#endif

			}

#ifdef SRAM_MAPPING_ENABLED
#ifdef BUFFER_HDR_IN_SRAM
			macRx->sram_en=1;
#else
			macRx->sram_en=0;
#endif
#else
			macRx->sram_en=0;
#endif

			macRx->sram_map_id=sram_id;
			sram_id++;
			skb_id++;
			macRx->sram_map_of=sram_map_offset[SAR_INTFS+j];

		}
	}

	/* ------------------------ SAR PRx ring ---------------------------- */
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	for(i=0;i<SAR_PRX_RING_SIZE;i++)
	{	
		for(j=0;j<SAR_INTFS;j++)
		{
			struct sar_pRx *sarRx;
			sarRx=user_sar_prx_idx(j,i);
			sarRx->own=1;
			sarRx->data_length=READ_MEM32(BUFFERSIZE);
			
			if(i==SAR_PRX_RING_SIZE-1) 
				sarRx->eor=1;
			else
				sarRx->eor=0;

			rtl8672_getAsicNetInterface(j, &intf);			
			sarRx->rx_shift=intf.RXshift;

			if(skbs[skb_id]==NULL) 
			{
//#ifdef CONFIG_USB_RTL8192SU_SOFTAP
#ifdef CONFIG_SKB_POOL_PREALLOC //czyao 0437FPGA
				skbs[skb_id]=prealloc_skb_get();
#else
				skbs[skb_id]=dev_alloc_skb(BUFFER_SIZE);
#endif
				ASSERT(skbs[skb_id]!=NULL);
				init_skb(skbs[skb_id],BUFFER_SIZE);
				skb_reserve(skbs[skb_id],intf.RXshift);

				sarRx->rx_buffer_addr=(uint32)skbs[skb_id]->head;
				sarRx->skb_header_addr=(uint32)(unsigned char *)(skbs[skb_id]);
			}

#ifdef SRAM_MAPPING_ENABLED
#ifdef BUFFER_HDR_IN_SRAM
			sarRx->sram_en=1;
#else
			sarRx->sram_en=0;
#endif
#else
			sarRx->sram_en=0;
#endif
			sarRx->sram_map_id=sram_id;
			skb_id++;
			sram_id++;
			sarRx->sram_map_of=sram_map_offset[j];			
		}
	}
#endif
#ifdef CONFIG_RTL8681_PTM
	/* ------------------------ PTM PRx ring ---------------------------- */

	for(i=0;;i++)
	{
		if((i>=PTM_PRX_RING0_SIZE)&&(i>=PTM_PRX_RINGx_SIZE))
			break;

		for(j=0;j<PTM_INTFS;j++)
		{
			struct mac_pRx *ptmRx;

			switch(j)
			{
				case 0:
					if(i>=PTM_PRX_RING0_SIZE) continue;
					break;
				default:
					if(i>=PTM_PRX_RINGx_SIZE) continue;
					break;
			}
			
			ptmRx=user_ptm_prx_idx(j,i);
			ptmRx->own=1;

			ptmRx->eor=0;
			switch(j)
			{
				case 0:					
					if(i==PTM_PRX_RING0_SIZE-1) ptmRx->eor=1;
					break;
				default:					
					if(i==PTM_PRX_RINGx_SIZE-1) ptmRx->eor=1;
					break;				
			}

			rtl8672_getAsicNetInterface(j, &intf);			
			ptmRx->rx_shift=intf.RXshift;

			if(skbs[skb_id]==NULL) 
			{
//#ifdef CONFIG_USB_RTL8192SU_SOFTAP
#ifdef CONFIG_SKB_POOL_PREALLOC //czyao 0437FPGA
				skbs[skb_id]=prealloc_skb_get();
#else
				skbs[skb_id]=dev_alloc_skb(BUFFER_SIZE);
#endif
				ASSERT(skbs[skb_id]!=NULL);
				init_skb(skbs[skb_id],BUFFER_SIZE);
				skb_reserve(skbs[skb_id],intf.RXshift);

#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
				ptmRx->rx_buffer_addr=((uint32)skbs[skb_id]->head);
				ptmRx->skb_header_addr=((uint32)(unsigned char *)(skbs[skb_id]));
#else //0437 use physical address
				ptmRx->rx_buffer_addr=PHYSICAL_ADDRESS((uint32)skbs[skb_id]->head);
				ptmRx->skb_header_addr=PHYSICAL_ADDRESS((uint32)(unsigned char *)(skbs[skb_id]));
#endif
			}

#ifdef SRAM_MAPPING_ENABLED
#ifdef BUFFER_HDR_IN_SRAM
			ptmRx->sram_en=1;
#else
			ptmRx->sram_en=0;
#endif
#else
			ptmRx->sram_en=0;
#endif
			ptmRx->sram_map_id=sram_id;
			skb_id++;
			sram_id++;
			ptmRx->sram_map_of=sram_map_offset[j];		

		}
	}

#endif

	

	/* ------------------------ EXT PRx ring ---------------------------- */
	for(i=0;i<EXT_PRX_RING_SIZE;i++)
	{
		for(j=0;j<EXT_INTFS;j++)
		{
			struct ext_Rx *extRx;
			extRx=user_ext_prx_idx(j,i);
			extRx->own=1;
			extRx->data_length=READ_MEM32(BUFFERSIZE);
			if(i==EXT_PRX_RING_SIZE-1) 
				extRx->eor=1;
			else
				extRx->eor=0;

			rtl8672_getAsicNetInterface(j+13, &intf);						
			extRx->rx_shift=intf.RXshift;

			if(skbs[skb_id]==NULL) 
			{
//#ifdef CONFIG_USB_RTL8192SU_SOFTAP
#ifdef CONFIG_SKB_POOL_PREALLOC //czyao 0437FPGA
				skbs[skb_id]=prealloc_skb_get();
#else
				skbs[skb_id]=dev_alloc_skb(BUFFER_SIZE);
#endif
				ASSERT(skbs[skb_id]!=NULL);
				init_skb(skbs[skb_id],BUFFER_SIZE);
				skb_reserve(skbs[skb_id],intf.RXshift);

				
				extRx->rx_buffer_addr=(uint32)skbs[skb_id]->head;
				extRx->skb_header_addr=(uint32)(unsigned char *)(skbs[skb_id]);
			}

			extRx->linkid=0;
			skb_id++;

		}	
	}

	// vRx init
	/* ------------------------ MAC VRx ring ---------------------------- */
	for(i=0;i<MAC_VRX_RING_SIZE;i++)
	{

		for(j=0;j<MAC_RX_INTFS;j++)
		{
			struct mac_vRx *macRx;
			macRx=user_mac_vrx_idx(j,i);
			macRx->own=1;
			macRx->data_length=READ_MEM32(BUFFERSIZE);
			if(i==MAC_VRX_RING_SIZE-1) 
				macRx->eor=1;
			else
				macRx->eor=0;

			rtl8672_getAsicNetInterface(j+8, &intf);						
			macRx->rx_shift=intf.RXshift;

			if(skbs[skb_id]==NULL) 
			{
//#ifdef CONFIG_USB_RTL8192SU_SOFTAP
#ifdef CONFIG_SKB_POOL_PREALLOC //czyao 0437FPGA
				skbs[skb_id]=prealloc_skb_get();
#else
				skbs[skb_id]=dev_alloc_skb(BUFFER_SIZE);
#endif
				ASSERT(skbs[skb_id]!=NULL);
				init_skb(skbs[skb_id],BUFFER_SIZE);
				skb_reserve(skbs[skb_id],intf.RXshift);

#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)	
				macRx->rx_buffer_addr=(uint32)skbs[skb_id]->head;				
				macRx->skb_header_addr=(uint32)(unsigned char *)(skbs[skb_id]);
#else
				macRx->rx_buffer_addr=PHYSICAL_ADDRESS(skbs[skb_id]->head); 			
				macRx->skb_header_addr=PHYSICAL_ADDRESS((unsigned char *)(skbs[skb_id]));
#endif

			}
			
			skb_id++;

		}
	}

	/* ------------------------ SAR VRx ring ---------------------------- */
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	for(i=0;i<SAR_VRX_RING_SIZE;i++)
	{

		for(j=0;j<SAR_INTFS;j++)
		{
			struct sar_vRx *sarRx;
			sarRx=user_sar_vrx_idx(j,i);

//			if((j==0)&&(i==(SAR_VRX_RING_SIZE>>1)))  id2=0;
		
			sarRx->own=1;
			sarRx->data_length=READ_MEM32(BUFFERSIZE);
			if(i==SAR_VRX_RING_SIZE-1) 
				sarRx->eor=1;
			else
				sarRx->eor=0;
			
			rtl8672_getAsicNetInterface(j, &intf);						
			sarRx->rx_shift=intf.RXshift;			

			//if(i<(SAR_VRX_RING_SIZE>>1))
			{
				if(skbs[skb_id]==NULL) 
				{
//#ifdef CONFIG_USB_RTL8192SU_SOFTAP
#ifdef CONFIG_SKB_POOL_PREALLOC //czyao 0437FPGA
					skbs[skb_id]=prealloc_skb_get();
#else
					skbs[skb_id]=dev_alloc_skb(BUFFER_SIZE);
#endif
					ASSERT(skbs[skb_id]!=NULL);
					init_skb(skbs[skb_id],BUFFER_SIZE);
					skb_reserve(skbs[skb_id],intf.RXshift);
					
					sarRx->rx_buffer_addr=(uint32)skbs[skb_id]->head;
					sarRx->skb_header_addr=(uint32)(unsigned char *)(skbs[skb_id]);					
				}
			
			}
			/*
			else
			{
				
				if(skbs[skb_id]==NULL) 
				{
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
					skbs[skb_id]=prealloc_skb_get();
#else
					skbs[skb_id]=dev_alloc_skb(BUFFER_SIZE);
#endif
					ASSERT(skbs[skb_id]!=NULL);
					init_skb(skbs[skb_id],BUFFER_SIZE);
					skb_reserve(skbs[skb_id],intf.RXshift);
					
					sarRx->rx_buffer_addr=(uint32)skbs[skb_id]->head;
					sarRx->skb_header_addr=(uint32)(unsigned char *)(skbs[skb_id]);					
				}

			}
			*/


			skb_id++;

		}
	}
#endif
#ifdef CONFIG_RTL8681_PTM
	/* ------------------------ PTM VRx ring ---------------------------- */
	for(i=0;i<PTM_VRX_RING_SIZE;i++)
	{
		for(j=0;j<PTM_INTFS;j++)
		{
			struct mac_vRx *ptmRx;
			ptmRx=user_ptm_vrx_idx(j,i);

//			if((j==0)&&(i==(SAR_VRX_RING_SIZE>>1)))  id2=0;
		
			ptmRx->own=1;
			ptmRx->data_length=READ_MEM32(BUFFERSIZE);
			if(i==PTM_VRX_RING_SIZE-1) 
				ptmRx->eor=1;
			else
				ptmRx->eor=0;
			
			rtl8672_getAsicNetInterface(j, &intf);						
			ptmRx->rx_shift=intf.RXshift;			

			//if(i<(SAR_VRX_RING_SIZE>>1))
			{
				if(skbs[skb_id]==NULL) 
				{
//#ifdef CONFIG_USB_RTL8192SU_SOFTAP
#ifdef CONFIG_SKB_POOL_PREALLOC //czyao 0437FPGA
					skbs[skb_id]=prealloc_skb_get();
#else
					skbs[skb_id]=dev_alloc_skb(BUFFER_SIZE);
#endif
					ASSERT(skbs[skb_id]!=NULL);
					init_skb(skbs[skb_id],BUFFER_SIZE);
					skb_reserve(skbs[skb_id],intf.RXshift);

#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)					
					ptmRx->rx_buffer_addr=(uint32)skbs[skb_id]->head;
					ptmRx->skb_header_addr=(uint32)(unsigned char *)(skbs[skb_id]);					
#else
					ptmRx->rx_buffer_addr=PHYSICAL_ADDRESS((uint32)skbs[skb_id]->head);
					ptmRx->skb_header_addr=PHYSICAL_ADDRESS((uint32)(unsigned char *)(skbs[skb_id]));	
#endif
				}
			
			}
			skb_id++;

		}
	}
#endif

	/* ------------------------ EXT VRx ring ---------------------------- */
	for(i=0;i<EXT_VRX_RING_SIZE;i++)
	{
		for(j=0;j<EXT_INTFS;j++)
		{
			struct ext_Rx *extRx;
			extRx=user_ext_vrx_idx(j,i);
			extRx->own=1;
			extRx->data_length=READ_MEM32(BUFFERSIZE);
			if(i==EXT_VRX_RING_SIZE-1) 
				extRx->eor=1;
			else
				extRx->eor=0;

			rtl8672_getAsicNetInterface(j+13, &intf);						
			extRx->rx_shift=intf.RXshift;

			if(skbs[skb_id]==NULL) 
			{
//#ifdef CONFIG_USB_RTL8192SU_SOFTAP
#ifdef CONFIG_SKB_POOL_PREALLOC //czyao 0437FPGA
				skbs[skb_id]=prealloc_skb_get();
#else
				skbs[skb_id]=dev_alloc_skb(BUFFER_SIZE);
#endif
				ASSERT(skbs[skb_id]!=NULL);
				init_skb(skbs[skb_id],BUFFER_SIZE);
				skb_reserve(skbs[skb_id],intf.RXshift);
				
				extRx->rx_buffer_addr=(uint32)skbs[skb_id]->head;
				extRx->skb_header_addr=(uint32)(unsigned char *)(skbs[skb_id]);				
			}

			extRx->linkid=0;

			skb_id++;
	
		}	
	}

	

	// pTx init	
	/* ------------------------ MAC PTx ring ---------------------------- */
	for(i=0;;i++)
	{
		if((i>=MAC_PTX_RING0_SIZE)&&(i>=MAC_PTX_RING1_SIZE)&&(i>=MAC_PTX_RING2_SIZE)&&(i>=MAC_PTX_RING3_SIZE)&&(i>=MAC_PTX_RING4_SIZE))
			break;

		for(j=0;j<MAC_TX_INTFS;j++)
		{
			struct mac_pTx *macTx;

			switch(j)
			{
				case 0:
					if(i>=MAC_PTX_RING0_SIZE) continue;
					break;
				case 1:
					if(i>=MAC_PTX_RING1_SIZE) continue;
					break;
				case 2:
					if(i>=MAC_PTX_RING2_SIZE) continue;
					break;
				case 3:
					if(i>=MAC_PTX_RING3_SIZE) continue;
					break;
				case 4:
					if(i>=MAC_PTX_RING4_SIZE) continue;
					break;
			}

			macTx=user_mac_ptx_idx(j,i);
			macTx->own=0;


			macTx->eor=0;
			switch(j)
			{
				case 0:					
					if(i==MAC_PTX_RING0_SIZE-1) macTx->eor=1;
					break;
				case 1:					
					if(i==MAC_PTX_RING1_SIZE-1) macTx->eor=1;
					break;
				case 2:					
					if(i==MAC_PTX_RING2_SIZE-1) macTx->eor=1;
					break;
				case 3:					
					if(i==MAC_PTX_RING3_SIZE-1) macTx->eor=1;
					break;
				case 4:					
					if(i==MAC_PTX_RING4_SIZE-1) macTx->eor=1;
					break;				
			}			
			macTx->tx_buffer_addr=(uint32)NULL;
			macTx->orgAddr=(uint32)NULL;		
		}
	}

	/* ------------------------ SAR PTx ring ---------------------------- */
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	for(i=0;i<SAR_PTX_RING_SIZE;i++)
	{
		for(j=0;j<SAR_INTFS;j++)
		{
			struct sar_pTx *sarTx;
			sarTx=user_sar_ptx_idx(j,i);
			
			sarTx->own=0;
			if(i==SAR_PTX_RING_SIZE-1) 
				sarTx->eor=1;
			else
				sarTx->eor=0;

			sarTx->tx_buffer_addr=(uint32)NULL;
			sarTx->orgAddr=(uint32)NULL;	
		}
	}
#endif

	/* ------------------------ PTM PTx ring ---------------------------- */
#ifdef CONFIG_RTL8681_PTM

	for(i=0;;i++)
	{

		if((i>=PTM_PTX_RING0_SIZE)&&(i>=PTM_PTX_RINGx_SIZE))
			break;

		for(j=0;j<PTM_INTFS;j++)
		{
			struct mac_pTx *ptmTx;

			switch(j)
			{
				case 0:
					if(i>=PTM_PTX_RING0_SIZE) continue;
					break;
				default:
					if(i>=PTM_PTX_RINGx_SIZE) continue;
					break;
			}

			ptmTx=user_ptm_ptx_idx(j,i);
			ptmTx->own=0;

			switch(j)
			{
				case 0:					
					if(i==PTM_PTX_RING0_SIZE-1) ptmTx->eor=1;
					break;
				default:					
					if(i==PTM_PTX_RINGx_SIZE-1) ptmTx->eor=1;
					break;			
			}		

			ptmTx->tx_buffer_addr=(uint32)NULL;
			ptmTx->orgAddr=(uint32)NULL;	

		}
	}

#endif

	/* ------------------------ EXT PTx ring ---------------------------- */
	for(i=0;;i++)
	{

		if((i>=EXT_PTX_RING0_SIZE)&&(i>=EXT_PTX_RING1_SIZE)&&(i>=EXT_PTX_RING2_SIZE))
			break;

		for(j=0;j<EXT_INTFS;j++)
		{
			struct ext_Tx *extTx;

			switch(j)
			{
				case 0:
					if(i>=EXT_PTX_RING0_SIZE) continue;
					break;
				case 1:
					if(i>=EXT_PTX_RING1_SIZE) continue;
					break;
				case 2:
					if(i>=EXT_PTX_RING2_SIZE) continue;
					break;
			}
		
			extTx=user_ext_ptx_idx(j,i);
			extTx->own=0;
			extTx->rsv=0;
			extTx->oeor=0;
			extTx->eor=0;
			switch(j)
			{
				case 0:					
					if(i==EXT_PTX_RING0_SIZE-1) extTx->eor=1;
					break;
				case 1:					
					if(i==EXT_PTX_RING1_SIZE-1) extTx->eor=1;
					break;
				case 2:					
					if(i==EXT_PTX_RING2_SIZE-1) extTx->eor=1;
					break;				
			}			

			extTx->tx_buffer_addr=(uint32)NULL;
			extTx->orgAddr=(uint32)NULL;	
			extTx->linkid=0;
		}	
	}



	
	// vTx init
	/* ------------------------ MAC VTx ring ---------------------------- */
	for(j=0;j<MAC_TX_INTFS;j++)
	{	
		for(i=0;i<MAC_VTX_RING_SIZE;i++)
		{
			struct mac_vTx *macTx;
			macTx=user_mac_vtx_idx(j,i);
			macTx->own=0;
			if(i==MAC_VTX_RING_SIZE-1) 
				macTx->eor=1;
			else
				macTx->eor=0;

			macTx->tx_buffer_addr=(uint32)NULL;
			macTx->orgAddr=(uint32)NULL;		
		}
		user_current_mac_vtx[j]=user_mac_vtx_idx(j,0);
	}

	/* ------------------------ SAR VTx ring ---------------------------- */
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	for(j=0;j<SAR_INTFS;j++)
	{
		for(i=0;i<SAR_VTX_RING_SIZE;i++)
		{
			struct sar_vTx *sarTx;
			sarTx=user_sar_vtx_idx(j,i);
			sarTx->own=0;
			if(i==SAR_VTX_RING_SIZE-1) 
				sarTx->eor=1;
			else
				sarTx->eor=0;

			sarTx->tx_buffer_addr=(uint32)NULL;
			sarTx->orgAddr=(uint32)NULL;	
		}
		user_current_sar_vtx[j]=user_sar_vtx_idx(j,0);		
	}
#endif

	/* ------------------------ PTM VTx ring ---------------------------- */
#ifdef CONFIG_RTL8681_PTM
	for(j=0;j<PTM_INTFS;j++)
	{
		for(i=0;i<PTM_VTX_RING_SIZE;i++)
		{
			struct mac_vTx *ptmTx;
			ptmTx=user_ptm_vtx_idx(j,i);
			ptmTx->own=0;
			if(i==PTM_VTX_RING_SIZE-1) 
				ptmTx->eor=1;
			else
				ptmTx->eor=0;

			ptmTx->tx_buffer_addr=(uint32)NULL;
			ptmTx->orgAddr=(uint32)NULL;	
		}
		user_current_ptm_vtx[j]=user_ptm_vtx_idx(j,0);		
	}
#endif

	/* ------------------------ EXT VTx ring ---------------------------- */
	for(j=0;j<EXT_INTFS;j++)
	{
		for(i=0;i<EXT_VTX_RING_SIZE;i++)
		{	
			struct ext_Tx *extTx;
			extTx=user_ext_vtx_idx(j,i);
			
			extTx->own=0;
			if(i==EXT_VTX_RING_SIZE-1) 
				extTx->eor=1;
			else
				extTx->eor=0;

			extTx->tx_buffer_addr=(uint32)NULL;
			extTx->skb_header_addr=(uint32)NULL;
			extTx->orgAddr=(uint32)NULL;	
			extTx->linkid=0;
		}
		user_current_ext_vtx[j]=user_ext_vtx_idx(j,0);
	}



	for(j=0;j<EXT_INTFS;j++)
	{
		cpu_ext_vrx_idx[j]=0;
		cpu_ext_ptx_idx[j]=0;
		cpu_ext_vtx_idx[j]=0;

	}

#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	for(j=0;j<SAR_INTFS;j++)
	{
		int k;
		cpu_sar_vrx_idx[j]=0;
		cpu_sar_vtx_idx[j]=0;
		for(k=0;k<SAR_VTX_RING_SIZE;k++){
			cpu_sar_vtx_skb[j][k]=0;
		}
	}
#endif

#ifdef CONFIG_RTL8681_PTM
	for(j=0;j<PTM_INTFS;j++)
	{
		int k;
		cpu_ptm_vrx_idx[j]=0;
		cpu_ptm_vtx_idx[j]=0;
		for(k=0;k<PTM_VTX_RING_SIZE;k++){
			cpu_ptm_vtx_skb[j][k]=0;
		}
	}
#endif

	for(j=0;j<MAC_RX_INTFS;j++)
	{
		cpu_mac_vrx_idx[j]=0;
	}

	for(j=0;j<MAC_TX_INTFS;j++)
	{	
		int k;
		cpu_mac_vtx_idx[j]=0;
		for(k=0;k<MAC_VTX_RING_SIZE;k++)
		{
			cpu_mac_vtx_skb[j][k]=0;
		}
	}	

	if(max_sram_id>512)
	{
		rtlglue_printf("ERROR: SRAM_ID out of range, max_sram_id(%d) is > 512!!\n",max_sram_id);
		while(1);
		return FAILED;
	}
	rtlglue_printf("INFO: max_sram_id(%d)\n",max_sram_id);

	if(max_sram_id!=sram_id)
	{
		rtlglue_printf("ERROR: max_sram_id(%d) != sram_id(%d)!!\n",max_sram_id,sram_id);
		while(1);
		return FAILED;
	}
	
	if(skb_id!=MAX_SKB_ID)
	{
		rtlglue_printf("ERROR: MAX_SKB_ID(%d) != skb_id(%d)!!\n",MAX_SKB_ID,skb_id);
		while(1);
		return FAILED;
	}
	
	//WRITE_MEM32(PPCR,PPCR_ENABLE|PPCR_START|max_sram_id);
#if defined(CONFIG_RLE0437) || defined(CONFIG_RTL8681)
	WRITE_MEM32(SRAMMAPADDR,(uint32)PHYSICAL_ADDRESS(sram_buf));
#else
	WRITE_MEM32(SRAMMAPADDR,(uint32)sram_buf);
#endif
	WRITE_MEM32(PENDINGRING,0);

	/* ------------------------ SAR PRx Weighting ---------------------------- */
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	if(SAR_PRX_RING_SIZE==0)
	{
		WRITE_MEM32(DESCWT0,0x0);	
		WRITE_MEM32(DESCWT1,0x0);		
	}
	else
	{
		WRITE_MEM32(DESCWT0,0x01010101);	
		WRITE_MEM32(DESCWT1,0x01010101);	
	}
#endif

	/* ------------------------ PTM PRx Weighting ---------------------------- */
#ifdef CONFIG_RTL8681_PTM

	if(PTM_PRX_RING0_SIZE==0 && PTM_PRX_RINGx_SIZE==0)
	{
		WRITE_MEM32(DESCWT0,0x0);	
		WRITE_MEM32(DESCWT1,0x0);		
	}
	else
	{
		WRITE_MEM32(DESCWT0,0x01010101);	
		WRITE_MEM32(DESCWT1,0x01010101);	
	}
#endif

	/* ------------------------ MAC PRx Weighting ---------------------------- */
#ifdef EXT_PHY	
	WRITE_MEM32(DESCWT2,0x01);		 
#else
	//WRITE_MEM32(DESCWT2,0x01010101);		 
	WRITE_MEM32(DESCWT2,0x100);		//disable MAC0 pRx, EXT0, EXT1 pRx, enabled MAC1
#endif
	WRITE_MEM32(DESCWT3,0x01);		

	/* ------------------------ SAR VTx Weighting ---------------------------- */
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	if(SAR_VTX_RING_SIZE==0)
	{
		WRITE_MEM32(DESCWT4,0x0);			
		WRITE_MEM32(DESCWT5,0x0);	
	}
	else
	{
	WRITE_MEM32(DESCWT4,0x01010101);			
	WRITE_MEM32(DESCWT5,0x01010101);			
	}
#endif

	/* ------------------------ PTM VTx Weighting ---------------------------- */
#ifdef CONFIG_RTL8681_PTM
	if(PTM_VTX_RING_SIZE==0)
	{
		WRITE_MEM32(DESCWT4,0x0);			
		WRITE_MEM32(DESCWT5,0x0);	
	}
	else
	{
		WRITE_MEM32(DESCWT4,0x01010101);			
		WRITE_MEM32(DESCWT5,0x01010101);			
	}
#endif

	/* ------------------------ EXT VTx Weighting ---------------------------- */
#ifdef EXT_PHY	
	WRITE_MEM32(DESCWT6,0x01);			
#else
	//WRITE_MEM32(DESCWT6,0x01010101);			
	//WRITE_MEM32(DESCWT7,0x01010101);
	WRITE_MEM32(DESCWT6,0x100);	//disable MAC0,2,3 vTx
#endif
	WRITE_MEM32(DESCWT7,0x01000000);	//disable MAC4 vTx , EXT0,1 vTx

	
#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	WRITE_MEM32(SVRXDESC0,(uint32)user_sar_vrx_idx(0,0));
	WRITE_MEM32(SVRXDESC1,(uint32)user_sar_vrx_idx(1,0));		
	WRITE_MEM32(SVRXDESC2,(uint32)user_sar_vrx_idx(2,0));		
	WRITE_MEM32(SVRXDESC3,(uint32)user_sar_vrx_idx(3,0));		
	WRITE_MEM32(SVRXDESC4,(uint32)user_sar_vrx_idx(4,0));		
	WRITE_MEM32(SVRXDESC5,(uint32)user_sar_vrx_idx(5,0));		
	WRITE_MEM32(SVRXDESC6,(uint32)user_sar_vrx_idx(6,0));		
	WRITE_MEM32(SVRXDESC7,(uint32)user_sar_vrx_idx(7,0));			

	WRITE_MEM32(SVTXDESC0,(uint32)user_sar_vtx_idx(0,0));
	WRITE_MEM32(SVTXDESC1,(uint32)user_sar_vtx_idx(1,0));
	WRITE_MEM32(SVTXDESC2,(uint32)user_sar_vtx_idx(2,0));	
	WRITE_MEM32(SVTXDESC3,(uint32)user_sar_vtx_idx(3,0));
	WRITE_MEM32(SVTXDESC4,(uint32)user_sar_vtx_idx(4,0));
	WRITE_MEM32(SVTXDESC5,(uint32)user_sar_vtx_idx(5,0));
	WRITE_MEM32(SVTXDESC6,(uint32)user_sar_vtx_idx(6,0));
	WRITE_MEM32(SVTXDESC7,(uint32)user_sar_vtx_idx(7,0));

	WRITE_MEM32(SPRXDESC0,(uint32)user_sar_prx_idx(0,0));
	WRITE_MEM32(SPRXDESC1,(uint32)user_sar_prx_idx(1,0));
	WRITE_MEM32(SPRXDESC2,(uint32)user_sar_prx_idx(2,0));
	WRITE_MEM32(SPRXDESC3,(uint32)user_sar_prx_idx(3,0));	
	WRITE_MEM32(SPRXDESC4,(uint32)user_sar_prx_idx(4,0));	
	WRITE_MEM32(SPRXDESC5,(uint32)user_sar_prx_idx(5,0));	
	WRITE_MEM32(SPRXDESC6,(uint32)user_sar_prx_idx(6,0));	
	WRITE_MEM32(SPRXDESC7,(uint32)user_sar_prx_idx(7,0));

	WRITE_MEM32(SPTXDESC0,(uint32)user_sar_ptx_idx(0,0));
	WRITE_MEM32(SPTXDESC1,(uint32)user_sar_ptx_idx(1,0));
	WRITE_MEM32(SPTXDESC2,(uint32)user_sar_ptx_idx(2,0));	
	WRITE_MEM32(SPTXDESC3,(uint32)user_sar_ptx_idx(3,0));	
	WRITE_MEM32(SPTXDESC4,(uint32)user_sar_ptx_idx(4,0));	
	WRITE_MEM32(SPTXDESC5,(uint32)user_sar_ptx_idx(5,0));	
	WRITE_MEM32(SPTXDESC6,(uint32)user_sar_ptx_idx(6,0));	
	WRITE_MEM32(SPTXDESC7,(uint32)user_sar_ptx_idx(7,0));
#endif

#ifdef CONFIG_RTL8681_PTM
	WRITE_MEM32(SVRXDESC0,PHYSICAL_ADDRESS((uint32)user_ptm_vrx_idx(0,0)));
	//printk("PTM VRX 0 : 0x%08X\n",(uint32)user_ptm_vrx_idx(0,0));
	WRITE_MEM32(SVRXDESC1,PHYSICAL_ADDRESS((uint32)user_ptm_vrx_idx(1,0)));
	//printk("PTM VRX 1 : 0x%08X\n",(uint32)user_ptm_vrx_idx(1,0));
	WRITE_MEM32(SVRXDESC2,PHYSICAL_ADDRESS((uint32)user_ptm_vrx_idx(2,0)));		
	//printk("PTM VRX 2 : 0x%08X\n",(uint32)user_ptm_vrx_idx(2,0));
	WRITE_MEM32(SVRXDESC3,PHYSICAL_ADDRESS((uint32)user_ptm_vrx_idx(3,0)));		
	//printk("PTM VRX 3 : 0x%08X\n",(uint32)user_ptm_vrx_idx(3,0));
	WRITE_MEM32(SVRXDESC4,PHYSICAL_ADDRESS((uint32)user_ptm_vrx_idx(4,0)));		
	//printk("PTM VRX 4 : 0x%08X\n",(uint32)user_ptm_vrx_idx(4,0));
	WRITE_MEM32(SVRXDESC5,PHYSICAL_ADDRESS((uint32)user_ptm_vrx_idx(5,0)));		
	//printk("PTM VRX 5 : 0x%08X\n",(uint32)user_ptm_vrx_idx(5,0));
	WRITE_MEM32(SVRXDESC6,PHYSICAL_ADDRESS((uint32)user_ptm_vrx_idx(6,0)));		
	//printk("PTM VRX 6 : 0x%08X\n",(uint32)user_ptm_vrx_idx(6,0));
	WRITE_MEM32(SVRXDESC7,PHYSICAL_ADDRESS((uint32)user_ptm_vrx_idx(7,0)));			
	//printk("PTM VRX 7 : 0x%08X\n",(uint32)user_ptm_vrx_idx(7,0));

	WRITE_MEM32(SVTXDESC0,PHYSICAL_ADDRESS((uint32)user_ptm_vtx_idx(0,0)));
	//printk("PTM VTX 0 : 0x%08X\n",(uint32)user_ptm_vtx_idx(0,0));
	WRITE_MEM32(SVTXDESC1,PHYSICAL_ADDRESS((uint32)user_ptm_vtx_idx(1,0)));
	//printk("PTM VTX 1 : 0x%08X\n",(uint32)user_ptm_vtx_idx(1,0));
	WRITE_MEM32(SVTXDESC2,PHYSICAL_ADDRESS((uint32)user_ptm_vtx_idx(2,0)));	
	//printk("PTM VTX 2 : 0x%08X\n",(uint32)user_ptm_vtx_idx(2,0));
	WRITE_MEM32(SVTXDESC3,PHYSICAL_ADDRESS((uint32)user_ptm_vtx_idx(3,0)));
	//printk("PTM VTX 3 : 0x%08X\n",(uint32)user_ptm_vtx_idx(3,0));
	WRITE_MEM32(SVTXDESC4,PHYSICAL_ADDRESS((uint32)user_ptm_vtx_idx(4,0)));
	//printk("PTM VTX 4 : 0x%08X\n",(uint32)user_ptm_vtx_idx(4,0));
	WRITE_MEM32(SVTXDESC5,PHYSICAL_ADDRESS((uint32)user_ptm_vtx_idx(5,0)));
	//printk("PTM VTX 5 : 0x%08X\n",(uint32)user_ptm_vtx_idx(5,0));
	WRITE_MEM32(SVTXDESC6,PHYSICAL_ADDRESS((uint32)user_ptm_vtx_idx(6,0)));
	//printk("PTM VTX 6 : 0x%08X\n",(uint32)user_ptm_vtx_idx(6,0));
	WRITE_MEM32(SVTXDESC7,PHYSICAL_ADDRESS((uint32)user_ptm_vtx_idx(7,0)));
	//printk("PTM VTX 7 : 0x%08X\n",(uint32)user_ptm_vtx_idx(7,0));

#if 0  //0437 setting
	WRITE_MEM32(SPRXDESC0,PHYSICAL_ADDRESS((uint32)user_ptm_prx_idx(1,0)));
	WRITE_MEM32(SPRXDESC1,PHYSICAL_ADDRESS((uint32)user_ptm_prx_idx(0,0)));
	WRITE_MEM32(SPRXDESC2,PHYSICAL_ADDRESS((uint32)user_ptm_prx_idx(3,0)));
	WRITE_MEM32(SPRXDESC3,PHYSICAL_ADDRESS((uint32)user_ptm_prx_idx(2,0)));	
	WRITE_MEM32(SPRXDESC4,PHYSICAL_ADDRESS((uint32)user_ptm_prx_idx(4,0)));	
	WRITE_MEM32(SPRXDESC5,PHYSICAL_ADDRESS((uint32)user_ptm_prx_idx(5,0)));	
	WRITE_MEM32(SPRXDESC6,PHYSICAL_ADDRESS((uint32)user_ptm_prx_idx(6,0)));	
	WRITE_MEM32(SPRXDESC7,PHYSICAL_ADDRESS((uint32)user_ptm_prx_idx(7,0)));
#else //0513 setting !
	WRITE_MEM32(SPRXDESC0,PHYSICAL_ADDRESS((uint32)user_ptm_prx_idx(0,0)));
	WRITE_MEM32(SPRXDESC1,PHYSICAL_ADDRESS((uint32)user_ptm_prx_idx(1,0)));
	WRITE_MEM32(SPRXDESC2,PHYSICAL_ADDRESS((uint32)user_ptm_prx_idx(2,0)));
	WRITE_MEM32(SPRXDESC3,PHYSICAL_ADDRESS((uint32)user_ptm_prx_idx(3,0)));	
	WRITE_MEM32(SPRXDESC4,PHYSICAL_ADDRESS((uint32)user_ptm_prx_idx(4,0)));	
	WRITE_MEM32(SPRXDESC5,PHYSICAL_ADDRESS((uint32)user_ptm_prx_idx(5,0)));	
	WRITE_MEM32(SPRXDESC6,PHYSICAL_ADDRESS((uint32)user_ptm_prx_idx(6,0)));	
	WRITE_MEM32(SPRXDESC7,PHYSICAL_ADDRESS((uint32)user_ptm_prx_idx(7,0)));
#endif

	WRITE_MEM32(SPTXDESC0,PHYSICAL_ADDRESS((uint32)user_ptm_ptx_idx(0,0)));
	WRITE_MEM32(SPTXDESC1,PHYSICAL_ADDRESS((uint32)user_ptm_ptx_idx(1,0)));
	WRITE_MEM32(SPTXDESC2,PHYSICAL_ADDRESS((uint32)user_ptm_ptx_idx(2,0)));	
	WRITE_MEM32(SPTXDESC3,PHYSICAL_ADDRESS((uint32)user_ptm_ptx_idx(3,0)));	
	WRITE_MEM32(SPTXDESC4,PHYSICAL_ADDRESS((uint32)user_ptm_ptx_idx(4,0)));	
	WRITE_MEM32(SPTXDESC5,PHYSICAL_ADDRESS((uint32)user_ptm_ptx_idx(5,0)));	
	WRITE_MEM32(SPTXDESC6,PHYSICAL_ADDRESS((uint32)user_ptm_ptx_idx(6,0)));	
	WRITE_MEM32(SPTXDESC7,PHYSICAL_ADDRESS((uint32)user_ptm_ptx_idx(7,0)));
#endif

	/* clear all SAR counter, same method as PTM */
	WRITE_MEM32(SVRXRECVCNT0,1);
	WRITE_MEM32(SVRXRECVCNT1,1);
	WRITE_MEM32(SVRXRECVCNT2,1);
	WRITE_MEM32(SVRXRECVCNT3,1);
	WRITE_MEM32(SVRXRECVCNT4,1);
	WRITE_MEM32(SVRXRECVCNT5,1);
	WRITE_MEM32(SVRXRECVCNT6,1);
	WRITE_MEM32(SVRXRECVCNT7,1);

	WRITE_MEM32(SVTXFWDCNT0,1);
	WRITE_MEM32(SVTXFWDCNT1,1);
	WRITE_MEM32(SVTXFWDCNT2,1);
	WRITE_MEM32(SVTXFWDCNT3,1);
	WRITE_MEM32(SVTXFWDCNT4,1);
	WRITE_MEM32(SVTXFWDCNT5,1);
	WRITE_MEM32(SVTXFWDCNT6,1);
	WRITE_MEM32(SVTXFWDCNT7,1);

	WRITE_MEM32(SPRXDROPCNT0,1);
	WRITE_MEM32(SPRXDROPCNT1,1);
	WRITE_MEM32(SPRXDROPCNT2,1);
	WRITE_MEM32(SPRXDROPCNT3,1);
	WRITE_MEM32(SPRXDROPCNT4,1);
	WRITE_MEM32(SPRXDROPCNT5,1);
	WRITE_MEM32(SPRXDROPCNT6,1);
	WRITE_MEM32(SPRXDROPCNT7,1);

	WRITE_MEM32(MVRXDESC0,(uint32)user_mac_vrx_idx(0,0));
//printk("vrx for nic 0 addr=%x\n",(uint32)user_mac_vrx_idx(0,0));	
	WRITE_MEM32(MVRXDESC1,(uint32)user_mac_vrx_idx(1,0));

//printk("vrx for nic 1 addr=%x\n",(uint32)user_mac_vrx_idx(1,0));
	WRITE_MEM32(MVTXDESC0,(uint32)user_mac_vtx_idx(0,0));
	WRITE_MEM32(MVTXDESC1,(uint32)user_mac_vtx_idx(1,0));
	WRITE_MEM32(MVTXDESC2,(uint32)user_mac_vtx_idx(2,0));
	WRITE_MEM32(MVTXDESC3,(uint32)user_mac_vtx_idx(3,0));
	WRITE_MEM32(MVTXDESC4,(uint32)user_mac_vtx_idx(4,0));
//printk("prx for nic 0 addr=%x\n",(uint32)user_mac_prx_idx(0,0));		
	WRITE_MEM32(MPRXDESC0,(uint32)user_mac_prx_idx(0,0));	
//printk("prx for nic 1 addr=%x\n",(uint32)user_mac_prx_idx(1,0));	
	WRITE_MEM32(MPRXDESC1,(uint32)user_mac_prx_idx(1,0));		
	WRITE_MEM32(MPTXDESC0,(uint32)user_mac_ptx_idx(0,0));		
//printk("ptx for nic 0 addr=%x\n",(uint32)user_mac_ptx_idx(0,0));		
	WRITE_MEM32(MPTXDESC1,(uint32)user_mac_ptx_idx(1,0));
//printk("ptx for nic 1 addr=%x\n",(uint32)user_mac_ptx_idx(1,0));	
	WRITE_MEM32(MPTXDESC2,(uint32)user_mac_ptx_idx(2,0));
//printk("ptx for nic 2 addr=%x\n",(uint32)user_mac_ptx_idx(2,0));		
	WRITE_MEM32(MPTXDESC3,(uint32)user_mac_ptx_idx(3,0));
//printk("ptx for nic 3 addr=%x\n",(uint32)user_mac_ptx_idx(3,0));			
	WRITE_MEM32(MPTXDESC4,(uint32)user_mac_ptx_idx(4,0));
//printk("ptx for nic 4 addr=%x\n",(uint32)user_mac_ptx_idx(4,0));			

	/* clear all MAC counter */
	WRITE_MEM32(MVRXRECVCNT0,1);
	WRITE_MEM32(MVRXRECVCNT1,1);

	WRITE_MEM32(MVTXFWDCNT0,1);
	WRITE_MEM32(MVTXFWDCNT1,1);
	WRITE_MEM32(MVTXFWDCNT2,1);
	WRITE_MEM32(MVTXFWDCNT3,1);
	WRITE_MEM32(MVTXFWDCNT4,1);

	WRITE_MEM32(MPRXDROPCNT0,1);
	WRITE_MEM32(MPRXDROPCNT1,1);


	WRITE_MEM32(EVRXDESC0,(uint32)user_ext_vrx_idx(0,0));
	WRITE_MEM32(EVRXDESC1,(uint32)user_ext_vrx_idx(1,0));	
	WRITE_MEM32(EVRXDESC2,(uint32)user_ext_vrx_idx(2,0));		
	WRITE_MEM32(EVTXDESC0,(uint32)user_ext_vtx_idx(0,0));
	WRITE_MEM32(EVTXDESC1,(uint32)user_ext_vtx_idx(1,0));	
	WRITE_MEM32(EVTXDESC2,(uint32)user_ext_vtx_idx(2,0));			

	WRITE_MEM32(EPRXDESC0,(uint32)user_ext_prx_idx(0,0));	
	WRITE_MEM32(EPRXDESC1,(uint32)user_ext_prx_idx(1,0));	
	WRITE_MEM32(EPRXDESC2,(uint32)user_ext_prx_idx(2,0));		
	WRITE_MEM32(EPTXDESC0,(uint32)user_ext_ptx_idx(0,0));	
	WRITE_MEM32(EPTXDESC1,(uint32)user_ext_ptx_idx(1,0));	
	WRITE_MEM32(EPTXDESC2,(uint32)user_ext_ptx_idx(2,0));	




	/* clear all SAR counter */
	WRITE_MEM32(EVRXRECVCNT0,1);
	WRITE_MEM32(EVRXRECVCNT1,1);
	WRITE_MEM32(EVRXRECVCNT2,1);

	WRITE_MEM32(EVTXFWDCNT0,1);
	WRITE_MEM32(EVTXFWDCNT1,1);
	WRITE_MEM32(EVTXFWDCNT2,1);

	WRITE_MEM32(EPRXDROPCNT0,1);
	WRITE_MEM32(EPRXDROPCNT1,1);
	WRITE_MEM32(EPRXDROPCNT2,1);

	/* clear all share_pool counter */
	WRITE_MEM32(SPSAR0FWDCNT,1);
	WRITE_MEM32(SPSAR1FWDCNT,1);
	WRITE_MEM32(SPSAR2FWDCNT,1);
	WRITE_MEM32(SPSAR3FWDCNT,1);
	WRITE_MEM32(SPSAR4FWDCNT,1);
	WRITE_MEM32(SPSAR5FWDCNT,1);
	WRITE_MEM32(SPSAR6FWDCNT,1);
	WRITE_MEM32(SPSAR7FWDCNT,1);	
	WRITE_MEM32(SPMAC0FWDCNT,1);
	WRITE_MEM32(SPMAC1FWDCNT,1);
	WRITE_MEM32(SPMAC2FWDCNT,1);
	WRITE_MEM32(SPMAC3FWDCNT,1);
	WRITE_MEM32(SPMAC4FWDCNT,1);
	WRITE_MEM32(SPEXT0FWDCNT,1);
	WRITE_MEM32(SPEXT1FWDCNT,1);
	WRITE_MEM32(SPEXT2FWDCNT,1);
	


	/* enable all interrupts */
	//WRITE_MEM32(SARIMR,SAR_VTDF|SAR_RDA|SAR_RBF);
	WRITE_MEM32(SARIMR,SAR_RDA|SAR_RBF);


#ifdef SAR_LLIP_SIM
	WRITE_MEM32(SARIMR,READ_MEM32(SARIMR)|SAR_PTDF); /* just for Packet Processor IP verify */
#endif


	WRITE_MEM32(MAC0IMR,MAC_ROK|MAC_RER_OVF|MAC_VTOK|MAC_TDU|MAC_SWINT);	
	WRITE_MEM32(MAC1IMR,MAC_ROK|MAC_RER_OVF|MAC_VTOK|MAC_TDU|MAC_SWINT);	



#ifdef MAC_LLIP_SIM
	 /* just for Packet Processor IP verify */
	WRITE_MEM32(MAC0IMR,READ_MEM32(MAC0IMR)|MAC_PTOK0);
	WRITE_MEM32(MAC1IMR,READ_MEM32(MAC1IMR)|MAC_PTOK0|MAC_PTOK1|MAC_PTOK2|MAC_PTOK3); 
#endif	

	
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
	WRITE_MEM32(EPIMR,EXT0_VRX_OK|EXT1_VRX_OK|EXT2_VRX_OK|EXT0_PTX_OK|EXT1_PTX_OK|EXT2_PTX_OK|
						EXT0_VRX_FULL|EXT1_VRX_FULL|EXT2_VRX_FULL|EXT0_VTX_EMPTY|EXT1_VTX_EMPTY|EXT2_VTX_EMPTY|
						EXT0_PTX_FULL|EXT1_PTX_FULL|EXT2_PTX_FULL);
#else
	WRITE_MEM32(EPIMR,0);
#endif	
	WRITE_MEM32(EPISR,EXT0_VRX_OK|EXT1_VRX_OK|EXT2_VRX_OK|EXT0_PTX_OK|EXT1_PTX_OK|EXT2_PTX_OK|
						EXT0_VRX_FULL|EXT1_VRX_FULL|EXT2_VRX_FULL|EXT0_VTX_EMPTY|EXT1_VTX_EMPTY|EXT2_VTX_EMPTY|
						EXT0_PTX_FULL|EXT1_PTX_FULL|EXT2_PTX_FULL);


#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	WRITE_MEM32(SPSAR0DESC,(uint32)user_sp_to_sar_idx(0,0));
	WRITE_MEM32(SPSAR1DESC,(uint32)user_sp_to_sar_idx(1,0));
	WRITE_MEM32(SPSAR2DESC,(uint32)user_sp_to_sar_idx(2,0));
	WRITE_MEM32(SPSAR3DESC,(uint32)user_sp_to_sar_idx(3,0));
	WRITE_MEM32(SPSAR4DESC,(uint32)user_sp_to_sar_idx(4,0));
	WRITE_MEM32(SPSAR5DESC,(uint32)user_sp_to_sar_idx(5,0));
	WRITE_MEM32(SPSAR6DESC,(uint32)user_sp_to_sar_idx(6,0));
	WRITE_MEM32(SPSAR7DESC,(uint32)user_sp_to_sar_idx(7,0));
#endif

#ifdef CONFIG_RTL8681_PTM
#if 0
	WRITE_MEM32(SPSAR0DESC,(uint32)user_sp_to_ptm_idx(0,0));
	WRITE_MEM32(SPSAR1DESC,(uint32)user_sp_to_ptm_idx(1,0));
	WRITE_MEM32(SPSAR2DESC,(uint32)user_sp_to_ptm_idx(2,0));
	WRITE_MEM32(SPSAR3DESC,(uint32)user_sp_to_ptm_idx(3,0));
	WRITE_MEM32(SPSAR4DESC,(uint32)user_sp_to_ptm_idx(4,0));
	WRITE_MEM32(SPSAR5DESC,(uint32)user_sp_to_ptm_idx(5,0));
	WRITE_MEM32(SPSAR6DESC,(uint32)user_sp_to_ptm_idx(6,0));
	WRITE_MEM32(SPSAR7DESC,(uint32)user_sp_to_ptm_idx(7,0));
#else
	WRITE_MEM32(SPSAR0DESC,PHYSICAL_ADDRESS(user_sp_to_ptm_idx(0,0)));
	WRITE_MEM32(SPSAR1DESC,PHYSICAL_ADDRESS(user_sp_to_ptm_idx(1,0)));
	WRITE_MEM32(SPSAR2DESC,PHYSICAL_ADDRESS(user_sp_to_ptm_idx(2,0)));
	WRITE_MEM32(SPSAR3DESC,PHYSICAL_ADDRESS(user_sp_to_ptm_idx(3,0)));
	WRITE_MEM32(SPSAR4DESC,PHYSICAL_ADDRESS(user_sp_to_ptm_idx(4,0)));
	WRITE_MEM32(SPSAR5DESC,PHYSICAL_ADDRESS(user_sp_to_ptm_idx(5,0)));
	WRITE_MEM32(SPSAR6DESC,PHYSICAL_ADDRESS(user_sp_to_ptm_idx(6,0)));
	WRITE_MEM32(SPSAR7DESC,PHYSICAL_ADDRESS(user_sp_to_ptm_idx(7,0)));
#endif
#endif

#if 0
	WRITE_MEM32(SPMAC0DESC,(uint32)user_sp_to_mac_idx(0,0));
	WRITE_MEM32(SPMAC1DESC,(uint32)user_sp_to_mac_idx(1,0));
	WRITE_MEM32(SPMAC2DESC,(uint32)user_sp_to_mac_idx(2,0));
	WRITE_MEM32(SPMAC3DESC,(uint32)user_sp_to_mac_idx(3,0));
	WRITE_MEM32(SPMAC4DESC,(uint32)user_sp_to_mac_idx(4,0));
#else
	WRITE_MEM32(SPMAC0DESC,PHYSICAL_ADDRESS(user_sp_to_mac_idx(0,0)));
	WRITE_MEM32(SPMAC1DESC,PHYSICAL_ADDRESS(user_sp_to_mac_idx(1,0)));
	WRITE_MEM32(SPMAC2DESC,PHYSICAL_ADDRESS(user_sp_to_mac_idx(2,0)));
	WRITE_MEM32(SPMAC3DESC,PHYSICAL_ADDRESS(user_sp_to_mac_idx(3,0)));
	WRITE_MEM32(SPMAC4DESC,PHYSICAL_ADDRESS(user_sp_to_mac_idx(4,0)));
#endif

	WRITE_MEM32(SPEXT0DESC,(uint32)user_sp_to_ext_idx(0,0));
	WRITE_MEM32(SPEXT1DESC,(uint32)user_sp_to_ext_idx(1,0));
	WRITE_MEM32(SPEXT2DESC,(uint32)user_sp_to_ext_idx(2,0));


	for(i=0;i<EXT_INTFS;i++)
	{
		current_ext_prx[i]=user_ext_prx_idx(i,0);
		current_ext_ptx[i]=user_ext_ptx_idx(i,0);
		current_sp_to_ext[i]=user_sp_to_ext_idx(i,0);
		pp_ext_prx[i]=user_ext_prx_idx(i,0);
		pp_ext_ptx[i]=user_ext_ptx_idx(i,0);
		pp_ext_vrx[i]=user_ext_vrx_idx(i,0);
		pp_ext_vtx[i]=user_ext_vtx_idx(i,0);
		pp_sp_to_ext[i]=user_sp_to_ext_idx(i,0);
	}

	init_ale();

#if defined(RTL867X_MODEL_KERNEL) && (!defined(MAC_LLIP_SIM))
	init_nic();
#ifdef CONFIG_RTL8681_PTM
	init_ptm();
#endif
#endif


#ifdef MAC_LLIP_SIM
		rtlglue_printf("MAC_LLIP_SIM is enabled!\n");
#else
		rtlglue_printf("MAC_LLIP_SIM is disabled!\n");
#endif

#ifdef SAR_LLIP_SIM
		rtlglue_printf("SAR_LLIP_SIM is enabled!\n");
#else
		rtlglue_printf("SAR_LLIP_SIM is disabled!\n");
#endif

#ifdef EXT_LLIP_SIM
		rtlglue_printf("EXT_LLIP_SIM is enabled!\n");
#else
		rtlglue_printf("EXT_LLIP_SIM is disabled!\n");
#endif

#ifdef SRAM_MAPPING_ENABLED
		rtlglue_printf("SRAM_MAPPING_ENABLED is enabled, total_used_sram_size=%d!\n",total_used_sram_size);
#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
		WRITE_MEM32(PPCR,PPCR_ENABLE|PPCR_START|max_sram_id);
#else
		WRITE_MEM32(PPCR,PPCR_ENABLE|PPCR_START|PPCR_RESET|max_sram_id);
#endif

#else
		rtlglue_printf("SRAM_MAPPING_ENABLED is disabled!\n");
#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
		WRITE_MEM32(PPCR,PPCR_ENABLE|PPCR_START);
#else
		WRITE_MEM32(PPCR,PPCR_ENABLE|PPCR_START|PPCR_RESET);
#endif
#endif

	
#ifdef EXT_PHY
	mdelay(5);
	//external PHY need this reset	
	WRITE_MEM32(NIC100MII_BASE+0x5c,0x94001200); 
#endif
	init_table_timeout_timer();
	return SUCCESS;
	
	
}
int MCAST_MAC(unsigned char *da)
{
	if ((*da) & 0x01)
		return TRUE;
	else
		return FALSE;
}

#if 0
/*
@func void   | rtl8672_SMac_learning 
@parm uint8* | smac    | source mac address
@parm uint8* | apmac   | AP mac address
@parm uint16 | myvid   | used VLAN ID
@parm uint32 | portidx | extension port index

Do source MAC learning for extension devices.
*/

int rtl8672_SMac_learning(uint8 *smac, uint8 *apmac, uint16 myvid, uint32 portidx)
{
	uint32							dmac_idx;
	int32							retval;
	rtl8672_tblAsicDrv_l2Param_t	l2p;
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
	return SUCCESS;
#endif
	if(MCAST_MAC(smac)) return SUCCESS; /* multicast or broadcast address is not need to learn. */
	if(rtl8672_L2EntryLookup(smac,myvid,&l2p)==SUCCESS) return SUCCESS; //entry already exist 

	bzero(&l2p, sizeof(rtl8672_tblAsicDrv_l2Param_t));
	/* SMAC Learning  */
	l2p.MAC[0] = smac[0];
	l2p.MAC[1] = smac[1];
	l2p.MAC[2] = smac[2];
	l2p.MAC[3] = smac[3];
	l2p.MAC[4] = smac[4];
	l2p.MAC[5] = smac[5];

	l2p.APMAC[0] = apmac[0];
	l2p.APMAC[1] = apmac[1];
	l2p.APMAC[2] = apmac[2];
	l2p.APMAC[3] = apmac[3];
	l2p.APMAC[4] = apmac[4];
	l2p.APMAC[5] = apmac[5];

	l2p.VlanID=myvid;
	l2p.Port=portidx;
	l2p.Valid=1;
	dmac_idx = rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID);
	retval = rtl8672_setAsicL2Table_Sram(dmac_idx, 0, &l2p);
	return retval;
	//ASSERT(retval == SUCCESS);
}
#endif

__IRAM_PP_MIDDLE struct sk_buff *rtl8672_extPortRecv(void *id, uint8 *data,  uint32 len, uint16 myvid, uint32 portidx, uint32 linkID)
{
	struct ext_Rx *rx=NULL;
	struct sk_buff *result_skb=NULL;	
	int intf=portidx-SAR_INTFS-MAC_TX_INTFS;
	MT_RINGCTRL("call");
	/*
	if(first_time==TRUE)
	{
		init_ring();
	}
	*/

	/*xl_yue:20100204 start: add for bug that eth2wifi blocking*/
	if(eth2WifiBlockingFlag && isEthernetDomainMac(((struct sk_buff *)id)->data)){
		//printk("block wifi->lan\n");
		return((struct sk_buff *)id);
	}
	/*xl_yue:20100204 end*/
	
	if(rtl8139rx_drop==1) return NULL;
	rx=current_ext_prx[intf];

	if (unlikely(len>READ_MEM32(BUFFERSIZE)))
	{
		MT_RINGCTRL_DEBUG("pktsize=[%d] over data length!\n",len);		
		return NULL;	
	}
	
	if(rx->own==1) /* find a free extension port desc */
	{
		rx->data_length=len;  //not include crc
		result_skb=(struct sk_buff *)id;		
		rx->rx_buffer_addr=(uint32)result_skb->head;
		rx->rx_shift=(uint32)result_skb->data-(uint32)result_skb->head;
		result_skb=(struct sk_buff *)(rx->skb_header_addr);
		rx->skb_header_addr=(uint32)id;
		rx->linkid=linkID;
		rx->reason=0; // not to CPU
		MT_RINGCTRL_DEBUG("EXT PRX desc=%x used=%d eor=%d datalen=%d rxbufaddr=%x rxshift=%d skb_header_addr=%x linkid=%d\n",(uint32)rx, ext_prx_ring_used[intf],rx->eor,rx->data_length,rx->rx_buffer_addr,rx->rx_shift,rx->skb_header_addr,rx->linkid);		
		rx->own=0;

		if(rx->eor==0)
		{
			current_ext_prx[intf]++;		
		}
		else
		{
			//current_ext_prx[intf]=first_ext_prx[intf];
			current_ext_prx[intf]=user_ext_prx_idx(intf,0);
		}

		ext_prx_ring_used[intf]++;

		WRITE_MEM32(PENDINGRING,(1<<(16+intf)));  /* write one to trigger */
		
		MT_RINGCTRL_DEBUG("InSkb=%x ReturnSkb=%x\n",(uint32)id,(uint32)result_skb);
//		model_pp();
		MT_RINGCTRL_DEBUG("%s return %x\n",__FUNCTION__,(uint32)result_skb);		
	
		return result_skb;

		
	}
	else // ring full
	{
		MT_RINGCTRL_DEBUG("EXT RX RING FULL\n");
		// Drop or send by CPU
	}


	MT_RINGCTRL_DEBUG("%s return NULL\n",__FUNCTION__);
	return NULL;
}

//int rtl8672_vtx_start_xmit (struct sk_buff *skb, struct net_device *dev)
int rtl8672_vtx_start_xmit (struct sk_buff *skb, struct net_device *dev, int tx_port_idx)
{
	struct ext_Tx *etx;
	int intf=tx_port_idx-13;
	
	/*
	if(first_time==TRUE)
	{
		init_ring();
	}
	*/
	//etx=user_ext_vtx_idx(2,cpu_ext_vtx_idx[2]);
	etx=user_ext_vtx_idx(intf,cpu_ext_vtx_idx[intf]);
	if (unlikely(etx->own))
		goto DROP;
	
	// must leave at least one upper owned desc, so PP won't wrap.
	do {
		struct ext_Tx *tmpVtx;
		tmpVtx = user_ext_vtx_idx(intf,(cpu_ext_vtx_idx[intf]+1) % EXT_VTX_RING_SIZE);
		if (tmpVtx->own) {                  
			goto DROP;               
		}
	} while (0);
	
		
		etx->tx_buffer_addr=(uint32)skb->data;
		etx->data_length=skb->len;
		etx->orgAddr=(uint32)etx>>2;
		etx->fcpu=1;
		etx->skb_header_addr=(uint32)skb;
		etx->own=1;
		WRITE_MEM32(PENDINGRING,1<<tx_port_idx);
				
		cpu_ext_vtx_idx[intf]=(++cpu_ext_vtx_idx[intf])%EXT_VTX_RING_SIZE;
				
		MT_RINGCTRL_DEBUG("fill a EXT2 VTX desc!\n");		
			
	return SUCCESS;

DROP:
		MT_RINGCTRL_DEBUG("EXT VTX Ring is FULL!\n");
		return FAILED;
	}
void dCacheFlush(void){
__asm__ volatile(
		"mtc0 $0,$20\n\t"
		"nop\n\t"
		"li $8,512\n\t"
		"mtc0 $8,$20\n\t"
		"nop\n\t"
		"nop\n\t"
		"mtc0 $0,$20\n\t"
		"nop"
		: /* no output */
		: /* no input */
			);
}

int rtl8672_mac_vtx_start_xmit (struct sk_buff *skb, struct net_device *dev)
{
	struct mac_vTx *vtx,*next_vtx;
	int intf;
	unsigned long flags;
	struct mac_private *tp;
	tp=(struct mac_private *)dev->priv;

	spin_lock_irqsave(&tp->lock, flags);

	if(pp_nic_enable==0){
		spin_unlock_irqrestore(&tp->lock, flags);
		return 0;
	}

	intf=tp->tx_intf;		
	vtx=user_mac_vtx_idx(intf,cpu_mac_vtx_idx[intf]);
	if(vtx->own==0) /* not full */
	{
		vtx->tagc=0;
		next_vtx=user_mac_vtx_idx(intf,(cpu_mac_vtx_idx[intf]+1)%MAC_VTX_RING_SIZE);

		if(next_vtx->own==1)
		{
			MT_RINGCTRL_DEBUG("MAC %d VTX Ring is FULL,skb=%x!\n",intf,(uint32)skb);
			tp->stats.tx_dropped++;			
			return FAILED;
			//return 0;
		}
		// recycle the old skb
		if(cpu_mac_vtx_skb[intf][cpu_mac_vtx_idx[intf]]!=0)
		{
			dev_kfree_skb((struct sk_buff *)cpu_mac_vtx_skb[intf][cpu_mac_vtx_idx[intf]]);
			cpu_mac_vtx_skb[intf][cpu_mac_vtx_idx[intf]] = (uint32)NULL;
		}

		// Kaohj --- external switch support
		if (pp_mac_vtx_sw(skb, vtx)==0)
			return 0;

		tp->stats.tx_packets++;
		tp->stats.tx_bytes += skb->len;
		cpu_mac_vtx_skb[intf][cpu_mac_vtx_idx[intf]]	=(uint32)skb;
#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
		vtx->tx_buffer_addr=((uint32)skb->data);
#else
		vtx->tx_buffer_addr=PHYSICAL_ADDRESS(skb->data);
#endif

		if(Dump_ppTrapCPUpkt_debug){
			pp_memDump((void *)(skb->data),skb->len, "PP_MAC_VTX_SKB", 0);
		}

		/* tony: flush the cache for DMA data (make sure the data is uncached.) */
		dma_cache_wback_inv((unsigned long)skb->data, skb->len);

#if defined(CONFIG_RLE0437) ||defined(CONFIG_RTL8681)
		vtx->lgsen=0;   //czyao for 0437
#endif
		vtx->data_length=skb->len;
#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
		vtx->orgAddr=(uint32)vtx>>2;
#else
		vtx->orgAddr=(PHYSICAL_ADDRESS(vtx))>>2;
#endif
		vtx->fcpu=1;
		vtx->fs=1;
		vtx->ls=1;
		vtx->crc=1;
		vtx->own=1;

		if(Dump_ppTrapCPUpkt_debug)
			pp_memDump((void *)(vtx),20, "PP_MAC_VTX_DESC", 0);

		spin_unlock_irqrestore(&tp->lock, flags);
		
		WRITE_MEM32(PENDINGRING,PENDINGRING_MAC_VTX0<<intf);
		
	
		cpu_mac_vtx_idx[intf]=(++cpu_mac_vtx_idx[intf])%MAC_VTX_RING_SIZE;
		MT_RINGCTRL_DEBUG("fill a MAC %d VTX desc!\n",intf);		
	}
	else
	{
		spin_unlock_irqrestore(&tp->lock, flags);
		/* assert: never go here!! */
		rtlglue_printf("Exception: MAC %d VTX Ring is FULL!\n",intf);
		dev_kfree_skb(skb);
		tp->stats.tx_dropped++;
		return 0;
	}
	return SUCCESS;
}

#ifdef CONFIG_RTL8681_PTM
int __rtl8672_ptm_vtx_start_xmit (struct sk_buff *skb, struct net_device *dev)
	{
		struct mac_vTx *vtx,*next_vtx;
		int intf;
	unsigned long flags;
	struct mac_private *tp;
	tp=(struct mac_private *)dev->priv;

	spin_lock_irqsave(&tp->lock, flags);	

	if(pp_ptm_enable==0){
		spin_unlock_irqrestore(&tp->lock, flags);
		return 0;
	}

	intf=tp->tx_intf;		
	
	vtx=user_ptm_vtx_idx(intf,cpu_ptm_vtx_idx[intf]);
	if(vtx->own==0) /* not full */
	{
		vtx->tagc=0;
		next_vtx=user_ptm_vtx_idx(intf,(cpu_ptm_vtx_idx[intf]+1)%PTM_VTX_RING_SIZE);
			
		if(next_vtx->own==1)
		{
			spin_unlock_irqrestore(&tp->lock, flags);			
			//MT_RINGCTRL_DEBUG("PTM %d VTX Ring is FULL,skb=%x!\n",intf,(uint32)skb);
			tp->stats.tx_dropped++; 		
			return FAILED;
		}

		// recycle the old skb
		if(cpu_ptm_vtx_skb[intf][cpu_ptm_vtx_idx[intf]]!=0)
		{
			dev_kfree_skb((struct sk_buff *)cpu_ptm_vtx_skb[intf][cpu_ptm_vtx_idx[intf]]);
			cpu_ptm_vtx_skb[intf][cpu_ptm_vtx_idx[intf]] = (uint32)NULL;
		}


		if (pp_mac_vtx_sw(skb, vtx)==0)
			return 0;

		tp->stats.tx_packets++;
		tp->stats.tx_bytes += skb->len;
		cpu_ptm_vtx_skb[intf][cpu_ptm_vtx_idx[intf]]	=(uint32)skb;
		vtx->tx_buffer_addr=PHYSICAL_ADDRESS((uint32)skb->data);
		//printk("[%s, line %d] vtx->tx_buffer_addr = 0x%08X, skb->data = 0x%08X\n"
		//	,__func__,__LINE__,vtx->tx_buffer_addr, (unsigned int)(skb->data));
		IN_MT_RINGCTRL_DEBUG(
			memDump(skb->data,skb->len,"mac tx data");
			);

		if(Dump_ppTrapCPUpkt_debug)
			pp_memDump((void *)(skb->data),(u32)(skb->len),"PP_PTM_TX_DATA", 1);		

		/* tony: flush the cache for DMA data (make sure the data is uncached.) */
		dma_cache_wback_inv((unsigned long)skb->data, skb->len);

#if defined(CONFIG_RLE0437) || defined(CONFIG_RTL8681)
		vtx->lgsen=0;	//czyao for 0437
#endif
		vtx->data_length=skb->len;
		vtx->orgAddr=((PHYSICAL_ADDRESS((uint32)vtx))>>2);
		vtx->fcpu=1;
		vtx->fs=1;
		vtx->ls=1;
		vtx->crc=1;
		vtx->own=1;

		spin_unlock_irqrestore(&tp->lock, flags);

		if(Dump_ppTrapCPUpkt_debug)
			pp_memDump((void *)(vtx),20,"PP_PTM_VTX_DESC", 1);
		
		WRITE_MEM32(PENDINGRING,PENDINGRING_SAR_VTX0<<intf);

		cpu_ptm_vtx_idx[intf]=(++cpu_ptm_vtx_idx[intf])%PTM_VTX_RING_SIZE;
		MT_RINGCTRL_DEBUG("fill a PTM %d VTX desc!\n\n",intf);		
	}
	else
	{
		spin_unlock_irqrestore(&tp->lock, flags);
		/* assert: never go here!! */
		rtlglue_printf("Exception: PTM %d VTX Ring is FULL!\n",intf);
		dev_kfree_skb(skb);
		tp->stats.tx_dropped++;
		return 0;
	}
	return SUCCESS;
}

extern unsigned char adslup;
int rtl8672_ptm_vtx_start_xmit (struct sk_buff *skb, struct net_device *dev)
{
	if(adslup==1)
		__rtl8672_ptm_vtx_start_xmit (skb, dev);
	else
		dev_kfree_skb(skb);

	return 0;
}

#endif  //end RTL8672_PTM

#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
extern void ppsar_rx(struct sk_buff* skb, int port);
extern unsigned char PVC_mac_offset(int ch_no);
extern struct net_device *get_sar_netdev(struct atm_vcc *vcc, int port);
//for ipqos
#define IPQOS_LOW 2
#define IPQOS_HIGH 4
unsigned char get_SARvtx_available_desc_num(int port){
	if(cpu_sar_vtx_idx[port] >= cpu_sar_vtx_finish_idx[port])
		return (SAR_VTX_RING_SIZE - (cpu_sar_vtx_idx[port] - cpu_sar_vtx_finish_idx[port]));
	else
		return (cpu_sar_vtx_finish_idx[port] - cpu_sar_vtx_idx[port]);
}

unsigned int SAR_TDFI_get_and_clear(void){
	unsigned int sar_tdfi;
	sar_tdfi = READ_MEM32(SARVTDFI);
	WRITE_MEM32(SARIMR,READ_MEM32(SARIMR)|SAR_VTDF);
	WRITE_MEM32(SARVTDFI, sar_tdfi);
	WRITE_MEM32(SARIMR,READ_MEM32(SARIMR)&(~SAR_VTDF));
	return sar_tdfi;
}
atomic_t sar_vtx_lock_txbuff = ATOMIC_INIT(0);
void clearSARvtxBuffer(int port, struct net_device *dev){
	struct sar_vTx *sarTx;
	if( atomic_read(&sar_vtx_lock_txbuff) )
		return;
	atomic_set(&sar_vtx_lock_txbuff, 1);
	sarTx = user_sar_vtx_idx(port,cpu_sar_vtx_finish_idx[port]);
	while((sarTx->own == 0) && cpu_sar_vtx_skb[port][cpu_sar_vtx_finish_idx[port]]){
		dev_kfree_skb((struct sk_buff *)cpu_sar_vtx_skb[port][cpu_sar_vtx_finish_idx[port]]);
		cpu_sar_vtx_skb[port][cpu_sar_vtx_finish_idx[port]]=0;
		if(cpu_sar_vtx_finish_idx[port] == SAR_VTX_RING_SIZE-1)
			cpu_sar_vtx_finish_idx[port] = 0;
		else
			cpu_sar_vtx_finish_idx[port]++;
	
		sarTx = user_sar_vtx_idx(port,cpu_sar_vtx_finish_idx[port]);
	}
	if(enable_ipqos){
		if(get_SARvtx_available_desc_num(port) >= IPQOS_HIGH){
			if(netif_queue_stopped(dev)){
				netif_wake_queue(dev);
				//printk("wake queue %s\n",dev->name);
			}
		}
	}
	atomic_set(&sar_vtx_lock_txbuff, 0);
}
int SARvtx(struct atm_vcc *vcc, struct sk_buff* skb, unsigned int len, int port){
	struct sar_vTx *sarTx;
	struct sk_buff *skb_to_send;
	skb_to_send = skb;

	clearSARvtxBuffer(port, get_sar_netdev(vcc,port));
	//check for ipqos
	if(enable_ipqos){
		if(get_SARvtx_available_desc_num(port)<=IPQOS_LOW){
			netif_stop_queue(get_sar_netdev(vcc, port));
			//printk("stop queue %s\n",(get_sar_netdev(vcc,port))->name);
		}
	}

	// Kaohj --- external switch support
	if (!sar_send_sw(vcc, &skb_to_send))
		goto DropPacket;
	sarTx=user_sar_vtx_idx(port,cpu_sar_vtx_idx[port]);//&user_sar_vtx[SAR_VTX_RING_SIZE*port+current_desc[port]];
	//check if vtx is full
	if(unlikely(sarTx->own == 1)){
		goto DropPacket;
	}

	// must leave at least one upper owned desc, so PP won't wrap.
	do {
		struct sar_vTx *tmpVtx;
		tmpVtx = user_sar_vtx_idx(port,(cpu_sar_vtx_idx[port]+1) % SAR_VTX_RING_SIZE);
		if (tmpVtx->own) {                  
		goto DropPacket;
	}
	} while (0);
	
	//send a packet to SAR now
	sarTx->rsv=0;
	sarTx->rsv2=0;
	sarTx->rsv3=0;
	sarTx->rsv4=0;
	//sarTx->rsv5=0;
	sarTx->fs=1;
	sarTx->ls=1;
	if(sendToPort0)
		sarTx->atmport=1;
	else
		sarTx->atmport=0;
	sarTx->data_length=len;
	cpu_sar_vtx_skb[port][cpu_sar_vtx_idx[port]]=(uint32)skb_to_send;
	sarTx->tx_buffer_addr=(uint32)(skb_to_send->data);
	sarTx->trlren=1;
	sarTx->ethnt_offset=0x3f;
	sarTx->orgAddr=((uint32)sarTx>>2);
	sarTx->fcpu=1;
	sarTx->trlr=0;
	//debug sar rx
	sarTx->clp=0;
	
	//sarTx->sram_map_id=0;
	//sarTx->sram_size=0;

	//printk("set:%x\n",*(unsigned int*)sarTx);
	if(cpu_sar_vtx_idx[port]==SAR_VTX_RING_SIZE-1){
		sarTx->eor=1;
		//sarTx->oeor=1;
		}
	else{
		sarTx->eor=0;
		//sarTx->oeor=0;
		}
	//DCACHE flush
	sarTx->pti=0;

	dma_cache_wback_inv((unsigned long)skb_to_send->data, len);
#ifdef CONFIG_RTK_VOIP
       set_write_allocate();
#endif
	sarTx->own=1;
	//clear TBE to trigger SAR HW to send this packet
	//Clear_TBE(port);
	WRITE_MEM32(PENDINGRING,1<<port);
	if(cpu_sar_vtx_idx[port]==SAR_VTX_RING_SIZE-1)
		cpu_sar_vtx_idx[port]=0;
	else
		cpu_sar_vtx_idx[port]++;
	return 0;

DropPacket:
	dev_kfree_skb(skb_to_send);
	return 0;
}

#ifdef SAR_FIRST
static int kill_non_sar(void *ptr, void *user_data) {
	rtl8672_tblAsicDrv_l2Param_t *l2pt = ptr;
	/*
	printk("found: %d %02x%02x:%02x%02x:%02x%02x port%d vl%d ",
			l2pt->Valid,
			l2pt->MAC[0],l2pt->MAC[1],l2pt->MAC[2],l2pt->MAC[3],
			l2pt->MAC[4],l2pt->MAC[5],l2pt->Port, l2pt->VlanID);
	*/
	if ((l2pt->Port >= 8)&& (l2pt->Port != 9)) {
		rtl8672_delAsicL2Entry(l2pt, OPT_AUTO);
		//printk("del[%d]\n",	rtl8672_delAsicL2Entry(l2pt, OPT_AUTO));
	} else {
		//printk("\n");
	}

	return 0;
}
#endif


static inline void sar_l2learning(struct sk_buff *skb,int intfidx,int vlanid, int reason) {
	
#ifdef SAR_FIRST
	/* Two cases to consider here: 1. DMAC Port is VC, 2. SMAC is VC
	For case 1: Add SMAC to high queue.
	For case 2: Depend on the reason (a. SMAC unknown, or DMAC unknown)
		case a: 
		case b: 
	*/
	rtl8672_tblAsicDrv_l2Param_t l2;
	unsigned char mac_offset=PVC_mac_offset(intfidx);

	//for current IC, we can not handle ipqos cases.
	if(enable_ipqos || enable_vlan_grouping)
		return;

	//for pppoa and routed pvc, skb does not contain MAC addr
	if(mac_offset==0)
		return;

	//printk("sar_l2learning reason %d\n", reason);
	if (SUCCESS != rtl8672_L2EntryLookup(&skb->data[mac_offset+6],vlanid,&l2)) {
		printk("flush none vc entries\n");
		rtl8672_L2EntrySearch(kill_non_sar, 0);	
	}
	
#endif	
	rtl8672_l2learning(&skb->data[mac_offset+6],intfidx,vlanid);
}


unsigned int tmpisr=0;
extern int rx_small_pkt;
void ppsar_rx_bh(unsigned long cp){
		uint32 isr2,i,j,startidx,endidx;
		unsigned long flags;
//		WRITE_MEM32(SARIMR,READ_MEM32(SARIMR)&(~SAR_RDA));
		local_irq_save(flags); //local_irq_disable();
		isr2=tmpisr;
		tmpisr=0;
		local_irq_restore(flags);//local_irq_enable();		
		//isr2=READ_MEM32(SARRDAI);
		//WRITE_MEM32(SARRDAI,isr2);
		for(i=0;i<SAR_INTFS;i++)			
		{

			if(isr2&(1<<i)) // check any packet in this ring
			{
				int currentidx=READ_MEM32(SVRXDESC0+(i<<2));
				struct sar_vRx *sarRx;			

				startidx=cpu_sar_vrx_idx[i];
				endidx=currentidx;
				sarRx=user_sar_vrx_idx(i,startidx);

				if(startidx>endidx) endidx+=SAR_VRX_RING_SIZE;
				if((startidx==endidx)&&(sarRx->own==0)) endidx+=SAR_VRX_RING_SIZE;
				
//rtlglue_printf("intf=%d start=%d end=%d currentidx=%d\n",i,startidx%SAR_VRX_RING_SIZE,endidx%SAR_VRX_RING_SIZE,currentidx);

				for(j=startidx;j<endidx;j++)
				{
					struct sar_vRx *sarRx;
					sarRx=user_sar_vrx_idx(i,j%SAR_VRX_RING_SIZE);

					if(sarRx->own==0)
					{
						rtl8672_tblAsicDrv_intfParam_t rxintf;	

						// receive packet here ....

						uint8 *head,*data,*tail,*end;
						struct sk_buff *newskb,*skb;
						
						skb=(struct sk_buff *)sarRx->skb_header_addr;
						//printk("sar rx...crc:%d   icerr:%d  reason:%d\n",sarRx->crc32err,sarRx->ipcerr,sarRx->reason);
						skb->data=skb->head+sarRx->rx_shift;
						rtl8672_getAsicNetInterface(i, &rxintf);

						//tylo, handle crc
					#if 1
						if(sarRx->reason==2 || (sarRx->reason==1)){ //drop crc & (fs or ls !=0)
							sarRx->data_length=READ_MEM32(BUFFERSIZE);							
							sarRx->rx_shift=rxintf.RXshift;
							//skb_reserve(skb,rxintf.RXshift);
							//sarRx->rx_buffer_addr=(uint32)skb->head;
							init_skb(skb,BUFFER_SIZE);
							sarRx->own=1;
							cpu_sar_vrx_idx[i]=(cpu_sar_vrx_idx[i]+1)%SAR_VRX_RING_SIZE;
							//printk("crc!\n");
							continue;
						}
					#endif
						
						newskb=(struct sk_buff *)(((uint32)dev_alloc_skb(BUFFER_SIZE)));
						if(newskb==NULL)
						{
							if (printk_ratelimit())
								printk("allocate failed at: %s %d\n",__FUNCTION__,__LINE__);
							init_skb(skb,BUFFER_SIZE);
						} else {

							init_skb(newskb,BUFFER_SIZE);
							newskb->fcpu = 1;
							head=newskb->head;
							data=newskb->data;
							tail=newskb->tail;
							end=newskb->end;
							if(sarRx->lenerr) {
								newskb->len=sarRx->data_length;
							}
							else{		
								newskb->len=sarRx->bpc;
							}
							//newskb->len=sarRx->data_length;
							newskb->head=(uint8 *)(((uint32)skb->head)&(~0x20000000));
							newskb->data=(uint8 *)(((uint32)skb->data)&(~0x20000000));
							newskb->tail=(uint8 *)(((uint32)skb->tail)&(~0x20000000));
							newskb->end=(uint8 *)(((uint32)skb->end)&(~0x20000000));
							skb->head=head;
							skb->data=data;
							skb->tail=tail;
							skb->end=end;
							skb->len=0;
							//rtl8672_l2learning(&newskb->data[6], i+SAR_INTFS,mac_dev[i]->vlanid);
							//  andrew, using sar_l2learning. rtl8672_l2learning(&newskb->data[16], i,rxintf.PortVlanID); //tylo, temp. marked vlanid field
							sar_l2learning(newskb, i, rxintf.PortVlanID, sarRx->reason);
							// andrew, bug? rtl8672_l2learning(&newskb->data[16], i,8);
							//skb_debug(newskb,newskb->len);
							if(newskb->len < 256)
								rx_small_pkt++;
						
							newskb->tail = newskb->data + newskb->len;
							ppsar_rx(newskb,i);
							//MT_RINGCTRL_DEBUG("newskb->dataref=%d newskb->nr_frags=%d skb->users=%d\n",atomic_read(&skb_shinfo(newskb)->dataref),skb_shinfo(newskb)->nr_frags,atomic_read(&skb->users) );
						}
						//recycle the desc
						cpu_sar_vrx_idx[i]=(cpu_sar_vrx_idx[i]+1)%SAR_VRX_RING_SIZE;
						sarRx->data_length=READ_MEM32(BUFFERSIZE);
						sarRx->rx_shift=rxintf.RXshift;
						skb_reserve(skb,rxintf.RXshift);
						sarRx->rx_buffer_addr=(uint32)skb->head;
						sarRx->own=1;																		

					}
				}

				// clear the sarrdai				
				//tmpisr&=(~(1<<i));
			}
		}
		MT_RINGCTRL_DEBUG("SARISR - SARRDAI clear.\n")
		//WRITE_MEM32(SARIMR,READ_MEM32(SARIMR)|SAR_RDA);
		// clear sar_rda
}
#endif     //end RTL8672_SAR

#ifdef CONFIG_RTL8681_PTM
unsigned int ptm_tmpisr=0;
void ppptm_rx_bh(unsigned long cp)
{
		uint32 isr2,i,j,startidx,endidx;
		unsigned long flags;
//		WRITE_MEM32(SARIMR,READ_MEM32(SARIMR)&(~SAR_RDA));
		if(pp_ptm_enable==0)
			return;

		local_irq_save(flags); //local_irq_disable();
		isr2=ptm_tmpisr;
		ptm_tmpisr=0;
		local_irq_restore(flags);//local_irq_enable();		

		for(i=0;i<PTM_INTFS;i++)			
		{

			if(isr2&(1<<i)) // check any packet in this ring
			{
				int currentidx=READ_MEM32(SVRXDESC0+(i<<2));
				struct mac_vRx *ptmRx;			

				startidx=cpu_ptm_vrx_idx[i];
				endidx=currentidx;
				ptmRx=user_ptm_vrx_idx(i,startidx);

				if(startidx>endidx) endidx+=PTM_VRX_RING_SIZE;
				if((startidx==endidx)&&(ptmRx->own==0)) endidx+=PTM_VRX_RING_SIZE;
				
				for(j=startidx;j<endidx;j++)
				{
					ptmRx=user_ptm_vrx_idx(i,j%PTM_VRX_RING_SIZE);

					if(ptmRx->own==0)
					{
						rtl8672_tblAsicDrv_intfParam_t rxintf;	

						// receive packet here ....
						uint8 *head,*data,*tail,*end;
						struct sk_buff *newskb,*skb;
				
						skb=(struct sk_buff *)(KSEG1_ADDRESS(ptmRx->skb_header_addr));
						skb->data=skb->head+ptmRx->rx_shift;

						if(Dump_ppTrapCPUpkt_debug){
							pp_memDump((void *)ptmRx, 20, "PP_PTM_VRX_DESC", 1);
							pp_memDump((void *)(skb->data), (u32)(ptmRx->data_length), "PP_PTM_VRX_SKB", 1);
						}

						if((ptmRx->ls!=1) ||(ptmRx->fs!=1)){
							printk("[%s] Fragment packet\n",__func__);
							goto NEXT_DESC;
						}

						if(ptmRx->crc==1){
							printk("[%s] CRC error packet\n",__func__);
							goto NEXT_DESC;

						}

						if((ptmRx->data_length >1518) && (ptmRx->data_length < 64)){
							printk("[%s]Illegal data length\n",__func__);
							goto NEXT_DESC;
						}

						if(Dump_ppTrapCPU_hsab_debug){
							pp_memDump((void *)0xb8625140, 64, "PP_PTM_HSA", 1);
							pp_memDump((void *)0xb8625100, 64, "PP_PTM_HSB", 1);
						}

						rtl8672_getAsicNetInterface(i, &rxintf);
					
						newskb=(struct sk_buff *)(((uint32)dev_alloc_skb(BUFFER_SIZE)));
						if(newskb==NULL)
						{
							if (printk_ratelimit()){
								printk("allocate failed at: %s %d\n",__FUNCTION__,__LINE__);
								goto NEXT_DESC;
							}
							//init_skb(skb,BUFFER_SIZE);

						} else {

							init_skb(newskb,BUFFER_SIZE);
							newskb->fcpu = 1;
							head=newskb->head;
							data=newskb->data;
							tail=newskb->tail;
							end=newskb->end;
							newskb->dev=ptmPP_dev;
							newskb->len=ptmRx->data_length;
							newskb->head=(uint8 *)(((uint32)skb->head)&(~0x20000000));
							newskb->data=(uint8 *)(((uint32)skb->data)&(~0x20000000));
							newskb->tail=(uint8 *)(((uint32)skb->tail)&(~0x20000000));
							newskb->end=(uint8 *)(((uint32)skb->end)&(~0x20000000));
							skb->head=head;
							skb->data=data;
							skb->tail=tail;
							skb->end=end;
							skb->len=0;
							//rtl8672_l2learning(&newskb->data[6], i+SAR_INTFS,mac_dev[i]->vlanid);
							//  andrew, using sar_l2learning. rtl8672_l2learning(&newskb->data[16], i,rxintf.PortVlanID); //tylo, temp. marked vlanid field
#if 0
							mac_l2learning(newskb, 0, rxintf.PortVlanID);
							mac_l2learning(newskb, i, rxintf.PortVlanID);
#else
							mac_l2learning(newskb, i,rxintf.PortVlanID);
#endif
							newskb->protocol = eth_type_trans (newskb, newskb->dev);
							newskb->len = newskb->len -4;
							newskb->tail = newskb->data + newskb->len;
							netif_rx(newskb);
							//MT_RINGCTRL_DEBUG("newskb->dataref=%d newskb->nr_frags=%d skb->users=%d\n",atomic_read(&skb_shinfo(newskb)->dataref),skb_shinfo(newskb)->nr_frags,atomic_read(&skb->users) );
						}
NEXT_DESC:
						//recycle the desc
						cpu_ptm_vrx_idx[i]=(cpu_ptm_vrx_idx[i]+1)%PTM_VRX_RING_SIZE;
						ptmRx->data_length=READ_MEM32(BUFFERSIZE);
						ptmRx->rx_shift=rxintf.RXshift;
						skb_reserve(skb,rxintf.RXshift);
						ptmRx->rx_buffer_addr=PHYSICAL_ADDRESS((uint32)skb->head);
						ptmRx->own=1;																		

					}
				}

			}
		}
		MT_RINGCTRL_DEBUG("PTMISR - PTMRDAI clear.\n")

}
#endif

static inline void mac_l2learning(struct sk_buff *skb,int intfidx,int vlanid) {
	int offset = 0;
#ifdef SAR_FIRST
	/* Refer to sar_l2learning
	*/
	rtl8672_tblAsicDrv_l2Param_t l2;
	offset = 1;

	if (SUCCESS != rtl8672_L2EntryLookup(&skb->data[0],vlanid,&l2)) 
		goto OUT;

	if (l2.Port > 7)
		goto OUT;

	//printk("mac: put %02x%02x:%02x%02x:%02x%02x to hi queue %d\n", 
	//	skb->data[6],skb->data[7],skb->data[8],skb->data[9],skb->data[10],skb->data[11],intfidx);
	rtl8672_l2learning(&skb->data[6],intfidx,vlanid);

	return;	
		
OUT:
#endif	
	rtl8672_l2learning(&skb->data[6],offset+intfidx,vlanid);
}


extern struct sk_buff *re8670_getAlloc(unsigned int size);

#ifdef MAC_USE_TASKLET
#ifndef CONFIG_USB_RTL8192SU_SOFTAP
__IRAM 
#endif
#else
static inline 
#endif
void ppmac_rx_bh(unsigned long cp)
{
			
	uint32 isr,j,startidx,endidx,i;
	int tag=0;

	extern int eth_poll;

	
	#ifdef MAC_USE_TASKLET		
	unsigned long flags;	

	if(pp_nic_enable==0)
		return;

	local_irq_save(flags);//local_irq_disable();
	#endif
	isr=tmpmacisr;
	tmpmacisr=0;
	#ifdef MAC_USE_TASKLET
	local_irq_restore(flags);//local_irq_enable();	
	#endif
	
	#ifdef NEW_ARCH
#ifdef EXT_PHY
	i=0;		
#else
	i=1;
#endif //EXT_PHY

	if (unlikely(0!=eth_poll)) {		
		isr = READ_MEM32(MAC1ISR);
		WRITE_MEM32(MAC1ISR,isr);
		isr = isr << i;
	}
	
	#else
	for(i=0;i<2;i++)
	#endif	
	{
		if(isr & (1<<i)){
			int currentidx=READ_MEM32(MVRXDESC0+(i<<2));
			struct mac_vRx *macRx;
			
			startidx=cpu_mac_vrx_idx[i];
			endidx=currentidx;
			macRx=user_mac_vrx_idx(i,startidx);			

			if(startidx>endidx) endidx+=MAC_VRX_RING_SIZE;
			if((startidx==endidx)&&(macRx->own==0)) endidx+= MAC_VRX_RING_SIZE;
			
	//rtlglue_printf("intf=%d start=%d end=%d currentidx=%d\n",i,startidx%MAC_VRX_RING_SIZE,endidx%MAC_VRX_RING_SIZE,currentidx);

			for(j=startidx;j<endidx;j++)
			{
				//struct mac_vRx *macRx;
				macRx=user_mac_vrx_idx(i,j%MAC_VRX_RING_SIZE);

				if(macRx->own==0)
				{
					rtl8672_tblAsicDrv_intfParam_t rxintf;	

	//rtlglue_printf("------------- %s %d: macRx addrs = 0x%08X\n",__FILE__,__LINE__,(unsigned int)macRx);	

					// receive packet to protocol stack!

					{
						struct mac_private *tp;
						uint8 *head,*data,*tail,*end;
						struct sk_buff *newskb,*skb;
						int pkt_vid=8;

						if(Dump_ppTrapCPU_hsab_debug){
							pp_memDump((void *)0xb8625140, 64, "PP_MAC_HSA", 0);
							pp_memDump((void *)0xb8625100, 64, "PP_MAC_HSB", 0);						
						}
#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
						skb=(struct sk_buff *)(macRx->skb_header_addr);
#else
						skb=(struct sk_buff *)KSEG1_ADDRESS(macRx->skb_header_addr);						
#endif					
						skb->data=skb->head+macRx->rx_shift;

						if(Dump_ppTrapCPUpkt_debug){							
							pp_memDump((void *)macRx, 20, "PP_MAC_VRX_DESC", 0);
							pp_memDump((void *)(skb->data), (u32)(macRx->data_length), "PP_MAC_VRX_SKB", 0);
						}
						
						if((macRx->ls!=1) && (macRx->fs!=1)){
							printk("[%s, line %d] frag packet, drop it\n",__func__,__LINE__);
							pp_memDump((void *)macRx, 20, "========== MAC VRx desc ==========",0);
							pp_memDump((void *)(skb->data), 64, "========== PP_MAC_SKB ==========",0);
							goto NEXT_DESC;
						}

						if(macRx->crc==1 ){
							printk("[%s, line %d] crc error packet, drop it\n",__func__,__LINE__);
							pp_memDump((void *)macRx, 20, "========== MAC VRx desc ==========",0);
							pp_memDump((void *)(skb->data), 64, "========== PP_MAC_SKB ==========",0);
							goto NEXT_DESC;
						}
						
						if((macRx->data_length<64) ||(macRx->data_length>1518)  ){
							printk("[%s, line %d] illegal  packet length = %d, drop it\n",__func__,__LINE__,macRx->data_length);
							pp_memDump((void *)macRx, 20, "========== MAC VRx desc ==========",0);
							pp_memDump((void *)(skb->data), 64, "========== PP_MAC_SKB ==========",0);
							goto NEXT_DESC;
						}				
					
						newskb=(struct sk_buff *)(((uint32)re8670_getAlloc(BUFFER_SIZE)));
 						rtl8672_getAsicNetInterface(i+SAR_INTFS, &rxintf);
						if (unlikely(newskb==NULL))
						{
							if (printk_ratelimit())
								printk("allocate failed at: %s %d\n",__FUNCTION__,__LINE__);
							init_skb(skb,BUFFER_SIZE);
						} else {
							init_skb(newskb,BUFFER_SIZE);
							newskb->fcpu = 1;

							head=newskb->head;
							data=newskb->data;
							tail=newskb->tail;
							end=newskb->end;
							newskb->len=macRx->data_length;
							newskb->dev=mac_dev[i];
							tp=(struct mac_private *)mac_dev[i]->priv;
							tp->stats.rx_packets++;
							tp->stats.rx_bytes += skb->len;						
							tp->tx_intf=i;
							newskb->head=(uint8 *)(((uint32)skb->head)&(~0x20000000));
							newskb->data=(uint8 *)(((uint32)skb->data)&(~0x20000000));
							newskb->tail=(uint8 *)(((uint32)skb->tail)&(~0x20000000));
							newskb->end=(uint8 *)(((uint32)skb->end)&(~0x20000000));
							skb->head=head;
							skb->data=data;
							skb->tail=tail;
							skb->end=end;
							skb->len=0;
							MT_RINGCTRL_DEBUG("newskb->dataref=%d newskb->nr_frags=%d skb->users=%d\n",atomic_read(&skb_shinfo(newskb)->dataref),skb_shinfo(newskb)->nr_frags,atomic_read(&skb->users) );
						
							
							//rtl8672_l2learning(&newskb->data[6], i+SAR_INTFS,mac_dev[i]->vlanid);
							
							pkt_vid=rxintf.PortVlanID;
#if 0 // do this after l2learning or packet processor wont work due to skb->data will be changed in eth_type_trans
							newskb->protocol = eth_type_trans (newskb, newskb->dev); 
#endif
							tag = pp_mac_rx_sw(i, macRx, newskb);
							//rtl8672_l2learning(&newskb->data[6], i+SAR_INTFS,rxintf.PortVlanID); 
							mac_l2learning(newskb, i+SAR_INTFS,rxintf.PortVlanID); 
							newskb->protocol = eth_type_trans (newskb, newskb->dev); //shlee
							pp_check_igmp_snooping_rx(newskb,tag);
							//printk("nic tag:%d\n",tag);
							
							//skb_debug(newskb,newskb->len);
	 						newskb->len = newskb->len -4;
							newskb->tail = newskb->data + newskb->len;
							netif_rx(newskb);
						}
NEXT_DESC:
						//recycle the desc
						cpu_mac_vrx_idx[i]=(cpu_mac_vrx_idx[i]+1)%MAC_VRX_RING_SIZE;
						macRx->data_length=READ_MEM32(BUFFERSIZE);
						//rtl8672_getAsicNetInterface(SAR_INTFS+i, &rxintf);
						macRx->rx_shift=rxintf.RXshift;
						skb_reserve(skb,rxintf.RXshift);
#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
						macRx->rx_buffer_addr=(uint32)skb->head;
#else
						macRx->rx_buffer_addr=PHYSICAL_ADDRESS(skb->head);
#endif
						macRx->own=1;
					}
				}
			}
			
		}
	}
}


#if defined(CONFIG_RTL8672_SAR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
#ifndef CONFIG_USB_RTL8192SU_SOFTAP
__IRAM
#endif
irqreturn_t sar_intHandler(int irq, void *dev_instance)
{
	uint32 isr,isr2,imr;
	//uint32 i,j,startidx,endidx;
	MT_RINGCTRL("call");

	isr=READ_MEM32(SARISR);
	imr=READ_MEM32(SARIMR);
	MT_RINGCTRL_DEBUG("SARISR=%x SARIMR=%x\n",isr,imr);	
	WRITE_MEM32(SARIMR,0);

	if(isr&SAR_RDA)
	{
		isr2=READ_MEM32(SARRDAI);
		WRITE_MEM32(SARRDAI,isr2);
		tmpisr |= isr2;
		tasklet_hi_schedule(&ppsar_rx_tasklets);

	}

	if(isr&SAR_RBF)
	{
		isr2=READ_MEM32(SARRBFI);
		MT_RINGCTRL_DEBUG("SARISR - SARRBFI clear.\n");
		WRITE_MEM32(SARRBFI,isr2);
		
	}

	if(isr&SAR_VTDF)
	{
		//isr2=READ_MEM32(SARVTDFI);
		//MT_RINGCTRL_DEBUG("SARISR - SARVTDFI clear.\n");
		//WRITE_MEM32(SARVTDFI,isr2);
	}

#ifdef SAR_LLIP_SIM
	if(isr&SAR_PTDF)
	{
		isr2=READ_MEM32(SARPTDFI);

		if(isr2)
		{
			model_sar_tx();
			MT_RINGCTRL_DEBUG("SARISR - SARPTDFI clear.\n");			
			WRITE_MEM32(SARPTDFI,isr2);		
		}
	}
#endif	
	WRITE_MEM32(SARIMR,imr);
		
	return IRQ_RETVAL(1);
}
#endif

#ifdef CONFIG_RTL8681_PTM
irqreturn_t ptm_intHandler(int irq, void *dev_instance)
{
	uint32 isr,isr2,imr;
	//uint32 i,j,startidx,endidx;
	MT_RINGCTRL("call");

	isr=READ_MEM32(SARISR);
	imr=READ_MEM32(SARIMR);
	MT_RINGCTRL_DEBUG("PTMISR=%x PTMIMR=%x\n",isr,imr);	
	WRITE_MEM32(SARIMR,0);

	if(isr&SAR_RDA)
	{
		isr2=READ_MEM32(SARRDAI);
		WRITE_MEM32(SARRDAI,isr2);
		ptm_tmpisr |= isr2;
		tasklet_hi_schedule(&ppptm_rx_tasklets);
	}

	if(isr&SAR_RBF)
	{
		isr2=READ_MEM32(SARRBFI);
		MT_RINGCTRL_DEBUG("PTMISR - PTMRBFI clear.\n");
		WRITE_MEM32(SARRBFI,isr2);
		
	}

	if(isr&SAR_VTDF)
	{
		//isr2=READ_MEM32(SARVTDFI);
		//MT_RINGCTRL_DEBUG("SARISR - SARVTDFI clear.\n");
		//WRITE_MEM32(SARVTDFI,isr2);
	}

#ifdef SAR_LLIP_SIM
	if(isr&SAR_PTDF)
	{
		isr2=READ_MEM32(SARPTDFI);

		if(isr2)
		{
			model_sar_tx();
			MT_RINGCTRL_DEBUG("PTMISR - PTMPTDFI clear.\n");			
			WRITE_MEM32(SARPTDFI,isr2);		
		}
	}
#endif	
	WRITE_MEM32(SARIMR,imr);
		
	return IRQ_RETVAL(1);
}
#endif

irqreturn_t mac_intHandler(int irq, void *dev_instance)
{
	uint32 isr,i,imr1,imr2;
	#ifndef MAC_USE_TASKLET
	//uint32 j,startidx,endidx;	
	//int tag;
	#endif

	MT_RINGCTRL("call");

	imr1=READ_MEM32(MAC0IMR);
	WRITE_MEM32(MAC0IMR,0);	
	imr2=READ_MEM32(MAC1IMR);	
	WRITE_MEM32(MAC1IMR,0);
//rtlglue_printf("------------- %s %d\n",__FILE__,__LINE__);	


#ifdef MAC_LLIP_SIM
	for(i=0;i<MAC_RX_INTFS;i++)
	{
		if(i==0)
		{
			isr=READ_MEM32(MAC0ISR);			
		}
		else
		{
			isr=READ_MEM32(MAC1ISR);					
		}

		if(isr&(MAC_PTOK0|MAC_PTOK1|MAC_PTOK2|MAC_PTOK3))
		{
			model_mac_tx();

			// clear mac-rok
			if(i==0)
			{
				WRITE_MEM32(MAC0ISR,MAC_PTOK0);
			}
			else
			{
				WRITE_MEM32(MAC1ISR,MAC_PTOK0|MAC_PTOK1|MAC_PTOK2|MAC_PTOK3);
			}
		}	
	}
#endif	

#ifdef NEW_ARCH
#ifdef EXT_PHY
	i=0;	
#else
	i=1;
#endif //EXT_PHY
#else
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
#ifndef EXT_PHY
	for(i=1;i<MAC_RX_INTFS;i++) //skip mac intf 8
#else	
	for(i=0;i<MAC_RX_INTFS;i++)
#endif	
#else
	for(i=0;i<MAC_RX_INTFS;i++)
#endif
#endif /* NEW_ARCH */
	{
		if(i==0)
		{
			isr=READ_MEM32(MAC0ISR);
			MT_RINGCTRL_DEBUG("MAC0ISR=%x MAC0IMR=%x\n",isr,imr1);
			if(isr&MAC_RER_OVF)
			{
				MT_RINGCTRL_DEBUG("MAC0ISR - MAC_RER_OVF clear.\n");	
				WRITE_MEM32(MAC0ISR,MAC_RER_OVF);
			}

			if(isr&MAC_VTOK)
			{
				MT_RINGCTRL_DEBUG("MAC0ISR - MAC_VTOK clear.\n");	
				WRITE_MEM32(MAC0ISR,MAC_VTOK);
			}

			if(isr&MAC_TDU)
			{
				MT_RINGCTRL_DEBUG("MAC0ISR - MAC_TDU clear.\n");	
				WRITE_MEM32(MAC0ISR,MAC_TDU);
			}

			if(isr&MAC_SWINT)
			{
				MT_RINGCTRL_DEBUG("MAC0ISR - MAC_SWINT clear.\n");	
				WRITE_MEM32(MAC0ISR,MAC_SWINT);
			}			

#if 0
			if(isr&MAC_PTOK0)
			{
				MT_RINGCTRL_DEBUG("MAC0ISR - MAC_PTOK0 clear.\n");	
				WRITE_MEM32(MAC0ISR,MAC_PTOK0);
			}
#endif

		}
		else
		{
//rtlglue_printf("------------- %s %d\n",__FILE__,__LINE__);	
			isr=READ_MEM32(MAC1ISR);
			MT_RINGCTRL_DEBUG("MAC1ISR=%x MAC1IMR=%x\n",isr,imr2);	
			
			if(isr&MAC_RER_OVF)
			{
				MT_RINGCTRL_DEBUG("MAC1ISR - MAC_RER_OVF clear.\n");	
				WRITE_MEM32(MAC1ISR,MAC_RER_OVF);
			}

			if(isr&MAC_VTOK)
			{
				MT_RINGCTRL_DEBUG("MAC1ISR - MAC_VTOK clear.\n");	
				WRITE_MEM32(MAC1ISR,MAC_VTOK);
			}

			if(isr&MAC_TDU)
			{
				MT_RINGCTRL_DEBUG("MAC1ISR - MAC_TDU clear.\n");	
				WRITE_MEM32(MAC1ISR,MAC_TDU);
			}

			if(isr&MAC_SWINT)
			{
				MT_RINGCTRL_DEBUG("MAC1ISR - MAC_SWINT clear.\n");	
				WRITE_MEM32(MAC1ISR,MAC_SWINT);
			}			
#if 0
			if(isr&MAC_PTOK0)
			{
				MT_RINGCTRL_DEBUG("MAC1ISR - MAC_PTOK0 clear.\n");	
				WRITE_MEM32(MAC1ISR,MAC_PTOK0);
			}

			if(isr&MAC_PTOK1)
			{
				MT_RINGCTRL_DEBUG("MAC1ISR - MAC_PTOK1 clear.\n");	
				WRITE_MEM32(MAC1ISR,MAC_PTOK0);
			}
			
			if(isr&MAC_PTOK2)
			{
				MT_RINGCTRL_DEBUG("MAC1ISR - MAC_PTOK2 clear.\n");	
				WRITE_MEM32(MAC1ISR,MAC_PTOK0);
			}
			
			if(isr&MAC_PTOK3)
			{
				MT_RINGCTRL_DEBUG("MAC1ISR - MAC_PTOK3 clear.\n");	
				WRITE_MEM32(MAC1ISR,MAC_PTOK0);
			}
#endif			
		}
			

		if((isr&MAC_ROK)||(isr&MAC_RER_OVF))
		{
			
//rtlglue_printf("------------- %s %d\n",__FILE__,__LINE__);	

			tmpmacisr|=(1<<i);
#ifdef MAC_USE_TASKLET			
			tasklet_hi_schedule(&ppmac_rx_tasklets);
#else
			ppmac_rx_bh(0);
#endif			
			// clear mac-rok
			if(i==0)
			{
				MT_RINGCTRL_DEBUG("MAC0ISR - MAC_ROK clear.\n");	
				WRITE_MEM32(MAC0ISR,MAC_ROK);
			}
			else
			{
				MT_RINGCTRL_DEBUG("MAC1ISR - MAC_ROK clear.\n");				
				WRITE_MEM32(MAC1ISR,MAC_ROK);
			}
			
		}
	}
//rtlglue_printf("------------- %s %d\n",__FILE__,__LINE__);	


	WRITE_MEM32(MAC0IMR,imr1);
	WRITE_MEM32(MAC1IMR,imr2);	
//rtlglue_printf("------------- %s %d\n",__FILE__,__LINE__);	

	return IRQ_RETVAL(1);
}

#ifdef CONFIG_USB_RTL8192SU_SOFTAP
#define BIT(x)	(1 << (x))
#define FAIL							0
#define SUCCES							1
#define WLAN_ETHADDR_LEN				6
#define MACADDRLEN						6
#define WLAN_ETHHDR_LEN					14
#define WLAN_HDR_A3_LEN					24
#define WLAN_HDR_A4_LEN					30
#define WLAN_HDR_A3_QOS_LEN				26
#define WLAN_HDR_A4_QOS_LEN				32
#define WLAN_MAX_ETHFRM_LEN				1514

#define WLAN_IEEE_OUI_LEN				3
#define WLAN_PKT_FORMAT_ENCAPSULATED	0x01
#define WLAN_PKT_FORMAT_SNAP_RFC1042	0x02
#define WLAN_PKT_FORMAT_SNAP_TUNNEL		0x03
#define WLAN_PKT_FORMAT_IPX_TYPE4		0x04
#define WLAN_PKT_FORMAT_APPLETALK		0x05
#define WLAN_PKT_FORMAT_OTHERS			0x06

#define _TO_DS_		BIT(8)
#define _FROM_DS_	BIT(9)
#define _PRIVACY_	BIT(14)
#define GetToDs(pbuf)	(((*(unsigned short *)(pbuf)) & le16_to_cpu(_TO_DS_)) != 0)
#define GetFrDs(pbuf)	(((*(unsigned short *)(pbuf)) & le16_to_cpu(_FROM_DS_)) != 0)
#define get_tofr_ds(pframe)	((GetToDs(pframe) << 1) | GetFrDs(pframe))
#define GetFrameType(pbuf)	(le16_to_cpu(*(unsigned short *)(pbuf)) & (BIT(3) | BIT(2)))
#define GetFrameSubType(pbuf)	(cpu_to_le16(*(unsigned short *)(pbuf)) & (BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2)))
#define is_qos_data(pframe)	((GetFrameSubType(pframe) & (WIFI_DATA_TYPE | BIT(7))) == (WIFI_DATA_TYPE | BIT(7)))
#define GetPrivacy(pbuf)	(((*(unsigned short *)(pbuf)) & le16_to_cpu(_PRIVACY_)) != 0)

static unsigned char oui_rfc1042[] = {0x00, 0x00, 0x00};
static unsigned char oui_8021h[] = {0x00, 0x00, 0xf8};

static unsigned char SNAP_ETH_TYPE_IPX[2] = {0x81, 0x37};
static unsigned char SNAP_ETH_TYPE_APPLETALK_AARP[2] = {0x80, 0xf3};
static unsigned char SNAP_ETH_TYPE_APPLETALK_DDP[2] = {0x80, 0x9B};
static unsigned char SNAP_HDR_APPLETALK_DDP[3] = {0x08, 0x00, 0x07}; // Datagram Delivery Protocol

struct wlan_hdr {
	unsigned short	fmctrl;
	unsigned short	duration;
	unsigned char	addr1[MACADDRLEN];
	unsigned char	addr2[MACADDRLEN];
	unsigned char	addr3[MACADDRLEN];
	unsigned short	sequence;
	unsigned char	addr4[MACADDRLEN];
	unsigned short	qosctrl;
	unsigned char	iv[8];
} ;
/*
@func int32         | get_devPort_for8672_tx 
@parm net_device * | dev | net device

Given net device <p dev>, find the external port number(13~15)
*/
int32 get_devPort_for8672_tx(struct net_device *dev)
{
	int loopCnt;
	for (loopCnt=0;loopCnt<3;loopCnt++)
	{
		if ((uint32)dev==(uint32)netdev_for8672_tx[loopCnt])
			return (13+loopCnt);
	}
	return FAILED;
	
}

#if 1
#define get_netdev_for8672_tx(idx) (struct net_device*)netdev_for8672_tx[idx]
#else
/*
@func struct net_device* | get_netdev_for8672_tx 
@parm int  | idx | external port index

Given ext. port index, find the net device
*/
struct net_device* get_netdev_for8672_tx(int idx)
{
	return (struct net_device*)netdev_for8672_tx[idx];
}
#endif

/*
@func void         | set_netdev_for8672_tx 
@parm int          | idx | external port index
@parm net_device*  | dev | net device

Init device and external port index
*/
void set_netdev_for8672_tx(int idx, struct net_device *dev)
{
	int i;
	netdev_for8672_tx[idx-13]=dev;
	for (i=0;i<3;i++)
	{
		printk("netdev_for8672_tx[%d]=%p\n", i+13, netdev_for8672_tx[i]);
	}
}

void set_extDevMac(int portidx, unsigned char *addr)
{
	struct rtl8672_tblAsicDrv_intfParam_s *intftbl=NULL;
	rtl8672_tblAsicDrv_intfParam_t intf;
	
	intftbl = &intf;
	rtl8672_getAsicNetInterface(portidx, intftbl);
	intftbl->GMAC[0] = addr[0]; intftbl->GMAC[1] = addr[1]; intftbl->GMAC[2] = addr[2]; 
	intftbl->GMAC[3] = addr[3]; intftbl->GMAC[4] = addr[4]; intftbl->GMAC[5] = addr[5];
	//printk("%x:%x:%x:%x:%x:%x\n", intftbl->GMAC[0], intftbl->GMAC[1], intftbl->GMAC[2],
	//							intftbl->GMAC[3],intftbl->GMAC[4],intftbl->GMAC[5]);
	rtl8672_setAsicNetInterface(portidx, intftbl);
}

static __inline__ unsigned char	get_hdrlen(unsigned char *pframe)
{
	if (GetFrameType(pframe) == WIFI_DATA_TYPE)
	{
		if (is_qos_data(pframe)) {
			if (get_tofr_ds(pframe) == 0x03)
				return WLAN_HDR_A4_QOS_LEN;
			else
				return WLAN_HDR_A3_QOS_LEN;
		}
		else {
			if (get_tofr_ds(pframe) == 0x03)
				return WLAN_HDR_A4_LEN;
			else
				return WLAN_HDR_A3_LEN;
		}
	}
	else
	{
		return WLAN_HDR_A3_LEN;
	}
}


#endif
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
int skb_p80211_to_8023(struct sk_buff *skb)
{
	unsigned int	to_fr_ds;
	signed int		payload_length;
	signed int		payload_offset, trim_pad;
	unsigned char	daddr[WLAN_ETHADDR_LEN];
	unsigned char	saddr[WLAN_ETHADDR_LEN];
#ifdef SUPPORT_80211_FRAME_FORMAT	
	//unsigned char	da[WLAN_ETHADDR_LEN];
#endif
	unsigned char	*pframe=skb->data;
	//unsigned short	type;
	struct wlan_hdr *w_hdr;
	struct wlan_ethhdr_t   *e_hdr;
	struct wlan_llc_t      *e_llc;
	struct wlan_snap_t     *e_snap;
	int wlan_pkt_format;

	if (GetPrivacy(pframe))
	{
		printk("Do not recv. privacy packet to Packet Processor!!\n\n\n\n");
//		memDump(skb->data, skb->len, "skb_p80211_to_8023");
		return FAIL;
	}
	to_fr_ds	= get_tofr_ds(pframe);
	payload_offset = get_hdrlen(pframe);
	trim_pad = 0; // _CRCLNG_ has beed subtracted in isr
	w_hdr = (struct wlan_hdr *) pframe;

	if ( to_fr_ds == 0x00)
	{
		memcpy(daddr, (const void *)w_hdr->addr1, WLAN_ETHADDR_LEN);
		memcpy(saddr, (const void *)w_hdr->addr2, WLAN_ETHADDR_LEN);
	}
	else if( to_fr_ds == 0x01)
	{
			memcpy(daddr, (const void *)w_hdr->addr1, WLAN_ETHADDR_LEN);
			memcpy(saddr, (const void *)w_hdr->addr3, WLAN_ETHADDR_LEN);
	}
	else if( to_fr_ds == 0x02)
	{
		memcpy(daddr, (const void *)w_hdr->addr3, WLAN_ETHADDR_LEN);
		memcpy(saddr, (const void *)w_hdr->addr2, WLAN_ETHADDR_LEN);		
	}
	else
	{
		memcpy(daddr, (const void *)w_hdr->addr3, WLAN_ETHADDR_LEN);
		memcpy(saddr, (const void *)w_hdr->addr4, WLAN_ETHADDR_LEN);
	}

#if 0
	if (GetPrivacy(pframe))
	{
		printk("Do not recv. privacy packet to Packet Processor!!\n");
		return FAIL;
	}
#endif	

	payload_length = skb->len - payload_offset - trim_pad;

	if (payload_length <= 0)
	{
		printk("drop pkt due to payload_length<=0\n");
		return FAIL;
	}

	e_hdr = (struct wlan_ethhdr_t *) (pframe + payload_offset);
	e_llc = (struct wlan_llc_t *) (pframe + payload_offset);
	e_snap = (struct wlan_snap_t *) (pframe + payload_offset + sizeof(struct wlan_llc_t));

	if (e_llc->dsap==0xaa && e_llc->ssap==0xaa && e_llc->ctl==0x03)
	{

		if ( !memcmp(e_snap->oui, oui_rfc1042, WLAN_IEEE_OUI_LEN)) {
			wlan_pkt_format = WLAN_PKT_FORMAT_SNAP_RFC1042;
			if( !memcmp(&e_snap->type, SNAP_ETH_TYPE_IPX, 2) )
				wlan_pkt_format = WLAN_PKT_FORMAT_IPX_TYPE4;
			else if( !memcmp(&e_snap->type, SNAP_ETH_TYPE_APPLETALK_AARP, 2))
				wlan_pkt_format = WLAN_PKT_FORMAT_APPLETALK;
		}
		else if ( !memcmp(e_snap->oui, SNAP_HDR_APPLETALK_DDP, WLAN_IEEE_OUI_LEN) &&
					!memcmp(&e_snap->type, SNAP_ETH_TYPE_APPLETALK_DDP, 2) )
				wlan_pkt_format = WLAN_PKT_FORMAT_APPLETALK;
		else if ( !memcmp( e_snap->oui, oui_8021h, WLAN_IEEE_OUI_LEN))
			wlan_pkt_format = WLAN_PKT_FORMAT_SNAP_TUNNEL;
		else {
			return FAIL;
		}

	}
	else if ( (memcmp(daddr, e_hdr->daddr, WLAN_ETHADDR_LEN) == 0) &&
			     (memcmp(saddr, e_hdr->saddr, WLAN_ETHADDR_LEN) == 0) )
		wlan_pkt_format = WLAN_PKT_FORMAT_ENCAPSULATED;
	else
		wlan_pkt_format = WLAN_PKT_FORMAT_OTHERS;

	if ( (wlan_pkt_format == WLAN_PKT_FORMAT_SNAP_RFC1042)
			|| (wlan_pkt_format == WLAN_PKT_FORMAT_SNAP_TUNNEL) )
	{
		/* Test for an overlength frame */
		payload_length = payload_length - sizeof(struct wlan_llc_t) - sizeof(struct wlan_snap_t);
		if ( (payload_length+WLAN_ETHHDR_LEN) > WLAN_MAX_ETHFRM_LEN ) {
			/* A bogus length ethfrm has been sent. */
			/* Is someone trying an oflow attack? */
			printk("SNAP frame too large (%d>%d)\n",
				(payload_length+WLAN_ETHHDR_LEN), WLAN_MAX_ETHFRM_LEN);
		}

		/* chop 802.11 header from skb. */
		skb_pull(skb, payload_offset);
		
		/* chop llc header from skb. */
		skb_pull(skb, sizeof(struct wlan_llc_t));

		/* chop snap header from skb. */
		skb_pull(skb, sizeof(struct wlan_snap_t));
		
		/* create 802.3 header at beginning of skb. */
		e_hdr = (struct wlan_ethhdr_t *)skb_push(skb, WLAN_ETHHDR_LEN);
		
		e_hdr->type = e_snap->type;
		
		memcpy((void *)e_hdr->daddr, daddr, WLAN_ETHADDR_LEN);
		memcpy((void *)e_hdr->saddr, saddr, WLAN_ETHADDR_LEN);

		/* chop off the 802.11 CRC */
		skb_trim(skb, payload_length + WLAN_ETHHDR_LEN );
	}
	else if ( (wlan_pkt_format == WLAN_PKT_FORMAT_OTHERS)
			|| (wlan_pkt_format == WLAN_PKT_FORMAT_APPLETALK)
			|| (wlan_pkt_format == WLAN_PKT_FORMAT_IPX_TYPE4))
	{

		/* Test for an overlength frame */
		if ( (payload_length + WLAN_ETHHDR_LEN) > WLAN_MAX_ETHFRM_LEN ) {
			/* A bogus length ethfrm has been sent. */
			/* Is someone trying an oflow attack? */
			printk("IPX/AppleTalk frame too large (%d>%d)\n",
				(payload_length + WLAN_ETHHDR_LEN), WLAN_MAX_ETHFRM_LEN);
		}

		/* chop 802.11 header from skb. */
		skb_pull(skb, payload_offset);

		/* create 802.3 header at beginning of skb. */
		e_hdr = (struct wlan_ethhdr_t *)skb_push(skb, WLAN_ETHHDR_LEN);
		memcpy((void *)e_hdr->daddr, daddr, WLAN_ETHADDR_LEN);
		memcpy((void *)e_hdr->saddr, saddr, WLAN_ETHADDR_LEN);
		e_hdr->type = htons(payload_length);

		/* chop off the 802.11 CRC */
		skb_trim(skb, payload_length+WLAN_ETHHDR_LEN);
	}
	else if (wlan_pkt_format == WLAN_PKT_FORMAT_ENCAPSULATED) {

		if ( payload_length > WLAN_MAX_ETHFRM_LEN ) {
			/* A bogus length ethfrm has been sent. */
			/* Is someone trying an oflow attack? */
			printk("Encapsulated frame too large (%d>%d)\n",
				payload_length, WLAN_MAX_ETHFRM_LEN);
		}

		/* Chop off the 802.11 header. */
		skb_pull(skb, payload_offset);

		/* chop off the 802.11 CRC */
		skb_trim(skb, payload_length);
	}
	
#ifdef __KERNEL__
	skb->mac.raw = (unsigned char *) skb->data; /* new MAC header */
#endif

	return SUCCESS;
}
#endif
extern void set_wlan_vlan_member(struct sk_buff* skb);

struct net_device* L2Mac_Hash_Tbl[32]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
									NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
									NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
									NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
unsigned short rtl819x_L2Hash(uint8 *mac)
{
	unsigned short hashidx = 0;
	//printk("%s: mac:%x:%x:%x:%x:%x:%x\n", __FUNCTION__, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	hashidx = ((mac[0] << 4) | (mac[1] >> 4)) ^
		  ((mac[1] << 8) | (mac[2] >> 0)) ^
		  ((mac[3] << 4) | (mac[4] >> 4)) ^
		  ((mac[4] << 8) | (mac[5] >> 0));
	hashidx &= 0x1f;

	hashidx = ((hashidx>>4)&1) |
			((hashidx>>2)&2) |
			(hashidx&4) |
			((hashidx<<2)&8) |
			((hashidx<<4)&16);
	return hashidx;
}

#define wlan_igmp_tag 0x1f
__IRAM_PP_LOW  irqreturn_t ext_intHandler(int irq, void *dev_instance)
{
	uint32 isr;

	int i,j,startidx,endidx;
#ifndef CONFIG_USB_RTL8192SU_SOFTAP	
	//int k;
#endif
	MT_RINGCTRL("call");

	isr=READ_MEM32(EPISR);
//printk("isr = 0x%08x\n", isr);
	MT_RINGCTRL_DEBUG("EPISR=%x EPIMR=%x\n",isr,READ_MEM32(EPIMR));

#ifdef NEW_ARCH
	j=2;
#else
	for(j=0;j<EXT_INTFS;j++)
#endif		
	{

#ifdef EXT_LLIP_SIM
		if(isr& (EXT0_PTX_OK<<j))
#else
		if((isr& (EXT0_PTX_OK<<j))||(isr& (EXT0_PTX_FULL<<j)))
#endif
		{
			int ringsize;
			int loop=0;
			struct ext_Tx *extTx;
			int currentidx;
			
EXTPTX:	
			//if (j==0&&LINE==0)
				//udelay(PPDELAY);
				//udelay(200);//udelay(180);//printk("<<");
			//enum IC_TYPE type;

			currentidx=READ_MEM32(EPTXDESC0+(j<<2));
			
			switch(j)
			{
				case 0:
					ringsize=EXT_PTX_RING0_SIZE;
					break;
				case 1:
					ringsize=EXT_PTX_RING1_SIZE;
					break;
				case 2:
					ringsize=EXT_PTX_RING2_SIZE;
					break;
				default:
					ringsize=1;
			}

			startidx=cpu_ext_ptx_idx[j]%ringsize;
			endidx=currentidx;
//			printk("before startidx=%d endidx=%d\n",startidx,endidx);

			
			extTx=user_ext_ptx_idx(j,startidx);
			if ((extTx->orgAddr)==(uint32)NULL)
				extTx->orgAddr=(uint32)extTx>>2;
 			

			if(startidx>endidx) endidx+=ringsize;
			if((startidx==endidx)&&(extTx->own==1)) endidx+=ringsize;
			
			if (endidx-startidx==0)goto EXIT_EXTPTX;
			for(i=startidx;i<endidx;i++)
			{
				//if ((endidx-startidx)>1)
					//printk("%d,", endidx-startidx);
				struct ext_Tx *ptx=user_ext_ptx_idx(j,i%ringsize);
//				printk("pTx=%x intf=%d idx=%d i=%d start=%d end=%d currentidx=%d ptx->own=%d\n",(u32)ptx,j,i%ringsize,i,startidx,endidx,currentidx,ptx->own);
				
				if((ptx->own==1)&&(ptx->rsv!=1)) /* PCI device owned and not put to tx yet , try to fwd to hw */
				{
					//struct sp_pRx *ptr;
					ptx->rsv=1; //handle by network device driver
					
					MT_RINGCTRL_DEBUG("EXT TX intf=%x tx length=%d\n",2,ptx->data_length);

					{
						struct sk_buff *skb;
#if defined(EXT_LLIP_SIM) || defined(RTL867X_MODEL_USER)
#else
						struct net_device *netdev;
#endif						
						skb=(struct sk_buff *)ptx->skb_header_addr;
						skb->len=ptx->data_length;						
						skb->data=(uint8 *)ptx->tx_buffer_addr;						
						skb->tail= skb->data+skb->len;  //cathy
#if defined(EXT_LLIP_SIM) || defined(RTL867X_MODEL_USER)

						MT_RINGCTRL_DEBUG("EXT TX skb=%x(len=%d)\n",(uint32)skb,skb->len);
						IN_MT_RINGCTRL_DEBUG(
							memDump(skb->data,skb->len,"skb tx");
						)

#ifdef CONFIG_USB_RTL8192SU_SOFTAP						
						insert_skb_pool(skb);
#endif

#else
						/* test in real device */
#if 1
						if (j+MAC_TX_INTFS+SAR_INTFS==15)
						{
							unsigned short hashidx = 0;
							hashidx = rtl819x_L2Hash(&skb->data[0]);
							if (hashidx<32)
							{
								//printk("search: hashidx=%x\n", hashidx);
								skb->dev=L2Mac_Hash_Tbl[hashidx];
								if(skb->dev==NULL)
								{
									skb->dev=get_netdev_for8672_tx(j);
									printk("L2Hash fail!\n");
								}
							}
							else
								BUG();
						}
						else
#endif
						skb->dev=get_netdev_for8672_tx(j);
						if ((skb->dev)==NULL)
						{
							MT_RINGCTRL_DEBUG("netdev_for8672_tx[%d]=NULL pointer!!\n",j);

							continue;
						}

						netdev=skb->dev;		
						skb->head=(uint8 *)((uint32)skb->head&(~0x20000000));
						skb->data=(uint8 *)((uint32)skb->data&(~0x20000000));
						skb->tail=(uint8 *)((uint32)skb->tail&(~0x20000000));
						skb->end=(uint8 *)((uint32)skb->end&(~0x20000000));

						MT_RINGCTRL_DEBUG("EXT TX skb=%x(len=%d,data=%x,head=%x) netdev=%x(%s)\n",(uint32)skb,skb->len,(uint32)skb->data,(uint32)skb->head,(uint32)netdev,netdev->name);						
						IN_MT_RINGCTRL_DEBUG(
							memDump(skb->data,skb->len,"skb tx by PCI");
						)
						skb->fcpu=ptx->fcpu;

						skb->pptx=(u32*)ptx;
						
						/*xl_yue:20100204 start: add for bug that eth2wifi blocking*/
						if(eth2WifiBlockingFlag && isEthernetDomainMac(skb->data+6)){
							//printk("block lan->wifi\n");
							prealloc_skb_free(skb);
						}else
						/*xl_yue:20100204 end*/
						netdev->hard_start_xmit2(skb,netdev);								

#endif
	
					}
							
					MT_RINGCTRL_DEBUG("ptx->orgAddr=%x\n",ptx->orgAddr<<2);
							
					cpu_ext_ptx_idx[j]++;
					if(cpu_ext_ptx_idx[j]==ringsize) cpu_ext_ptx_idx[j]=0;

				}

			}

			loop++;
			if(loop<=2) goto EXTPTX;
EXIT_EXTPTX:	;

			WRITE_MEM32(EPISR,EXT0_PTX_OK<<j);

		}

//ext_ptx_ok_skip:

		if(isr& (EXT0_VRX_OK<<j))
		{
			int currentidx=READ_MEM32(EVRXDESC0+(j<<2));
			struct ext_Rx *extRx;
			startidx=cpu_ext_vrx_idx[j];
			endidx=currentidx;
			extRx=user_ext_vrx_idx(j,startidx);
			if(startidx>endidx) endidx+=EXT_VRX_RING_SIZE;
			if((startidx==endidx)&&(extRx->own==0)) endidx+=EXT_VRX_RING_SIZE;
			for(i=startidx;i<endidx;i++)
			{
				struct ext_Rx *vrx=user_ext_vrx_idx(j,i%EXT_VRX_RING_SIZE);
				if(vrx->own==0) /* Driver owned and not received to PS */
				{
					struct sk_buff *skb;	
					{
											
						skb=(struct sk_buff *)vrx->skb_header_addr;
#ifdef EXT_LLIP_SIM	// FREE the VRX cluster					
						skb->len=vrx->data_length;						
						skb->data=(uint8 *)vrx->rx_buffer_addr+vrx->rx_shift;	
						skb->tail=skb->data+skb->len;							
						MT_RINGCTRL_DEBUG("EXT RX(intHandler) skb=%x(len=%d)\n",(uint32)skb,skb->len);
						IN_MT_RINGCTRL_DEBUG(
							memDump(skb->data,skb->len,"skb vrx");
						)

#ifdef CONFIG_USB_RTL8192SU_SOFTAP					
						insert_skb_pool(skb);
#endif
#else /* test in real device */	
						
						skb->len=vrx->data_length;						
						skb->data=(uint8 *)vrx->rx_buffer_addr+vrx->rx_shift;	
						skb->tail=skb->data+skb->len;
						
						MT_RINGCTRL_DEBUG("EXT1 Recv 1 packets from vRx, reason=%d, length=%d.(swap vrx skb=%x, new skb=%x)\n",vrx->reason,skb->len,(uint32)skb,(uint32)skb2);
						IN_MT_RINGCTRL_DEBUG(
							memDump(skb->data,skb->len,"skb vrx");
						);



						if (j==0)
						{
							skb_p80211_to_8023(skb);
						}
//						============ learning l2 mac address in net device drver ======================
						rtl8672_l2learning(&skb->data[6], j+MAC_TX_INTFS+SAR_INTFS,skb->dev->vlanid);
						if(!(skb->data[0]&1))
						{
							if (j+MAC_TX_INTFS+SAR_INTFS==15)
							{
								unsigned short hashidx = 0;
								hashidx = rtl819x_L2Hash(&skb->data[6]);
								if (hashidx<32)
								{
									//printk("add: hashidx=%x\n", hashidx);
									L2Mac_Hash_Tbl[hashidx]=skb->dev;
								}
								else
									BUG();
							}
						}

						
						{
							uint8 *head,*data,*tail,*end;
							struct sk_buff *newskb;
							newskb=
								#if 0//def CONFIG_SKB_POOL_PRIV
								(struct sk_buff *)(((uint32)priv_skb_alloc(BUFFER_SIZE)));
								#else
								(struct sk_buff *)(((uint32)dev_alloc_skb(BUFFER_SIZE)));
								#endif
							if(newskb==NULL)
							{	
								if (printk_ratelimit())
									printk("allocate failed at: %s %d\n",__FUNCTION__,__LINE__);
								init_skb(skb,BUFFER_SIZE);								
							} else {
								//printk("%s %d allocskb=%x reason=%d\n",__FUNCTION__,__LINE__,(u32)newskb,vrx->reason);	
								init_skb(newskb,BUFFER_SIZE);
								newskb->fcpu = 1;
								head=newskb->head;
								data=newskb->data;
								tail=newskb->tail;
								end=newskb->end;
								newskb->len=skb->len;
								newskb->dev=skb->dev;							
								newskb->head=(uint8 *)(((uint32)skb->head)&(~0x20000000));
								newskb->data=(uint8 *)(((uint32)skb->data)&(~0x20000000));
								newskb->tail=(uint8 *)(((uint32)skb->tail)&(~0x20000000));
								newskb->end=(uint8 *)(((uint32)skb->end)&(~0x20000000));
								skb->head=head;
								skb->data=data;
								skb->tail=tail;
								skb->end=end;
								skb->len=0;
								MT_RINGCTRL_DEBUG("newskb->dataref=%d newskb->nr_frags=%d skb->users=%d\n",atomic_read(&skb_shinfo(newskb)->dataref),skb_shinfo(newskb)->nr_frags,atomic_read(&skb->users) );
								
								newskb->protocol = eth_type_trans (newskb, newskb->dev);
								if(*(newskb->mac.raw) & 0x1)
									pp_check_igmp_snooping_rx(newskb,wlan_igmp_tag);
								set_wlan_vlan_member(newskb);
								netif_rx(newskb);						
							}
						}
#endif
					}				

					// swap a empty descriptor into vRx ring.
					{
						rtl8672_tblAsicDrv_intfParam_t intf;						
						//last_skb=skb;
						rtl8672_getAsicNetInterface(SAR_INTFS+MAC_TX_INTFS+j, &intf);
						skb_reserve(skb,intf.RXshift);
						vrx->skb_header_addr=(uint32)skb;
						vrx->rx_shift=intf.RXshift;
						vrx->rx_buffer_addr=(uint32)skb->data-intf.RXshift;						
						vrx->data_length=READ_MEM32(BUFFERSIZE);
						vrx->own=1;
					}			

					cpu_ext_vrx_idx[j]=(cpu_ext_vrx_idx[j]+1)%EXT_VRX_RING_SIZE;
				
				}
				WRITE_MEM32(EPISR,EXT0_VRX_OK<<j);			

			}		
		}


		if(isr& (EXT0_PTX_FULL<<j))
		{
			MT_RINGCTRL_DEBUG("*** PTX %d full\n",j);
			//printk("*** PTX%d full\n",j);
			//rtl8672_dumpRing(3,SAR_INTFS+MAC_TX_INTFS+j);
			IN_MT_RINGCTRL(							
			rtl8672_dumpRing(3,SAR_INTFS+MAC_TX_INTFS+j);
			);
			WRITE_MEM32(EPISR,EXT0_PTX_FULL<<j);
		}

		if(isr& (EXT0_VRX_FULL<<j))
		{
			MT_RINGCTRL_DEBUG("******* VRX Ring %d full\n",j);
			//printk("******* VRX Ring %d full\n",j);
			IN_MT_RINGCTRL(							
			rtl8672_dumpRing(1,SAR_INTFS+MAC_TX_INTFS+j);
			);
			WRITE_MEM32(EPISR,EXT0_VRX_FULL<<j);
		}		

		if(isr& (EXT0_VTX_EMPTY<<j))
		{
			MT_RINGCTRL_DEBUG("******* VTX Ring %d empty\n",j);
			IN_MT_RINGCTRL(				
			rtl8672_dumpRing(2,SAR_INTFS+MAC_TX_INTFS+j);
			);
			WRITE_MEM32(EPISR,EXT0_VTX_EMPTY<<j);
		}		
		
	}
	MT_RINGCTRL("out");
	
	return IRQ_RETVAL(1);
}

static int kill_all(void *ptr, void *user_data) {
	rtl8672_tblAsicDrv_l2Param_t *l2p = ptr;						
	rtl8672_delAsicL2Entry(l2p, OPT_AUTO);
	return 0;
}


int rtl8672_l2flush(void) {
	rtl8672_L2EntrySearch(kill_all,0);
	return 0;
}

int rtl8672_l2learning(unsigned char *smac,int intfidx,int vlanid)
{
	rtl8672_tblAsicDrv_l2Param_t l2pt;
	//patch for packet processor vlan limitation to support port mapping
	//if(enable_port_mapping)
		//return SUCCESS;
	// Kaohj -- not learning if VLAN_GROUPING enabled (trap all packets to CPU).
	if (enable_vlan_grouping)
		return SUCCESS;
	if((smac[0]&1)||(vlanid==0)) return SUCCESS; /* multicast or broadcast address is not need to learn. */
	if(rtl8672_L2EntryLookup(smac,vlanid,&l2pt)==SUCCESS){ 
		return SUCCESS; //entry already exist 
	}

	printk("L2 learning:%x %x %x %x %x %x from idx:%d  vlanid:%d\n",smac[0],smac[1],smac[2],smac[3],smac[4],smac[5],intfidx,vlanid);
	memcpy(l2pt.MAC,smac,6);
	l2pt.VlanID=vlanid;
	l2pt.Port=intfidx;
	l2pt.Age=0;
	l2pt.PriorityIDRemr=0;

#if 1
	if(enable_port_mapping)
	{
		if(intfidx<8 || intfidx>13)
			l2pt.OTagIf=0;
		else
			l2pt.OTagIf=1;
	}
#endif

	l2pt.SADrop=0;
	l2pt.Dot1PRemr=0;
	l2pt.Valid=1;
	memset(l2pt.APMAC,0,6);
	return rtl8672_addAsicL2Entry(&l2pt, OPT_AUTO);	

}

int rtl8672_l2delete(unsigned char *smac, int intfidx, int vlanid, struct net_device *dev)
{
	rtl8672_tblAsicDrv_l2Param_t l2pt;
	unsigned short hashidx = 0;
	//patch for packet processor vlan limitation to support port mapping
	//if(enable_port_mapping)
		//return SUCCESS;
	if (enable_vlan_grouping)
		return SUCCESS;
	if(smac[0]&1) return SUCCESS; /* multicast or broadcast address is not need to learn. */
	if(rtl8672_L2EntryLookup(smac,vlanid,&l2pt)!=SUCCESS){ 
		return SUCCESS;
	}
	
	if (intfidx==15)
	{
		hashidx = rtl819x_L2Hash(smac);
		if (hashidx<32)
		{
			//printk("del: hashidx=%x\n", hashidx);
			L2Mac_Hash_Tbl[hashidx]=NULL;
		}
		else
			BUG();
	}
	printk("L2 delete:%x %x %x %x %x %x from idx:%d  vlanid:%d\n",smac[0],smac[1],smac[2],smac[3],smac[4],smac[5],intfidx,vlanid);
	memcpy(l2pt.MAC,smac,6);
	l2pt.VlanID=vlanid;
	l2pt.Port=intfidx;
	l2pt.Age=0;
	l2pt.PriorityIDRemr=0;

	l2pt.SADrop=0;
	l2pt.Dot1PRemr=0;
	l2pt.Valid=1;
	memset(l2pt.APMAC,0,6);
	return rtl8672_delAsicL2Entry(&l2pt, OPT_AUTO);	

}


module_init(init_ring);




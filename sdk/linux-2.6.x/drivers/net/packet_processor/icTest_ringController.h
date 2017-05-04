
#ifndef CONFIG_USB_RTL8192SU_SOFTAP

#if defined(CONFIG_RLE0437) || defined(CONFIG_RTL8681)
#define SRAM_MAPPING_ENABLED
//#define SRAM_PROTECTION

#define SAR_PRX_RING_SIZE 8
#define SAR_PTX_RING_SIZE 8
#define SAR_VRX_RING_SIZE 4
#define SAR_VTX_RING_SIZE 4
#define PTM_PRX_RING_SIZE 64
#define PTM_PTX_RING_SIZE 16
#define PTM_VRX_RING_SIZE 4
#define PTM_VTX_RING_SIZE 2

#define MAC_PRX_RING_SIZE 64
#define MAC_PTX_RING_SIZE 2 /* just for 256 byte aligment issue, each descriptor = 20bytes, 20 *64 =1280 = 0x500 */
#define MAC_VRX_RING_SIZE 2
#define MAC_VTX_RING_SIZE 2
#define EXT_PRX_RING_SIZE 1
#define EXT_PTX_RING_SIZE 1
#define EXT_VRX_RING_SIZE 1
#define EXT_VTX_RING_SIZE 1

//czyao 0437FPGA
#define MAC_PTX_RING0_SIZE 4
#define MAC_PTX_RING1_SIZE 64
#define MAC_PTX_RING2_SIZE 4
#define MAC_PTX_RING3_SIZE 4
#define MAC_PTX_RING4_SIZE 4
#define EXT_PTX_RING0_SIZE 1
#define EXT_PTX_RING1_SIZE 1
#define EXT_PTX_RING2_SIZE 1

//czyao 0513 ASIC
#define PTM_PTX_RING0_SIZE	64
#define PTM_PTX_RINGx_SIZE	2

#define PTM_PRX_RING0_SIZE	64
#define PTM_PRX_RINGx_SIZE	2

#define BUFFER_SIZE CONFIG_RTL867X_PREALLOCATE_SKB_SIZE

#else
#define SAR_PRX_RING_SIZE 8
#define SAR_PTX_RING_SIZE 8
#define SAR_VRX_RING_SIZE 8
#define SAR_VTX_RING_SIZE 8

#define MAC_PRX_RING_SIZE 4
#define MAC_PTX_RING_SIZE 8 /* just for 256 byte aligment issue, each descriptor = 20bytes, 20 *64 =1280 = 0x500 */
#define MAC_VRX_RING_SIZE 16
#define MAC_VTX_RING_SIZE 16
#define EXT_PRX_RING_SIZE 4
#define EXT_PTX_RING_SIZE 8
#define EXT_VRX_RING_SIZE 16
#define EXT_VTX_RING_SIZE 8

//czyao 0437FPGA
#define MAC_PTX_RING0_SIZE 8
#define MAC_PTX_RING1_SIZE 8
#define MAC_PTX_RING2_SIZE 8
#define MAC_PTX_RING3_SIZE 8
#define MAC_PTX_RING4_SIZE 8
#define EXT_PTX_RING0_SIZE 8
#define EXT_PTX_RING1_SIZE 8
#define EXT_PTX_RING2_SIZE 8
#define BUFFER_SIZE CONFIG_RTL867X_PREALLOCATE_SKB_SIZE
#endif

#else

//#define BUFFER_HDR_IN_SRAM
#define SRAM_MAPPING_ENABLED

#define SAR_PRX_RING_SIZE 2
#define SAR_PTX_RING_SIZE 12
#define SAR_VRX_RING_SIZE 16
#define SAR_VTX_RING_SIZE 4
#define MAC_PRX_RING_SIZE 8
//#define MAC_PTX_RING_SIZE 16 /* just for 256 byte aligment issue, each descriptor = 20bytes, 20 *64 =1280 = 0x500 */
#define MAC_PTX_RING0_SIZE 0
#ifdef BUFFER_HDR_IN_SRAM
#define MAC_PTX_RING1_SIZE 90 //90 sometime lose
#else
#define MAC_PTX_RING1_SIZE (256-64-(14*8))
#endif
#define MAC_PTX_RING2_SIZE (2+62)
#define MAC_PTX_RING3_SIZE 2
#define MAC_PTX_RING4_SIZE 2

#define MAC_VRX_RING_SIZE 40
#define MAC_VTX_RING_SIZE 32
#define EXT_PRX_RING_SIZE 4
//#define EXT_PTX_RING_SIZE 24
#define EXT_PTX_RING0_SIZE 0
#define EXT_PTX_RING1_SIZE 0
#ifdef BUFFER_HDR_IN_SRAM
#define EXT_PTX_RING2_SIZE 42
#else
#define EXT_PTX_RING2_SIZE (176-62)
#endif
#define EXT_VRX_RING_SIZE 16
#define EXT_VTX_RING_SIZE 8

#define BUFFER_SIZE CONFIG_RTL867X_PREALLOCATE_SKB_SIZE
#endif
#define DMA_SHIFT NET_SKB_PAD
//#define PS_LOOPBACK_MODE 1


#include "icModel_ringController.h"
#ifdef RTL867X_MODEL_KERNEL
#include <linux/netdevice.h>
extern unsigned char *last_data;  /* this data is valid before queue is reused, from: 8139cp.c */
extern unsigned int last_data_len; /* from 8139cp.c */
#endif

// all descriptors

extern struct mac_pRx *user_mac_prx;
extern struct sar_pRx *user_sar_prx;
extern struct ext_Rx *user_ext_prx;

#if 0
extern struct mac_pTx *user_mac_ptx;
extern struct sar_pTx *user_sar_ptx;
extern struct ext_Tx *user_ext_ptx;
#else
extern struct mac_pTx *user_mac_ptxs[5];
extern struct ext_Tx *user_ext_ptx[3];
#endif
extern struct sar_pTx *user_sar_ptx;


extern struct mac_vRx *user_mac_vrx;
extern struct sar_vRx *user_sar_vrx;
extern struct ext_Rx *user_ext_vrx;

extern struct mac_vTx *user_mac_vtx;
extern struct sar_vTx *user_sar_vtx;
extern struct ext_Tx *user_ext_vtx;

extern struct sp_pRx *user_sp_to_mac[5];
extern struct sp_pRx *user_sp_to_sar;
extern struct sp_pRx *user_sp_to_ext[3];


struct mac_pRx *user_mac_prx_idx(int intf,int ring);
struct sar_pRx *user_sar_prx_idx(int intf,int ring);
struct ext_Rx *user_ext_prx_idx(int intf,int ring);
struct mac_pTx *user_mac_ptx_idx(int intf,int ring);
struct sar_pTx *user_sar_ptx_idx(int intf,int ring);
struct ext_Tx *user_ext_ptx_idx(int intf,int ring);
struct mac_vRx *user_mac_vrx_idx(int intf,int ring);
struct sar_vRx *user_sar_vrx_idx(int intf,int ring);
struct ext_Rx *user_ext_vrx_idx(int intf,int ring);
struct mac_vTx *user_mac_vtx_idx(int intf,int ring);
struct sar_vTx *user_sar_vtx_idx(int intf,int ring);
struct ext_Tx *user_ext_vtx_idx(int intf,int ring);
struct sp_pRx *user_sp_to_mac_idx(int intf,int ring);
struct sp_pRx *user_sp_to_sar_idx(int intf,int ring);
struct sp_pRx *user_sp_to_ext_idx(int intf,int ring);

int32 testRingCtrl_from_sar_prx_to_ptx_but_ptx_full(uint32 caseNo);
int32 testRingCtrl_from_sar_prx_to_ptx(uint32 caseNo);
int32 testRingCtrl_from_sar_prx_and_prx_full(uint32 caseNo);
int32 testRingCtrl_from_sar_prx_to_vrx(uint32 caseNo);
int32 testRingCtrl_from_sar_prx_to_vrx_but_vrx_full(uint32 caseNo);
int32 testRingCtrl_from_sar_vtx_to_ptx(uint32 caseNo);
int32 testRingCtrl_from_sar_vtx_to_ptx_but_ptx_full(uint32 caseNo);

int32 testRingCtrl_from_mac_prx_to_ptx_but_ptx_full(uint32 caseNo);
int32 testRingCtrl_from_mac_prx_to_ptx(uint32 caseNo);
int32 testRingCtrl_from_mac_prx_and_prx_full(uint32 caseNo);
int32 testRingCtrl_from_mac_prx_to_vrx(uint32 caseNo);
int32 testRingCtrl_from_mac_prx_to_vrx_but_vrx_full(uint32 caseNo);
int32 testRingCtrl_from_mac_vtx_to_ptx(uint32 caseNo);
int32 testRingCtrl_from_mac_vtx_to_ptx_but_ptx_full(uint32 caseNo);

int32 testRingCtrl_from_ext_prx_to_ptx_but_ptx_full(uint32 caseNo);
int32 testRingCtrl_from_ext_prx_to_ptx(uint32 caseNo);
int32 testRingCtrl_from_ext_prx_and_prx_full(uint32 caseNo);
int32 testRingCtrl_from_ext_prx_to_vrx(uint32 caseNo);
int32 testRingCtrl_from_ext_prx_to_vrx_but_vrx_full(uint32 caseNo);
int32 testRingCtrl_from_ext_vtx_to_ptx(uint32 caseNo);
int32 testRingCtrl_from_ext_vtx_to_ptx_but_ptx_full(uint32 caseNo);
int32 testRingCtrl_init_ring(uint32 caseNo);

int32 testRingCtrl_from_mac_prx_to_ptx_with_phy(uint32 caseNo);

int32 testRingCtrl_Registers(uint32 caseNo);
//int init_ring(void);

#ifdef  RTL867X_MODEL_USER
struct pt_regs
{
	uint32 rsv;
};
#endif

#ifdef CONFIG_RTL8672_SAR
 irqreturn_t sar_intHandler(int irq, void *dev_instance);
#endif
#ifdef CONFIG_RTL8681_PTM
  irqreturn_t ptm_intHandler(int irq, void *dev_instance);
#endif
 irqreturn_t mac_intHandler(int irq, void *dev_instance);
 irqreturn_t ext_intHandler(int irq, void *dev_instance);

int32 get_devPort_for8672_tx(struct net_device *dev);
struct net_device* get_netdev_for8672_tx(int idx);
void set_netdev_for8672_tx(int idx, struct net_device *dev);
void set_extDevMac(int portidx, unsigned char *addr);
void set_pRx_own(struct ext_Tx *ptx, int ext_port);

//#define SKB_POOL_DEBUG 1
#ifdef SKB_POOL_DEBUG
int insert_skb_pool2(struct sk_buff *skb,char *free_func,int free_line);
#define insert_skb_pool(x) \
		do{ \
			MT_RINGCTRL_DEBUG("insert_skb_pool: %s %d\n",__FUNCTION__,__LINE__); \
			insert_skb_pool2(x,__FUNCTION__,__LINE__); \
		} while(0);
#else
int insert_skb_pool(struct sk_buff *skb);

#endif

struct sk_buff *get_free_skb(void);


int rtl8672_SMac_learning(uint8 *smac, uint8 *apmac, uint16 myvid, uint32 portidx);
struct sk_buff *rtl8672_extPortRecv(void *id, uint8 *data,  uint32 len, uint16 myvid, uint32 portidx, uint32 linkID);
int rtl8672_mac_vtx_start_xmit (struct sk_buff *skb, struct net_device *dev);
int rtl8672_l2flush(void);
int rtl8672_l2learning(unsigned char *smac,int intfidx,int vlanid);
int rtl8672_l2delete(unsigned char *smac,int intfidx,int vlanid, struct net_device *dev);

void rtl8672_send_pkt_by_rtl8139(int intf,unsigned char *data, int len);


#if 1//def RTL867X_MODEL_KERNEL
//int rtl8672_vtx_start_xmit (struct sk_buff *skb, struct net_device *dev);
int rtl8672_vtx_start_xmit (struct sk_buff *skb, struct net_device *dev, int tx_port_idx);
#endif

struct sk_buff *get_free_skb(void);
//void skb_reserve(struct sk_buff *skb, unsigned int len);




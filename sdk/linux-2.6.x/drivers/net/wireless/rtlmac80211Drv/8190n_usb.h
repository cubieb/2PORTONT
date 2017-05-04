
#ifndef _8190N_USB_H_
#define _8190N_USB_H_

#include <linux/version.h>

#ifdef __KERNEL__
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/in.h>
#include <linux/if.h>
#include <linux/ip.h>
#include <asm/io.h>
#include <linux/skbuff.h>
#include <linux/socket.h>
#include <linux/fs.h>
#include <linux/major.h>
#include <linux/fcntl.h>
#include <linux/signal.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#endif

#include <linux/nl80211.h>
#include <net/mac80211.h>

#include <linux/usb.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21))
#include <linux/usb_ch9.h>
#else
#include <linux/usb/ch9.h>
#endif

#include "8190n_cfg.h"
#include "8190n.h"
#include "8190n_hw.h"
#include "8190n_headers.h"
#include "8190n_debug.h"


struct mac80211_shared_priv;
struct priv_shared_hw;
struct tx_info;

#ifndef TRUE
	#define _TRUE	1
#else
	#define _TRUE	TRUE	
#endif
		
#ifndef FALSE		
	#define _FALSE	0
#else
	#define _FALSE	FALSE	
#endif

#define FIELD_OFFSET(s,field)	((int)&((s*)(0))->field)

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

static __inline u32 _RND128(u32 sz)
{
	u32	val;
	val = ((sz >> 7) + ((sz & 127) ? 1: 0)) << 7;	
	return val;
}

//defined for TX DESC Operation
//
#define MAX_TID (15)
//OFFSET 0
#define OFFSET_SZ (0)
#define OFFSET_SHT (16)
#define OWN 	BIT(31)
#define FSG	BIT(27)
#define LSG	BIT(26)
//OFFSET 4
#define PKT_OFFSET_SZ (0)
#define QSEL_SHT (8)
#define HWPC BIT(31)
//OFFSET 8
#define BMC BIT(7)
#define BK BIT(30)
#define AGG_EN BIT(29)
//OFFSET 12
#define SEQ_SHT (16)
//OFFSET 16
#define TXBW BIT(18)
//OFFSET 20
#define DISFB BIT(15)

#define TXDESC_SIZE 32
#define TXDESC_OFFSET TXDESC_SIZE

struct rtl8192s_txdesc{	
	unsigned int txdw0;
	unsigned int txdw1;
	unsigned int txdw2;
	unsigned int txdw3;
	unsigned int txdw4;
	unsigned int txdw5;
	unsigned int txdw6;
	unsigned int txdw7;
};

struct rtl8192s_rxdesc{
	unsigned int rxdw0;
	unsigned int rxdw1;
	unsigned int rxdw2;
	unsigned int rxdw3;
	unsigned int rxdw4;
	unsigned int rxdw5;	
};

#define	RTL871X_VENQT_READ	0xc0
#define	RTL871X_VENQT_WRITE	0x40

#define RTL8712_IOBASE_FF	0x10300000	//IOBASE_FIFO 0x1031000~0x103AFFFF 
#define RTL8712_DMA_BCNQ		(RTL8712_IOBASE_FF + 0x10000)
#define RTL8712_DMA_MGTQ		(RTL8712_IOBASE_FF + 0x20000)
#define RTL8712_DMA_BMCQ		(RTL8712_IOBASE_FF + 0x30000)
#define RTL8712_DMA_VOQ		(RTL8712_IOBASE_FF + 0x40000)
#define RTL8712_DMA_VIQ		(RTL8712_IOBASE_FF + 0x50000)
#define RTL8712_DMA_BEQ		(RTL8712_IOBASE_FF + 0x60000)
#define RTL8712_DMA_BKQ		(RTL8712_IOBASE_FF + 0x70000)
#define RTL8712_DMA_RX0FF		(RTL8712_IOBASE_FF + 0x80000)
#define RTL8712_DMA_H2CCMD	(RTL8712_IOBASE_FF + 0x90000)
#define RTL8712_DMA_C2HCMD	(RTL8712_IOBASE_FF + 0xA0000)


// The following two definition are only used for USB interface.
#if 0
#define	RF_BB_CMD_ADDR			0x02c0	// RF/BB read/write command address.
#define	RF_BB_CMD_DATA			0x02c4	// RF/BB read/write command data.
#else
#define	RF_BB_CMD_ADDR		0x10250370
#define	RF_BB_CMD_DATA		0x10250374
#endif

#define IOCMD_CTRL_ADDR		RF_BB_CMD_ADDR
#define IOCMD_DATA_ADDR		RF_BB_CMD_DATA


enum {
	DEVICE_RTL8187,
	DEVICE_RTL8187B,
	DEVICE_RTL8192S,
	DEVICE_RTL8712,
};



struct fw_priv {   //8-bytes alignment required

 //--- long word 0 ----
 unsigned char  signature_0;  //0x12: CE product, 0x92: IT product
 unsigned char  signature_1;  //0x87: CE product, 0x81: IT product
 unsigned char  hci_sel;   //0x81: PCI-AP, 01:PCIe, 02: 92S-U, 0x82: USB-AP, 0x12: 72S-U, 03:SDIO
 unsigned char  chip_version; //the same value as reigster value 
 unsigned char  customer_ID_0; //customer  ID low byte
 unsigned char  customer_ID_1; //customer  ID high byte
 unsigned char  rf_config;  //0x11:  1T1R, 0x12: 1T2R, 0x92: 1T2R turbo, 0x22: 2T2R
 unsigned char  usb_ep_num;  // 4: 4EP, 6: 6EP, 11: 11EP
 
 //--- long word 1 ----
 unsigned char  regulatory_class_0; //regulatory class bit map 0
 unsigned char  regulatory_class_1; //regulatory class bit map 1
 unsigned char  regulatory_class_2; //regulatory class bit map 2
 unsigned char  regulatory_class_3; //regulatory class bit map 3 
 unsigned char  rfintfs;    // 0:SWSI, 1:HWSI, 2:HWPI
 unsigned char  def_nettype;  //
 unsigned char  turboMode;
 unsigned char  rsvd011; 
 
 //--- long word 2 ----
 unsigned char  lbk_mode; //0x00: normal, 0x03: MACLBK, 0x01: PHYLBK
 unsigned char  mp_mode; // 1: for MP use, 0: for normal driver (to be discussed)
 unsigned char  vcsType; /* 0:off 1:on 2:auto */
 unsigned char  vcsMode; /* 1:RTS/CTS 2:CTS to self */
 unsigned char  rsvd022;
 unsigned char  rsvd023;
 unsigned char  rsvd024;
 unsigned char  rsvd025;
 
 //--- long word 3 ----
 unsigned char  qos_en;    //1: QoS enable
 unsigned char  bw_40MHz_en;   //1: 40MHz BW enable
 unsigned char  AMSDU2AMPDU_en;   //1: 4181 convert AMSDU to AMPDU, 0: disable
 unsigned char  AMPDU_en;   //1: 11n AMPDU enable
 unsigned char  rate_control_offload;  //1: FW offloads, 0: driver handles
 unsigned char  aggregation_offload;  //1: FW offloads, 0: driver handles
 unsigned char  rsvd030;
 unsigned char  rsvd031;

 //--- long word 4 ----
 unsigned char  beacon_offload;   // 1. FW offloads, 0: driver handles
 unsigned char  MLME_offload;   // 2. FW offloads, 0: driver handles
 unsigned char  hwpc_offload;   // 3. FW offloads, 0: driver handles
 unsigned char  tcp_checksum_offload; // 4. FW offloads, 0: driver handles
 unsigned char  tcp_offload;    // 5. FW offloads, 0: driver handles
 unsigned char  ps_control_offload;  // 6. FW offloads, 0: driver handles
 unsigned char  WWLAN_offload;   // 7. FW offloads, 0: driver handles
 unsigned char  rsvd040;
 
 //--- long word 5 ----
 unsigned char  tcp_tx_frame_len_L;  //tcp tx packet length low byte
 unsigned char  tcp_tx_frame_len_H;  //tcp tx packet length high byte
 unsigned char  tcp_rx_frame_len_L;  //tcp rx packet length low byte
 unsigned char  tcp_rx_frame_len_H;  //tcp rx packet length high byte
 unsigned char  rsvd050;
 unsigned char  rsvd051;
 unsigned char  rsvd052;
 unsigned char  rsvd053;
 
};

 
struct fw_hdr{//8-byte alinment required

	unsigned short	signature;//
	unsigned short	version;//0x8000 ~ 0x8FFF for FPGA version,	//0x0000 ~ 0x7FFF for ASIC version,
	
	unsigned int		dmem_size;    //define the size of boot loader

	unsigned int		img_IMEM_size;    //define the size of FW in IMEM
	
	unsigned int		img_SRAM_size;    //define the size of FW in SRAM

	unsigned int		fw_priv_sz;       //define the size of DMEM variable 
	
	unsigned short	efuse_addr;
	unsigned short 	h2ccnd_resp_addr;
	
	unsigned short	debug_dump_addr;
	unsigned short	rsvd1;
	
	unsigned int  		release_time; //Mon:Day:Hr:Min
	
	struct fw_priv 	fwpriv;	
	
};


struct urb_cb_info{
	struct urb *urb;
	struct mac80211_shared_priv *priv;
};

int usb_configure(struct priv_shared_hw *pshared_hw);

extern u8 read8(struct mac80211_shared_priv *priv, u32 addr);
extern u16 read16(struct mac80211_shared_priv *priv, u32 addr);
extern u32 read32(struct mac80211_shared_priv *priv, u32 addr);
extern void write8(struct mac80211_shared_priv *priv, u32 addr, u8 val);
extern void write16(struct mac80211_shared_priv *priv, u32 addr, u16 val);
extern void write32(struct mac80211_shared_priv *priv, u32 addr, u32 val);
extern void write8_async(struct mac80211_shared_priv *priv, u32 addr, u8 val);
extern void write16_async(struct mac80211_shared_priv *priv, u32 addr, u16 val);
extern void write32_async(struct mac80211_shared_priv *priv, u32 addr, u32 val);

extern unsigned int phy_QueryUsbBBReg(struct mac80211_shared_priv *priv, unsigned int RegAddr);
extern void phy_SetUsbBBReg(struct mac80211_shared_priv *priv, u32 RegAddr, u32 Data);
extern unsigned int phy_QueryUsbRFReg(struct mac80211_shared_priv *priv, RF90_RADIO_PATH_E eRFPath, unsigned int Offset);
extern void phy_SetUsbRFReg(struct mac80211_shared_priv *priv, RF90_RADIO_PATH_E eRFPath, unsigned int RegAddr, unsigned int Data);



extern int rtl819x_init_hw_USB(struct mac80211_shared_priv *priv);
int rtl8192s_usb_recv(struct mac80211_shared_priv *priv);
int rtl8192s_usb_xmit(struct mac80211_shared_priv *priv, struct sk_buff *skb, struct tx_info *txinfo);


size_t rtl8192s_open_fw(struct mac80211_shared_priv *priv, void **pphfwfile_hdl, u8 **ppmappedfw);
void rtl8192s_close_fw(struct mac80211_shared_priv *priv, void *phfwfile_hdl);
void fill_fwpriv(struct mac80211_shared_priv *priv, struct fw_priv *pfwpriv);
void update_fwhdr(struct fw_hdr *pfwhdr, u8* pmappedfw);
int chk_fwhdr(struct fw_hdr *pfwhdr, u32 ulfilelength);

int rtl8192SU_Write_FWIMG(struct mac80211_shared_priv *priv, void *src, size_t sz, int type);

int rtl819x_pos_init(struct mac80211_shared_priv *priv);
int rtl819x_fw_init(struct mac80211_shared_priv *priv);
int rtl819x_mac_init(struct mac80211_shared_priv *priv);
int rtl819x_bb_init(struct mac80211_shared_priv *priv);
int rtl819x_rf_init(struct mac80211_shared_priv *priv);
int rtl819x_misc_init(struct mac80211_shared_priv *priv);

#endif


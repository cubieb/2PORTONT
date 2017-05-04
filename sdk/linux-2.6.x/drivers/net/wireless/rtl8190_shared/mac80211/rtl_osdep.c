/*-----------------------------------------------------------------------------
	This file is for OS related functions.
------------------------------------------------------------------------------*/
#ifdef __KERNEL__
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

#ifdef RTL8192SU
#include "8190n_usb.h"
#else
#include <linux/pci.h>
#endif

#include "../8190n_cfg.h"

#ifdef __KERNEL__
#ifdef __LINUX_2_6__
#include <linux/syscalls.h>
#include <linux/file.h>
#include <asm/unistd.h>
#endif
#if defined(RTK_BR_EXT) || defined(BR_SHORTCUT)
#ifdef __LINUX_2_6__
#include <linux/syscalls.h>
#else
#include <linux/fs.h>
#endif
#include <../net/bridge/br_private.h>
#endif
#else
#include "./sys-support.h"
#endif

#include "../8190n.h"
#include "../8190n_hw.h"
#include "../8190n_headers.h"
#include "../8190n_rx.h"
#include "../8190n_debug.h"
#include "./rtl_rc.h"

static struct rtl8190_hw hw_info;
static struct priv_shared_hw shared_hw;

#define SUPPORTED_FILTERS			\
	(FIF_PROMISC_IN_BSS |			\
	FIF_ALLMULTI |				\
	FIF_CONTROL |				\
	FIF_OTHER_BSS |				\
	FIF_FCSFAIL)	|		\
	FIF_BCN_PRBRESP_PROMISC

static const struct net_device_ops rtl_wireless_netdev_ops = {
	.ndo_do_ioctl		= rtl819x_ioctl,
};
	
#ifdef RTL8192SE

int RetryCounter1=0;
static struct pci_driver rtl8190_driver;
static __inline__ int __rtl8192SE_interrupt(void *dev_instance);
int pci_init_sw(struct mac80211_shared_priv *priv);
int pci_stop_sw(struct mac80211_shared_priv *priv);

#endif

#ifdef RTL8192SU

int usb_init_sw(struct mac80211_shared_priv *priv);
int usb_stop_sw(struct mac80211_shared_priv *priv);
	
#endif

int rtl8190_open (struct ieee80211_hw *hw);
void rtl8190_close(struct ieee80211_hw *hw);

static int rtl80211_config(struct ieee80211_hw *hw, u32 changed);
static void rtl80211_bss_info_changed(struct ieee80211_hw *hw,
				     struct ieee80211_vif *vif,
				     struct ieee80211_bss_conf *info,
				     u32 changed);

/* kernel 2.6.31
static u64 rtl80211_prepare_multicast(struct ieee80211_hw *hw, int mc_count,
				     struct dev_addr_list *mc_list); */
				     
static void rtl80211_configure_filter(struct ieee80211_hw *hw,
				     unsigned int changed_flags,
				     unsigned int *total_flags,
				     // u64 multicast); //kernel 2.6.31
					 int mc_count, struct dev_addr_list *mclist);
					 
//int rtl8190_set_hwaddr(struct net_device *dev, void *addr)
int rtl819x_set_hwaddr(struct ieee80211_hw *hw, void *addr)
{
	unsigned long flags;
	struct mac80211_shared_priv *priv = (struct mac80211_shared_priv *)hw->priv;
	unsigned long ioaddr = priv->pshare_hw->ioaddr;
	unsigned long reg;
//	unsigned char *p;
#ifdef WDS
	int i;
#endif

//	p = (unsigned char *)addr;
	SAVE_INT_AND_CLI(flags);

	memcpy(priv->hwaddr, (unsigned char *)addr, ETH_ALEN);
	//memcpy(GET_MY_HWADDR, p, ETH_ALEN);

#if 0
#ifdef MBSSID
#ifdef RTL8192SE
	if (GET_ROOT(priv)->mib->miscEntry.vap_enable)
#endif
	{
		if (IS_VAP_INTERFACE(priv)) {
			RESTORE_INT(flags);
			return 0;
		}
	}
#endif

#ifdef WDS
	for (i=0; i<NUM_WDS; i++)
		if (priv->wds_dev[i])
			memcpy(priv->wds_dev[i]->dev_addr, p, 6);
#endif
#ifdef CONFIG_RTK_MESH
	if(NUM_MESH>0)
		if (priv->mesh_dev)
			memcpy(priv->mesh_dev->dev_addr, p, 6);
#endif

#ifdef UNIVERSAL_REPEATER
	if (IS_ROOT_INTERFACE(priv)) {
		if (GET_VXD_PRIV(priv)) {
			memcpy(GET_VXD_PRIV(priv)->dev->dev_addr, p, 6);
			memcpy(GET_VXD_PRIV(priv)->mib->dot11OperationEntry.hwaddr, p, 6);
		}
	}
	else if (IS_VXD_INTERFACE(priv)) {
		memcpy(GET_ROOT_PRIV(priv)->dev->dev_addr, p, 6);
		memcpy(GET_ROOT_PRIV(priv)->mib->dot11OperationEntry.hwaddr, p, 6);
	}
#endif

#if defined(RTL8190)
	reg = *(unsigned long *)(dev->dev_addr);
	RTL_W32(_IDR0_, (cpu_to_le32(reg)));
	reg = *(unsigned long *)((unsigned long)dev->dev_addr + 4);
	RTL_W32(_IDR0_ + 4, (cpu_to_le32(reg)));
#elif defined(RTL8192SE)
	reg = *(unsigned long *)(dev->dev_addr);
	RTL_W32(IDR0, (cpu_to_le32(reg)));
	reg = *(unsigned long *)((unsigned long)dev->dev_addr + 4);
	RTL_W32(IDR4, (cpu_to_le32(reg)));
#endif
#ifdef MBSSID
#ifdef RTL8192SE
	if (GET_ROOT(priv)->mib->miscEntry.vap_enable)
#endif
	{
		if (OPMODE & WIFI_AP_STATE)
			rtl8190_init_mbssid(priv);
	}
#endif
#endif  //#if 0

	RESTORE_INT(flags);

#ifdef CLIENT_MODE
	if (!(OPMODE & WIFI_AP_STATE) && netif_running(priv->dev)) {
		int link_status = chklink_wkstaQ(priv);
		if (link_status)
			start_clnt_join(priv);
	}
#endif

	return 0;
}

void force_stop_wlan_hw(struct mac80211_shared_priv *priv)
{

	extern	void phy_InitBBRFRegisterDefinition(struct mac80211_shared_priv *priv);

	phy_InitBBRFRegisterDefinition(priv);
	rtl819x_stop_hw(priv, 1);
	rtl819x_stop_hw(priv, 0);

}

static int rtl819x_init_hw(struct mac80211_shared_priv *priv)
{

#ifdef RTL8192SE
	rtl819x_init_hw_PCI(priv);
#endif

#ifdef RTL8192SU
	rtl819x_init_hw_USB(priv);
#endif

	return 0;
}


static int mac80211_init_sw(struct mac80211_shared_priv *priv)
{
        int rc;
	priv->raupdate_intv = 3000; // 3 secs
#ifdef RTL8192SE
	rc = pci_init_sw(priv);
#endif

#ifdef RTL8192SU
	usb_init_sw(priv);
#endif

	return rc;

}

static int rtl8190_stop_sw(struct mac80211_shared_priv *priv)
{

#ifdef RTL8192SE
	pci_stop_sw(priv);
#endif

#ifdef RTL8192SU
	usb_stop_sw(priv);
#endif

	return 0;
	
}

static int rtl80211_add_interface(struct ieee80211_hw *hw,
				 struct ieee80211_if_init_conf *conf)
{
	struct mac80211_shared_priv *priv = hw->priv;
	unsigned long	ioaddr = priv->pshare_hw->ioaddr;
	unsigned long reg = 0;

	if (priv->opmode != NL80211_IFTYPE_MONITOR)
		return -EOPNOTSUPP;

	switch (conf->type) {
	case NL80211_IFTYPE_STATION:
		printk("oops: sta \n ");
		RTL_W16(BCNTCFG, 0x0204);
		RTL_W8(MSR, MSR_INFRA);
		priv->opmode = conf->type;
		break;
	case NL80211_IFTYPE_AP:
		printk("oops: AP \n ");
		memcpy(priv->bssid, priv->hwaddr, MACADDRLEN);
		RTL_W16(BCNTCFG, 0x000a);
		RTL_W8(MSR, MSR_AP);
		priv->opmode = conf->type;
		break;
	case NL80211_IFTYPE_ADHOC:
		DEBUG_INFO("Adhoc-mode enabled...\n");
		//RTL_W16(BCNTCFG, 0x000a);  brian reference AP 
		RTL_W8(MSR, MSR_ADHOC);
	default:
		return -EOPNOTSUPP;
	}
				

	priv->vif = conf->vif;

	reg = *(unsigned long *)(conf->mac_addr);
	RTL_W32(IDR0, (cpu_to_le32(reg)));
	reg = *(unsigned long *)((unsigned long)conf->mac_addr + 4);
	RTL_W32(IDR4, (cpu_to_le32(reg)));
	

	printk("add_interface done!! 0x%x 0x%x\n", RTL_R8(IDR4), RTL_R8(IDR4+1));

	return 0;
}

static void rtl80211_remove_interface(struct ieee80211_hw *hw,
				     struct ieee80211_if_init_conf *conf)
{
	struct mac80211_shared_priv *priv = hw->priv;
	priv->opmode = NL80211_IFTYPE_MONITOR;
	priv->vif = NULL;
}

/* kernel 2.6.31
static u64 rtl80211_prepare_multicast(struct ieee80211_hw *hw, int mc_count,
				     struct dev_addr_list *mc_list)
{
	return mc_count;
}
*/


static int rtl80211_config(struct ieee80211_hw *hw, u32 changed)
{
	enum nl80211_channel_type ch_type;
	struct mac80211_shared_priv *priv = hw->priv;
	struct ieee80211_conf *conf = &hw->conf;

	/*
	printk("conf->flags >> 0x%x!!\n", conf->flags);
	printk("changed >> 0x%x!!\n", changed);
	*/

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30))	
      ch_type = conf->ht.channel_type;	
#else
	ch_type = conf->channel_type;	
#endif	
	
	//kernel 2.6.31
	//if (changed & IEEE80211_CONF_CHANGE_IDLE) {
		//printk("change_idle\n");
	//}

	if (changed & IEEE80211_CONF_CHANGE_PS) {
		//printk("change_ps\n");
	}

	if (changed & IEEE80211_CONF_CHANGE_CHANNEL) {
		int channel = ieee80211_frequency_to_channel(conf->channel->center_freq);
		//printk("change_channel\n");
		

		if (ch_type== NL80211_CHAN_HT40MINUS) {
			priv->pshare_hw->is_40m_bw = 1;
			priv->pshare_hw->offset_2nd_chan = HT_2NDCH_OFFSET_BELOW;
		} else if (ch_type == NL80211_CHAN_HT40PLUS) {
			priv->pshare_hw->is_40m_bw = 1;
			priv->pshare_hw->offset_2nd_chan = HT_2NDCH_OFFSET_ABOVE;
		} else {
			priv->pshare_hw->is_40m_bw = 0;
			priv->pshare_hw->offset_2nd_chan = HT_2NDCH_OFFSET_DONTCARE;
		}
		
		//printk("ch=%d, bw=%d, 2nd_ch=%d\n", channel, priv->pshare_hw->is_40m_bw, priv->pshare_hw->offset_2nd_chan);
		priv->pshare_hw->CurrentChannelBW = priv->pshare_hw->is_40m_bw;
		SwBWMode(priv, priv->pshare_hw->CurrentChannelBW, priv->pshare_hw->offset_2nd_chan);
		SwChnl(priv, (unsigned char) channel, priv->pshare_hw->offset_2nd_chan);
	}

	if (changed & IEEE80211_CONF_CHANGE_POWER) {
		//printk("change_pwr\n");
	}

	if (changed & IEEE80211_CONF_CHANGE_BEACON_INTERVAL) {
		set_fw_reg(priv, (0xF1000000 | (conf->beacon_int << 8)), 0, 0);
		priv->beacon_interval = conf->beacon_int;
	}

	
	return 0;
}



static void rtl80211_bss_info_changed(struct ieee80211_hw *hw,
				     struct ieee80211_vif *vif,
				     struct ieee80211_bss_conf *info,
				     u32 changed)
{
	struct mac80211_shared_priv *priv = hw->priv;
	//unsigned long ioaddr = priv->pshare_hw->ioaddr;
	//unsigned long reg;

	printk("rtl80211_bss_info_changed>> 0x%08x!!\n", changed);
	
	/* kernel 2.6.31
	if (changed & BSS_CHANGED_BSSID &&
		!is_zero_ether_addr(info->bssid)) {

		printk("bssid_changed\n");
		memcpy(priv->bssid, info->bssid, MACADDRLEN);
		
		reg = *(unsigned long *)(info->bssid);
		RTL_W32(BSSIDR, cpu_to_le32(reg));
		reg = *(unsigned long *)(info->bssid + 4);
		RTL_W16((BSSIDR + 4), cpu_to_le16(reg));

	}
	*/

	if(changed & BSS_CHANGED_ASSOC)
	{
		printk("bssid changed !\n");
	}

 	if (changed & BSS_CHANGED_ERP_CTS_PROT){
		if (info->use_cts_prot)
			priv->cfg_flags |= SC_OP_PROTECT_ENABLE;
		else
			priv->cfg_flags &= ~SC_OP_PROTECT_ENABLE;
	}
 	if (changed & BSS_CHANGED_ERP_PREAMBLE){
		if (info->use_short_preamble)
			priv->cfg_flags |= SC_OP_PREAMBLE_SHORT;
		else
			priv->cfg_flags &= ~SC_OP_PREAMBLE_SHORT;
	}
 	if (changed & BSS_CHANGED_ERP_SLOT){
	}

	if (changed & BSS_CHANGED_BASIC_RATES){
	}

	/* kernel 2.6.31
	if (changed & BSS_CHANGED_BEACON_INT) {
		set_fw_reg(priv, (0xF1000000 | (info->beacon_int << 8)), 0, 0);
		priv->beacon_interval = info->beacon_int;
	}
	
	if ((changed & BSS_CHANGED_BEACON) ||
		(changed & BSS_CHANGED_BEACON_ENABLED &&
			info->enable_beacon)) {
		if (priv->opmode == NL80211_IFTYPE_AP){
			mac80211_send_beacon(priv);
		}
	}
	*/
}

//for kernel 2.6.30
static int rtl80211_config_interface(struct ieee80211_hw *hw,
				  struct ieee80211_vif *vif,
				  struct ieee80211_if_conf *conf)
{
	struct mac80211_shared_priv *priv = hw->priv;
	unsigned long ioaddr = priv->pshare_hw->ioaddr;
	unsigned long reg;

	
	if ((conf->changed & IEEE80211_IFCC_BSSID) &&
	    !is_zero_ether_addr(conf->bssid)) {
		printk("bssid_changed\n");
		memcpy(priv->bssid, conf->bssid, MACADDRLEN);
		
		reg = *(unsigned long *)(conf->bssid);
		RTL_W32(BSSIDR, cpu_to_le32(reg));
		reg = *(unsigned long *)(conf->bssid + 4);
		RTL_W16((BSSIDR + 4), cpu_to_le16(reg));
		
		
		if (vif->type == NL80211_IFTYPE_STATION)
		{
			printk("bssid:%pM\n", conf->bssid);
			RTL_W8(MSR, MSR_INFRA);	
			RTL_W32(RCR, RTL_R32(RCR)|RCR_CBSSID);				
		}		

		
		RTL_W8(0x364, RTL_R8(0x364) | FW_REG364_DIG | FW_REG364_HP | FW_REG364_RSSI);

	}

	if (vif->type == NL80211_IFTYPE_AP) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30))
		if (conf->changed & IEEE80211_IFCC_BEACON)
#else
		if ((conf->changed & IEEE80211_IFCC_BEACON) ||
			(conf->changed & IEEE80211_IFCC_BEACON &&
			 conf->enable_beacon)) 
#endif			 
		{
			priv->beacon_interval = hw->conf.beacon_int ? hw->conf.beacon_int : DEFAULT_BINTVAL;
			set_fw_reg(priv, (0xF1000000 | (priv->beacon_interval << 8)), 0, 0);
			
			mac80211_send_beacon(priv);
		}
		
		RTL_W8(0x364, RTL_R8(0x364) | FW_REG364_DIG | FW_REG364_HP | FW_REG364_RSSI);
	}
	
	return 0;
}

static void rtl80211_configure_filter(struct ieee80211_hw *hw,
				     unsigned int changed_flags,
				     unsigned int *total_flags,
//				     u64 multicast) //kernel 2.6.31
				     int mc_count, struct dev_addr_list *mclist)
{
	struct mac80211_shared_priv *priv = hw->priv;
	unsigned long ioaddr = priv->pshare_hw->ioaddr;
#ifdef RTL8192SU
	unsigned int rcr = priv->pshare_hw->rcr_reg;
#endif

	printk("rtl80211_configure_filter(), changed_flags=0x%x, total_flags=0x%x\n", changed_flags, *total_flags);

	changed_flags &= SUPPORTED_FILTERS;
	*total_flags &= SUPPORTED_FILTERS;

	priv->rxfilter = *total_flags;
	
//2 ToDo: Need further check current status and opmode

#ifdef RTL8192SU

	if (*total_flags & FIF_PROMISC_IN_BSS)
	{
		rcr |= RCR_ADD3;
		RTL_W32_ASYNC(RCR, rcr);		
	}	
	if (*total_flags & FIF_ALLMULTI)
	{
		rcr |= RCR_AM;
		RTL_W32_ASYNC(RCR, rcr);
	}	
	if (*total_flags & FIF_CONTROL)
	{
		rcr |= RCR_ACF;
		RTL_W32_ASYNC(RCR, rcr);
	}	
	if (*total_flags & FIF_OTHER_BSS)
	{
		rcr |= RCR_AAP;
		RTL_W32_ASYNC(RCR, rcr);
	}	
	if (*total_flags & FIF_FCSFAIL)
	{
		rcr |= RCR_ACRC32;
		RTL_W32_ASYNC(RCR, rcr);
	}	

	priv->pshare_hw->rcr_reg = rcr;

#else

	if (*total_flags & FIF_PROMISC_IN_BSS)
		RTL_W32_ASYNC(RCR, RTL_R32(RCR) | RCR_ADD3);
	if (*total_flags & FIF_ALLMULTI)
		RTL_W32_ASYNC(RCR, RTL_R32(RCR) | RCR_AM);
	if (*total_flags & FIF_CONTROL)
		RTL_W32_ASYNC(RCR, RTL_R32(RCR) | RCR_ACF);
	if (*total_flags & FIF_OTHER_BSS)
		RTL_W32_ASYNC(RCR, RTL_R32(RCR) | RCR_AAP);
	if (*total_flags & FIF_FCSFAIL)
		RTL_W32_ASYNC(RCR, RTL_R32(RCR) | RCR_ACRC32);

#endif
	
	if(changed_flags & FIF_BCN_PRBRESP_PROMISC)
	{
		if(*total_flags & FIF_BCN_PRBRESP_PROMISC)
		{
			printk("change to MSR_NOLINK\n");
			RTL_W8_ASYNC(MSR, MSR_NOLINK);
		}	
		else
		{
			switch (priv->opmode) {
				case NL80211_IFTYPE_ADHOC:
					RTL_W8_ASYNC(MSR, MSR_ADHOC);
					break;
				case NL80211_IFTYPE_STATION:
					printk("change to MSR_INFRA\n");
					RTL_W8_ASYNC(MSR, MSR_INFRA);			
					break;
				case NL80211_IFTYPE_AP:		
					RTL_W8_ASYNC(MSR, MSR_AP);		
					break;
				default:
					RTL_W8_ASYNC(MSR, MSR_NOLINK);
			}
		}	

	}
	
}

//static void MDL_DEVINIT set_mib_default(struct mac80211_shared_priv *priv)
static void set_mib_default(struct mac80211_shared_priv *priv)
{
	unsigned char *p;
	unsigned char tmpbuf[10];
	p = (unsigned char *)tmpbuf;

        if( !priv->pmib ){
                printk("priv->mib is NULL pointer\n");
                return ;
        }
	priv->pmib->mib_version = MIB_VERSION;
	set_mib_default_tbl(priv);

	// others that are not types of byte and int
	strcpy(priv->pmib->dot11StationConfigEntry.dot11DesiredSSID, "close_test");
	priv->pmib->dot11StationConfigEntry.dot11DesiredSSIDLen = strlen("close_test");
	//memcpy(p, "\x00\xa9\x87x65\x43\x21", MACADDRLEN);
	//rtl819x_set_hwaddr(priv->hw, (void *)tmpbuf);

#if 0
//#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))  // is root interface
#endif
	{
#ifdef RTL8190
		// david, read EEPROM value as default if EEP existed -----------
		unsigned char data;
		ReadAdapterInfo(priv, EEPROM_RF_CHIP_ID, (void *)&data);
		if (!priv->EE_AutoloadFail) {
			PRINT_INFO("Load EEPROM value into MIB\n");

			if (priv->EE_RFTypeID == 9)
				priv->pmib->dot11RFEntry.dot11RFType = 7;
			else if (priv->EE_RFTypeID == 10)
				priv->pmib->dot11RFEntry.dot11RFType = 8;
			else {
				DEBUG_ERR("Invalid RF type, set Zebra as default\n");
				priv->pmib->dot11RFEntry.dot11RFType = 7;
			}

			memcpy(p, priv->EE_Mac, MACADDRLEN);
			rtl819x_set_hwaddr(priv->hw, (void *)p);
			memcpy(priv->pmib->dot11RFEntry.pwrlevelCCK, priv->EE_TxPower_CCK, MAX_CCK_CHANNEL_NUM);
			memcpy(priv->pmib->dot11RFEntry.pwrlevelOFDM, priv->EE_TxPower_OFDM, MAX_OFDM_CHANNEL_NUM);
		}
#endif

#ifdef DFS
		init_timer(&priv->ch52_timer);
		priv->ch52_timer.data = (unsigned long) priv;
		priv->ch52_timer.function = rtl8190_ch52_timer;

		init_timer(&priv->ch56_timer);
		priv->ch56_timer.data = (unsigned long) priv;
		priv->ch56_timer.function = rtl8190_ch56_timer;

		init_timer(&priv->ch60_timer);
		priv->ch60_timer.data = (unsigned long) priv;
		priv->ch60_timer.function = rtl8190_ch60_timer;

		init_timer(&priv->ch64_timer);
		priv->ch64_timer.data = (unsigned long) priv;
		priv->ch64_timer.function = rtl8190_ch64_timer;
#endif

		if (((priv->pshare_hw->type>>TYPE_SHIFT) & TYPE_MASK) == TYPE_EMBEDDED) {
			// not implement yet
		}
		else {
			// can't read correct h/w version here
			//GetHardwareVersion(priv);
		}
	}
}

static int rtl8190_open(struct ieee80211_hw *hw)
{
	struct mac80211_shared_priv *priv = hw->priv;
	int rc;
	unsigned long ioaddr = priv->pshare_hw->ioaddr;
	//unsigned long reg;
	//unsigned long x;
	
	int init_hw_cnt = 0;

	printk("8190_open\n");
	
	// stop h/w in the very beginning
	rtl819x_stop_hw(priv, 0);
	
	printk("rtl819x_stop_hw\n");

	rc = mac80211_init_sw(priv);
	
	printk("mac80211_init_sw\n");
	
    if (rc) {
		printk(KERN_ERR "%s: failed to init sw\n",
		       wiphy_name(hw->wiphy));
		return rc;
    }	


do_hw_init:	
	
	priv->opmode = NL80211_IFTYPE_MONITOR;

	rc = rtl819x_init_hw(priv);
	
	printk("rtl819x_init_hw\n");

	/*
	memcpy(&reg, MY_MACADDR, 4);
	RTL_W32(IDR0, (cpu_to_le32(reg)));
	memcpy(&reg, MY_MACADDR+4, 4);
	RTL_W32(IDR4, (cpu_to_le32(reg)));
	*/
	
	//spin_unlock_irqrestore(&priv->pshare_hw->lock2, x);

	//printk("ioaddr %lx IDR 0x%x 0x%x\n", priv->pshare_hw->ioaddr, RTL_R8(IDR4), RTL_R8(IDR4+1));

	if (rc && ++init_hw_cnt < 5) {
		goto do_hw_init;			
	}

	if (rc) {
			printk(KERN_ERR "init hw failed!\n");
			//printk("init hw failed!\n");

			force_stop_wlan_hw(priv);		
			local_irq_disable();

			return rc;
	}
	
	priv->drv_state |= DRV_STATE_OPEN;      // set driver as has been opened, david

	//RTL_W32(_RCR_, RTL_R32(_RCR_) | _ADD3_);
	//rtl8190_proc_init((void*)priv);

	return 0;
}

static void rtl8190_close(struct ieee80211_hw *hw)
{
    struct mac80211_shared_priv *priv = hw->priv;

	DBFENTER;

	if (!(priv->drv_state & DRV_STATE_OPEN)) {
		DBFEXIT;
		return;
	}

	priv->drv_state &= ~DRV_STATE_OPEN;     // set driver as has been closed, david
	
	priv->opmode = NL80211_IFTYPE_UNSPECIFIED;
	
//	free_skb_queue(priv, &priv->pshare_hw->skb_queue);
	
	rtl819x_stop_hw(priv, 1);

	rtl8190_stop_sw(priv);

#ifdef ENABLE_RTL_SKB_STATS
	DEBUG_INFO("skb_tx_cnt =%d\n", rtl_atomic_read(&priv->rtl_tx_skb_cnt));
	DEBUG_INFO("skb_rx_cnt =%d\n", rtl_atomic_read(&priv->rtl_rx_skb_cnt));
#endif

	//rtl8190_proc_remove(priv);

	DBFEXIT;

       return;
	
}

static void rtl80211_sta_notify(struct ieee80211_hw *hw,
			     struct ieee80211_vif *vif,
			     enum sta_notify_cmd cmd,
			     struct ieee80211_sta *sta)
{
	struct mac80211_shared_priv *priv = hw->priv;

	switch (cmd) {
	case STA_NOTIFY_ADD:
		//printk("add sta\n");
		rtl80211_addRATid(priv, sta);
		break;
	case STA_NOTIFY_REMOVE:
		//printk("remove sta\n");
		rtl80211_delRATid(priv, sta);
		break;
	default:
		break;
	}
}

static int rtl80211_ampdu_action(struct ieee80211_hw *hw,
			      enum ieee80211_ampdu_mlme_action action,
			      struct ieee80211_sta *sta,
			      u16 tid, u16 *ssn)
{
	struct mac80211_shared_priv *priv = hw->priv;
	int ret = 0;

	switch (action) {
	case IEEE80211_AMPDU_RX_START:
		break;
	case IEEE80211_AMPDU_RX_STOP:
		break;
	case IEEE80211_AMPDU_TX_START:
		ret = rtl_tx_aggr_start(priv, sta, tid, ssn);
		if(ret < 0)
			printk("Unable to start TX aggregation\n");
		else
	        ieee80211_start_tx_ba_cb_irqsafe(hw, sta->addr, tid);
		break;
	case IEEE80211_AMPDU_TX_STOP:
		ret = rtl_tx_aggr_stop(priv, sta, tid);
		if (ret < 0)
			printk("Unable to stop TX aggregation\n");
		else
			ieee80211_stop_tx_ba_cb_irqsafe(hw, sta->addr, tid);
		break;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	case IEEE80211_AMPDU_TX_OPERATIONAL:
		rtl_tx_aggr_resume(priv, sta, tid);
		break;
#endif		
	default:
		DEBUG_ERR("Unknown AMPDU action\n");
	}

	return ret;
}

static int rtl80211_cmd(struct ieee80211_hw *hw)
{
        
}

struct ieee80211_ops rtl819x_ops = {
	.tx					= rtl80211_tx,
	.start 				= rtl8190_open,
	.stop 				= rtl8190_close,
	.add_interface		= rtl80211_add_interface,
	.remove_interface	= rtl80211_remove_interface,
	.config				= rtl80211_config,
	.config_interface   = rtl80211_config_interface,
	.bss_info_changed	= rtl80211_bss_info_changed,
	//.prepare_multicast	= rtl80211_prepare_multicast, //kernel 2.6.31
	.configure_filter	= rtl80211_configure_filter,
	.sta_notify         = rtl80211_sta_notify,
	.ampdu_action		= rtl80211_ampdu_action,
//	.testmode_cmd          = rtl80211_cmd,
};



void rtl_set_hw_capab(struct mac80211_shared_priv *priv, struct ieee80211_hw *hw)
{
	memcpy(priv->channels, rtl819x_channels, sizeof(rtl819x_channels));
	memcpy(priv->rates[RTL_MODE_11G], rtl819x_11g_ratetbl.rates, sizeof(rtl819x_11g_ratetbl.rates));
	//memcpy(priv->rates[RTL_MODE_11N_HT20], rtl819x_11gn_ratetbl.rates, sizeof(rtl819x_11gn_ratetbl.rates));

	priv->band.band = IEEE80211_BAND_2GHZ;
	priv->band.channels = priv->channels;
	priv->band.n_channels = ARRAY_SIZE(rtl819x_channels);
	priv->band.bitrates = priv->rates[RTL_MODE_11G];
	//priv->band.bitrates = priv->rates[RTL_MODE_11N_HT20];
	priv->band.n_bitrates = rtl819x_11g_ratetbl.rate_cnt;
	//priv->band.n_bitrates = rtl819x_11gn_ratetbl.rate_cnt;
	
	// following is HT capability
	priv->band.ht_cap.ht_supported = true;
	priv->band.ht_cap.cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 |
							IEEE80211_HT_CAP_SM_PS |
							IEEE80211_HT_CAP_SGI_40 |
							IEEE80211_HT_CAP_SGI_20 |
							IEEE80211_HT_CAP_MAX_AMSDU |
							IEEE80211_HT_CAP_DSSSCCK40;
	//priv->band.ht_cap.ampdu_factor = _HTCAP_AMPDU_FAC_32K_;
	priv->band.ht_cap.ampdu_factor = _HTCAP_AMPDU_FAC_64K_;
	priv->band.ht_cap.ampdu_density = _HTCAP_AMPDU_SPC_16_US_;
	// set up supported mcs set
	memset(&priv->band.ht_cap.mcs, 0, sizeof(priv->band.ht_cap.mcs));
	priv->band.ht_cap.mcs.rx_mask[0] = 0xff;
	priv->band.ht_cap.mcs.rx_mask[1] = 0xff;
	priv->band.ht_cap.mcs.rx_mask[2] = 0xff;
	priv->band.ht_cap.mcs.rx_mask[3] = 0xff;
	priv->band.ht_cap.mcs.tx_params = IEEE80211_HT_MCS_TX_DEFINED;


	hw->wiphy->bands[IEEE80211_BAND_2GHZ] = &priv->band;
	hw->flags = IEEE80211_HW_HOST_BROADCAST_PS_BUFFERING |
		     	IEEE80211_HW_RX_INCLUDES_FCS |
				IEEE80211_HW_AMPDU_AGGREGATION |
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))				
				IEEE80211_HW_SUPPORTS_PS |
#endif				
		     	IEEE80211_HW_SIGNAL_DBM |
		     	IEEE80211_HW_NOISE_DBM;
	hw->wiphy->interface_modes = 
				BIT(NL80211_IFTYPE_AP) |
				BIT(NL80211_IFTYPE_STATION);
	hw->queues = 4;
	hw->max_rates = 4;
	hw->channel_change_time = 5000;
	hw->max_listen_interval = 10;
	hw->max_rate_tries = 4;
	hw->sta_data_size = sizeof(struct priv_stainfo);
	hw->vif_data_size = sizeof(struct priv_vifinfo);

	hw->rate_control_algorithm = "rtl_rate_control";
}


#ifdef RTL8192SU
int usb_init_sw(struct mac80211_shared_priv *priv)
{
	struct priv_shared_hw *pshared_hw = (struct priv_shared_hw *)priv->pshare_hw;
	//struct rtl8190_hw *phw = GET_HW(priv);

	tasklet_init(&pshared_hw->rx_tasklet, rtl80211_rx_tasklet, (unsigned long)priv);

	//
	pshared_hw->curr_band = BAND_2G;
	
	pshared_hw->working_channel = 1;

	pshared_hw->is_40m_bw = HT_CHANNEL_WIDTH_20;
	pshared_hw->CurrentChannelBW = HT_CHANNEL_WIDTH_20;

	pshared_hw->offset_2nd_chan = HT_2NDCH_OFFSET_DONTCARE;

	



	//skb_queue_head_init(&priv->pshare_hw->skb_queue);


	//
	priv->pshare_hw->rf_ft_var.raGoDownUpper=50;
	priv->pshare_hw->rf_ft_var.raGoUpUpper=55;
	priv->pshare_hw->rf_ft_var.raGoDown20MLower=18;
	priv->pshare_hw->rf_ft_var.raGoUp20MLower=23;
	priv->pshare_hw->rf_ft_var.raGoDown40MLower=15;
	priv->pshare_hw->rf_ft_var.raGoUp40MLower=20;		
	

	rtl8192s_usb_recv(priv);

	return 0;
}
int usb_stop_sw(struct mac80211_shared_priv *priv)
{
	struct priv_shared_hw *pshared_hw = (struct priv_shared_hw *)priv->pshare_hw;
	struct rtl8190_hw *phw = pshared_hw->phw;

	usb_kill_anchored_urbs(&phw->anchored);
	
	tasklet_kill(&pshared_hw->rx_tasklet);
	
	return 0;
}
static int MDL_DEVINIT rtl8192s_usb_probe(struct usb_interface *intf,
				   const struct usb_device_id *id)
{
	int rc;
	struct ieee80211_hw *hw;
	struct mac80211_shared_priv *priv;
	struct rtl8190_hw		*phw;
	struct rtl8190_tx_desc_info		*ptxdesc;
	struct priv_shared_hw *pshare_hw;	// david	
	struct usb_device *udev = interface_to_usbdev(intf);
	unsigned char mac[ETH_ALEN]={0x00, 0xe0, 0x4c, 0x87, 0x11, 0x22};
	

	priv = NULL;
	phw = NULL;
	ptxdesc = NULL;
	pshare_hw = NULL;
	

	phw = &hw_info;
	memset((void *)phw, 0, sizeof(struct rtl8190_hw));
	ptxdesc = &phw->tx_info;
	pshare_hw = &shared_hw;
	memset((void *)pshare_hw, 0, sizeof(struct priv_shared_hw));

	
	hw = ieee80211_alloc_hw(sizeof(struct mac80211_shared_priv), &rtl819x_ops);
	if (!hw) {
		printk(KERN_ERR "ieee80211_alloc_hw() error!\n");
		rc = -ENOMEM;
		goto err_out_res;
	}

	priv = hw->priv;
	priv->hw = hw;

	priv->pshare_hw = pshare_hw;
	priv->pshare_hw->phw = phw;
	priv->pshare_hw->pdesc_info = ptxdesc;
	priv->pshare_hw->udev = udev;
	phw->udev = udev;
	phw->pusb_intf = intf;
	
	
	SET_IEEE80211_DEV(hw, &intf->dev);
	usb_set_intfdata(intf, hw);	
	usb_get_dev(udev);


	usb_configure(pshare_hw);
	
	phw->bChangeBBInProgress = _FALSE;
	phw->bChangeRFInProgress = _FALSE;
	mutex_init(&phw->io_mutex);
	
	//init_usb_anchor(&phw->anchored);
	skb_queue_head_init(&phw->rx_queue);
	skb_queue_head_init(&phw->rx_skb_queue);

	start_work_thread(priv);

	

	spin_lock_init(&priv->pshare_hw->lock);
	spin_lock_init(&priv->rxlock);


	//todo: get hw mac addr from efuse/eeprom
	memcpy(priv->hwaddr, mac, ETH_ALEN);

	SET_IEEE80211_PERM_ADDR(hw, priv->hwaddr);

	rtl_set_hw_capab(priv, hw);

	hw->extra_tx_headroom = sizeof(struct rtl8192s_txdesc);//!!!


	rc = ieee80211_register_hw(hw);
	if (rc) {
		printk(KERN_ERR "rtl8192su: %d Cannot register device\n", rc);
		goto err_out_free;
	}

	printk(KERN_INFO "%s: hwaddr %pM\n",
		wiphy_name(hw->wiphy), hw->wiphy->perm_addr);
	

	return 0;
	
err_out_free:
	
	usb_set_intfdata(intf, NULL);
	
	usb_put_dev(udev);
	
	ieee80211_free_hw(hw);

err_out_res:
	
	return rc;
	
}

static void MDL_DEVEXIT rtl8192s_usb_disconnect(struct usb_interface *intf)
{
	struct ieee80211_hw *hw = usb_get_intfdata(intf);
	 struct mac80211_shared_priv *priv;

	printk("%s\n", __FUNCTION__);

	if (!hw) 
	{
        	BUG();
		return;
	}
	
	priv = hw->priv;



	RTL_W8(RF_CTRL, 0x00);

	// Turn off BB	
	//write8(priv, CR+1, 0x07);
	mdelay(5);

	// Turn off MAC	
	//write8(priv, SYS_CLKR+1, 0x78); // Switch Control Path
	RTL_W8(SYS_CLKR+1, 0x38); // Switch Control Path	
	RTL_W8(SYS_FUNC_EN+1, 0x70); // Reset MACTOP, IOREG, 4181
	RTL_W8(PMC_FSM, 0x06);  // Enable Loader Data Keep
	RTL_W8(SYS_ISO_CTRL, 0xF9); // Isolation signals from CORE, PLL
	RTL_W8(SYS_ISO_CTRL+1, 0xe8); // Enable EFUSE 1.2V(LDO) 
	//write8(priv, SYS_ISO_CTRL+1, 0x69); // Isolation signals from Loader
	RTL_W8(AFE_PLL_CTRL, 0x00); // Disable AFE PLL.
	RTL_W8(LDOA15_CTRL, 0x54);  // Disable A15V
	RTL_W8(SYS_FUNC_EN+1, 0x50); // Disable E-Fuse 1.2V

	//write8(priv, SPS1_CTRL, 0x64); // Disable LD12 & SW12 (for IT)
	RTL_W8(LDOV12D_CTRL, 0x24); // Disable LDO12(for CE)
	
	RTL_W8(AFE_MISC, 0x30); // Disable AFE BG&MB


	
	// <Roger_Notes> The following  options are alternative.
	// Disable 1.6V LDO or  1.8V Switch. 2008.09.26.

	// Option for Disable 1.6V LDO.	
	RTL_W8(SPS0_CTRL, 0x56); // Disable 1.6V LDO
	RTL_W8(SPS0_CTRL+1, 0x43);  // Set SW PFM 

	// Option for Disable 1.8V Switch.	
	//write8(priv, SPS0_CTRL, 0x55); // Disable SW 1.8V


	
	
	ieee80211_unregister_hw(hw);

	stop_work_thread(priv);	
	
	usb_reset_device(interface_to_usbdev(intf));
	usb_put_dev(interface_to_usbdev(intf));
	
	ieee80211_free_hw(hw);

	
}

static struct usb_device_id MDL_DEVINITDATA rtl8192s_usb_table[] = {

	/* Realtek 8192s */
	{USB_DEVICE(0x0bda, 0x8188), .driver_info = DEVICE_RTL8192S},
	{USB_DEVICE(0x0bda, 0x8171), .driver_info = DEVICE_RTL8192S},
	{USB_DEVICE(0x0bda, 0x8172), .driver_info = DEVICE_RTL8192S},
	{USB_DEVICE(0x0bda, 0x8173), .driver_info = DEVICE_RTL8192S},
	{USB_DEVICE(0x0bda, 0x8174), .driver_info = DEVICE_RTL8192S},
	{USB_DEVICE(0x0bda, 0x8712), .driver_info = DEVICE_RTL8712},
	{}
};

//MODULE_DEVICE_TABLE(usb, rtl8192s_usb_table);

static struct usb_driver rtl8192su_driver = {
	.name		= KBUILD_MODNAME,
	.id_table	= rtl8192s_usb_table,
	.probe		= rtl8192s_usb_probe,
	.disconnect	= __devexit_p(rtl8192s_usb_disconnect),
};

int MDL_INIT rtl8190_init(void)
{	
	printk(KERN_INFO "rtl8192su: Driver loaded\n" );
	return usb_register(&rtl8192su_driver);
}

static void MDL_EXIT rtl8190_exit (void)
{
	printk(KERN_INFO "rtl8192su: Driver unloaded\n" );
	usb_deregister (&rtl8192su_driver);
}
#endif

#ifdef RTL8192SE

irqreturn_t rtl8190_interrupt(int irq, void *dev_instance)
{
	int ret;
	//printk("int");
	ret = __rtl8192SE_interrupt(dev_instance);
	
	if (ret == FAIL)
		return IRQ_RETVAL(IRQ_NONE);
	else
		return IRQ_RETVAL(IRQ_HANDLED);
}
static __inline__ int __rtl8192SE_interrupt(void *dev_instance)
{
	// Use static variables for stack size overflow issue, david+2007-09-19
	static struct ieee80211_hw *hw = NULL;
	static struct mac80211_shared_priv *priv = NULL;
	static struct rtl8190_hw *phw = NULL;
	static unsigned long ioaddr = 0;

	static unsigned int status, status_ext;
	static unsigned int caseBcnInt, caseBcnStatusOK, caseBcnStatusER;
	static unsigned int caseRxStatus, caseRxRDU;
	static unsigned int caseRxCmd, caseTimer1, caseTimer2;
	//__DRAM_IN_865X static unsigned long current_value, timeout;
	static unsigned long current_value, timeout;

	hw = (struct ieee80211_hw *)dev_instance;
	priv = (struct mac80211_shared_priv *)hw->priv;
	phw = GET_HW(priv);
	ioaddr = priv->pshare_hw->ioaddr;

	caseBcnInt = caseBcnStatusOK = caseBcnStatusER = 0;
	caseRxStatus = caseRxRDU = 0;
	caseRxCmd = caseTimer1 = caseTimer2 = 0;


	//spin_lock(&priv->pshare_hw->lock);
	// For fixing 92SE  We must only clear IMR mask register


	status = RTL_R32(ISR);
	
	//printk("ISR 0x%x\n", RTL_R32(ISR));

	RTL_W32(ISR, status);

	status_ext = RTL_R32(ISR+4);
	if (status_ext == 0) {
		RetryCounter1++;
		if (RetryCounter1 > 100) {
			printk("!");
			while (status_ext == 0) {
				status_ext = RTL_R32(ISR+4);
			}
			RetryCounter1 = 0;
		}
	}
	else {
		RetryCounter1 = 0;
	}


	RTL_W32(ISR+4, status_ext);
	if (status == 0 && status_ext == 0) {
		
		return FAIL;
	}

	if (status & (IMR_BcnInt | IMR_BCNDMAINT1 | IMR_BCNDMAINT2 | IMR_BCNDMAINT3 | IMR_BCNDMAINT4))
		caseBcnInt = 1;

	if (status & (IMR_BDOK | IMR_BCNDOK1 | IMR_BCNDOK2 | IMR_BCNDOK3 | IMR_BCNDOK4))
		caseBcnStatusOK = 1;

	if (status_ext & IMR_TBDOK)
		caseBcnStatusOK = 1;

	if (status_ext & IMR_TBDER)
		caseBcnStatusER = 1;

	if (status & (IMR_ROK | IMR_RDU))
		caseRxStatus = 1;

	if (status & IMR_RDU) {
		priv->ext_stats.rx_rdu++;
		caseRxRDU = 1;
	}

	if (status & IMR_RXFOVW)
		priv->ext_stats.rx_fifoO++;

	if (status & IMR_RXCMDOK)
		caseRxCmd = 1;

	if (status & IMR_TIMEOUT1)
		caseTimer1 = 1;

	if (status & IMR_TIMEOUT2)
		caseTimer2 = 1;

	if (caseBcnInt){
#ifdef NEW_MAC80211_DRV
		mac80211_send_beacon(priv);
#else
		//insert old beacon procedure here
#endif
	}
	//if (caseBcnStatusOK || caseBcnStatusER){}


	//
	// Rx interrupt
	//
	if (caseRxStatus)
	{
		// stop RX first
		if (!priv->pshare_hw->has_triggered_rx_tasklet) {
			priv->pshare_hw->has_triggered_rx_tasklet = 1;
			RTL_W32(IMR, priv->pshare_hw->InterruptMask & ~(IMR_RXFOVW | IMR_RDU | IMR_ROK));			
			tasklet_hi_schedule(&priv->pshare_hw->rx_tasklet);
		}
	}

	//
	// Tx interrupt
	//

	//printk("%d - %d\n", phw->txhead0, phw->txtail0);
	if ((CIRC_CNT_RTK(phw->txhead0, phw->txtail0, NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead1, phw->txtail1, NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead2, phw->txtail2, NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead3, phw->txtail3, NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead4, phw->txtail4, NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead5, phw->txtail5, NUM_TX_DESC) > 10)
	) {

		if (!priv->pshare_hw->has_triggered_tx_tasklet) {
			tasklet_schedule(&priv->pshare_hw->tx_tasklet);
			priv->pshare_hw->has_triggered_tx_tasklet = 1;
		}
	}

	//
	// Rx command queue
	//
	
	if (caseRxCmd)
		printk("int: caseRxCmd\n");
		//rtl8190_rxcmd_isr(priv);


	if (caseTimer1) {
		//printk("int: caseTimer1\n");
		RTL_W32(IMR, RTL_R32(IMR) & ~IMR_TIMEOUT1);
#ifdef BUFFER_TX_AMPDU
		current_value = RTL_R32(TSFR) ;
		timeout = RTL_R32(TIMER0);
		if (TSF_LESS(current_value, timeout))
			setup_timer1(priv, timeout);
		else
			ampdu_timeout(priv, current_value);
#endif
	}

	//spin_unlock(&priv->pshare_hw->lock);
	
	return SUCCESS;
}

int pci_stop_sw(struct mac80211_shared_priv *priv)
{
	struct rtl8190_hw *phw;
	unsigned long	flags;
	int	i;
	struct ieee80211_hw *hw = priv->hw;

	// we hope all this can be done in critical section
	SAVE_INT_AND_CLI(flags);

	free_irq(priv->pshare_hw->pdev->irq, hw);
	
/*	del_timer_sync(&priv->frag_to_filter);
	del_timer_sync(&priv->expire_timer);
	del_timer_sync(&priv->pshare->rc_sys_timer);
	del_timer_sync(&priv->ss_timer);
	del_timer_sync(&priv->MIC_check_timer);
	del_timer_sync(&priv->assoc_reject_timer);
	// to avoid add RAtid fail
	del_timer_sync(&priv->add_RATid_timer);


	// for SW LED
	if ((LED_TYPE >= LEDTYPE_SW_LINK_TXRX) && (LED_TYPE < LEDTYPE_SW_MAX))
		disable_sw_LED(priv);*/

	tasklet_kill(&priv->pshare_hw->rx_tasklet);
	tasklet_kill(&priv->pshare_hw->tx_tasklet);
	//tasklet_kill(&priv->pshare->oneSec_tasklet);

	phw = GET_HW(priv);

	for (i=0; i<NUM_RX_DESC; i++)
	{
		pci_unmap_single(priv->pshare_hw->pdev, phw->rx_infoL[i].paddr, 
				(MAX_RX_BUF_LEN - sizeof(struct rx_frinfo)), PCI_DMA_FROMDEVICE);
		//rtl_kfree_skb(priv, (struct sk_buff*)(phw->rx_infoL[i].pbuf), _SKB_RX_);
		dev_kfree_skb_any((struct sk_buff*)(phw->rx_infoL[i].pbuf));
	}

	// free the skb buffer in Low and Hi queue
	DEBUG_INFO("free tx Q0 head %d tail %d\n", phw->txhead0, phw->txtail0);
	DEBUG_INFO("free tx Q1 head %d tail %d\n", phw->txhead1, phw->txtail1);
	DEBUG_INFO("free tx Q2 head %d tail %d\n", phw->txhead2, phw->txtail2);
	DEBUG_INFO("free tx Q3 head %d tail %d\n", phw->txhead3, phw->txtail3);
	DEBUG_INFO("free tx Q4 head %d tail %d\n", phw->txhead4, phw->txtail4);
	DEBUG_INFO("free tx Q5 head %d tail %d\n", phw->txhead5, phw->txtail5);

	for (i=0; i<NUM_TX_DESC; i++)
	{
		// free tx queue skb
		struct tx_desc_info *tx_info;
		int j;
		int	head, tail;

		for (j=0; j<=HIGH_QUEUE; j++) {
			head = get_txhead(phw, j);
			tail = get_txtail(phw, j);

			if( (tail < head) ? (i <tail || i >= head) :(i <tail && i >= head))
				continue;

			tx_info = get_txdesc_info(priv->pshare_hw->pdesc_info, j);

			if (tx_info[i].pframe &&
				(tx_info[i].type == _SKB_FRAME_TYPE_)) {
					pci_unmap_single(priv->pshare_hw->pdev, tx_info[i].paddr, (tx_info[i].len), PCI_DMA_TODEVICE);

				//rtl_kfree_skb(priv, tx_info[i].pframe, _SKB_TX_);
				dev_kfree_skb_any(tx_info[i].pframe);
				DEBUG_INFO("free skb in queue %d\n", j);
			}
		}
	} // TX descriptor Free


	pci_unmap_single(priv->pshare_hw->pdev, get_desc(phw->tx_descB->Dword8),
			128*sizeof(unsigned int), PCI_DMA_TODEVICE);


	RESTORE_INT(flags);

	return 0;
}

int pci_init_sw(struct mac80211_shared_priv *priv)
{
	int rc;
	struct rtl8190_hw *phw=NULL;
	unsigned long ring_virt_addr;
	unsigned long ring_dma_addr;
	unsigned int  ring_buf_len;
	unsigned char MIMO_TR_hw_support;
	unsigned int NumTotalRFPath;
	unsigned long alloc_dma_buf;
	unsigned int  i;
	struct sk_buff	*pskb;
	unsigned char	*page_ptr;
	struct ieee80211_hw *hw = priv->hw;
	
	tasklet_init(&priv->pshare_hw->rx_tasklet, rtl80211_rx_tasklet, (unsigned long)priv);
	tasklet_init(&priv->pshare_hw->tx_tasklet, rtl80211_tx_tasklet, (unsigned long)priv);
	//tasklet_init(&priv->pshare_hw->oneSec_tasklet, rtl80211_expire_timer, (unsigned long)priv);

	if (hw->flags & IEEE80211_HW_AMPDU_AGGREGATION) {
		priv->sc_flags |= SC_OP_TXAGGR;
		priv->sc_flags |= SC_OP_RXAGGR;
	}

	phw = GET_HW(priv);

	ring_virt_addr = phw->ring_virt_addr;
	ring_dma_addr = phw->ring_dma_addr;
	ring_buf_len = phw->ring_buf_len;
#ifndef PRIV_STA_BUF
	alloc_dma_buf = phw->alloc_dma_buf;
#endif
	MIMO_TR_hw_support = phw->MIMO_TR_hw_support;
	NumTotalRFPath = phw->NumTotalRFPath;

	memset((void *)phw, 0, sizeof(struct rtl8190_hw));
	phw->ring_virt_addr = ring_virt_addr;
	phw->ring_buf_len = ring_buf_len;
#ifndef PRIV_STA_BUF
	phw->alloc_dma_buf = alloc_dma_buf;
#endif
	phw->ring_dma_addr = ring_dma_addr;
	phw->MIMO_TR_hw_support = MIMO_TR_hw_support;
	phw->NumTotalRFPath = NumTotalRFPath;
	

	page_ptr = (unsigned char *)phw->ring_virt_addr;

	printk("ptr = %lx len=%lx\n", (unsigned long) page_ptr, (unsigned long) phw->ring_buf_len);
	memset(page_ptr, 0, phw->ring_buf_len); // this is vital!

	phw->rx_ring_addr  = phw->ring_dma_addr;
	printk("rx_ring_addr = %lx\n", (unsigned long) phw->rx_ring_addr);
	phw->tx_ring0_addr = phw->ring_dma_addr + NUM_RX_DESC * sizeof(struct rx_desc);
	printk("tx_ring0_addr = %lx\n", (unsigned long) phw->tx_ring0_addr);
	phw->tx_ring1_addr = phw->tx_ring0_addr + NUM_TX_DESC * sizeof(struct tx_desc);
	printk("tx_ring1_addr = %lx\n", (unsigned long) phw->tx_ring1_addr);
	phw->tx_ring2_addr = phw->tx_ring1_addr + NUM_TX_DESC * sizeof(struct tx_desc);
	printk("tx_ring2_addr = %lx\n", (unsigned long) phw->tx_ring2_addr);
	phw->tx_ring3_addr = phw->tx_ring2_addr + NUM_TX_DESC * sizeof(struct tx_desc);
	printk("tx_ring3_addr = %lx\n", (unsigned long) phw->tx_ring3_addr);
	phw->tx_ring4_addr = phw->tx_ring3_addr + NUM_TX_DESC * sizeof(struct tx_desc);
	printk("tx_ring4_addr = %lx\n", (unsigned long) phw->tx_ring4_addr);
	phw->tx_ring5_addr = phw->tx_ring4_addr + NUM_TX_DESC * sizeof(struct tx_desc);
	printk("tx_ring5_addr = %lx\n", (unsigned long) phw->tx_ring5_addr);
	phw->rxcmd_ring_addr = phw->tx_ring5_addr + NUM_TX_DESC * sizeof(struct tx_desc);
	printk("rxcmd_ring_addr = %lx\n", (unsigned long) phw->rxcmd_ring_addr);
	phw->txcmd_ring_addr = phw->rxcmd_ring_addr + NUM_CMD_DESC * sizeof(struct rx_desc);
	printk("txcmd_ring_addr = %lx\n", (unsigned long) phw->txcmd_ring_addr);
	phw->tx_ringB_addr = phw->txcmd_ring_addr + NUM_CMD_DESC * sizeof(struct tx_desc);
	printk("tx_ringB_addr = %lx\n", (unsigned long) phw->txcmd_ring_addr);

	phw->rx_descL = (struct rx_desc *)page_ptr;
	printk("rx_descL = %lx\n", (unsigned long) phw->rx_descL);
	phw->tx_desc0 = (struct tx_desc *)(page_ptr + NUM_RX_DESC * sizeof(struct rx_desc));
	printk("tx_desc0 = %lx\n", (unsigned long) phw->tx_desc0);
	phw->tx_desc1 = (struct tx_desc *)((unsigned long)phw->tx_desc0 + NUM_TX_DESC * sizeof(struct tx_desc));
	printk("tx_desc1 = %lx\n", (unsigned long) phw->tx_desc1);
	phw->tx_desc2 = (struct tx_desc *)((unsigned long)phw->tx_desc1 + NUM_TX_DESC * sizeof(struct tx_desc));
	printk("tx_desc2 = %lx\n", (unsigned long) phw->tx_desc2);
	phw->tx_desc3 = (struct tx_desc *)((unsigned long)phw->tx_desc2 + NUM_TX_DESC * sizeof(struct tx_desc));
	printk("tx_desc3 = %lx\n", (unsigned long) phw->tx_desc3);
	phw->tx_desc4 = (struct tx_desc *)((unsigned long)phw->tx_desc3 + NUM_TX_DESC * sizeof(struct tx_desc));
	printk("tx_desc4 = %lx\n", (unsigned long) phw->tx_desc4);
	phw->tx_desc5 = (struct tx_desc *)((unsigned long)phw->tx_desc4 + NUM_TX_DESC * sizeof(struct tx_desc));
	printk("tx_desc5 = %lx\n", (unsigned long) phw->tx_desc5);
	phw->rxcmd_desc = (struct rx_desc *)((unsigned long)phw->tx_desc5 + NUM_TX_DESC * sizeof(struct tx_desc));
	printk("rxcmd_desc = %lx\n", (unsigned long) phw->rxcmd_desc);
	phw->txcmd_desc = (struct tx_desc *)((unsigned long)phw->rxcmd_desc + NUM_CMD_DESC * sizeof(struct rx_desc));
	printk("txcmd_desc = %lx\n", (unsigned long) phw->txcmd_desc);
	phw->tx_descB = (struct tx_desc *)((unsigned long)phw->txcmd_desc + NUM_CMD_DESC * sizeof(struct tx_desc));
	printk("tx_descB = %lx\n", (unsigned long) phw->tx_descB);


	/* RX RING */
	for (i=0; i<NUM_RX_DESC; i++) {
		phw->rx_descL_dma_addr[i] = get_physical_addr(priv, (void *)(&phw->rx_descL[i]),
			sizeof(struct rx_desc), PCI_DMA_TODEVICE);
	}

	for (i=0; i<NUM_CMD_DESC; i++) {
		phw->rxcmd_desc_dma_addr[i] = get_physical_addr(priv, (void *)(&phw->rxcmd_desc[i]),
			sizeof(struct rx_desc), PCI_DMA_TODEVICE);
		phw->txcmd_desc_dma_addr[i] = get_physical_addr(priv, (void *)(&phw->txcmd_desc[i]),
			sizeof(struct tx_desc), PCI_DMA_TODEVICE);
	}
//	skb_queue_head_init(&priv->pshare_hw->skb_queue);

	for(i=0; i<NUM_RX_DESC; i++) {
		pskb = rtl_dev_alloc_skb(priv, MAX_RX_BUF_LEN, _SKB_RX_, 1);
		if (pskb == NULL) {
			printk("can't allocate skbuff for RX, abort!\n");
			return 1;
		}
		init_rxdesc(pskb, i, priv);
	}

	// Nothing to do for Tx desc...
	for(i=0; i<NUM_TX_DESC; i++)
	{
		init_txdesc(priv, phw->tx_desc0, phw->tx_ring0_addr, i);
		init_txdesc(priv, phw->tx_desc1, phw->tx_ring1_addr, i);
		init_txdesc(priv, phw->tx_desc2, phw->tx_ring2_addr, i);
		init_txdesc(priv, phw->tx_desc3, phw->tx_ring3_addr, i);
		init_txdesc(priv, phw->tx_desc4, phw->tx_ring4_addr, i);
		init_txdesc(priv, phw->tx_desc5, phw->tx_ring5_addr, i);
	}

	for(i=0; i<NUM_CMD_DESC; i++) {
		if (i == (NUM_CMD_DESC - 1))// set NextAddrs
			(phw->txcmd_desc + i)->Dword9 = set_desc(phw->txcmd_ring_addr);
		else
			(phw->txcmd_desc + i)->Dword9 = set_desc(phw->txcmd_ring_addr + (i+1) * sizeof(struct tx_desc));
	}

	rc = request_irq(priv->pshare_hw->pdev->irq, &rtl8190_interrupt, IRQF_SHARED, wiphy_name(hw->wiphy), hw);

	printk("request_irq\n");

	if (rc) {
		printk(KERN_ERR "%s: failed to register IRQ handler\n",
		       wiphy_name(hw->wiphy));
	}

	priv->pshare_hw->rf_ft_var.raGoDownUpper=50;
	priv->pshare_hw->rf_ft_var.raGoUpUpper=55;
	priv->pshare_hw->rf_ft_var.raGoDown20MLower=18;
	priv->pshare_hw->rf_ft_var.raGoUp20MLower=23;
	priv->pshare_hw->rf_ft_var.raGoDown40MLower=15;
	priv->pshare_hw->rf_ft_var.raGoUp40MLower=20;	

	return 0;
}

int MDL_DEVINIT rtl8190_init_one(struct pci_dev *pdev,
                  const struct pci_device_id *ent)
{
        struct net_device *dev;
	struct ieee80211_hw *hw;
	struct mac80211_shared_priv *priv;
	static struct net_device *ndev;
	struct rtl8190_hw		*phw;
	struct rtl8190_tx_desc_info		*ptxdesc;
	struct priv_shared_hw *pshare_hw;	// david
	unsigned char *page_ptr;
	unsigned char hwaddr[6] = {0x00, 0xa9, 0x87, 0x65, 0x43, 0x20};
	
	unsigned short pci_command;
	int rc;
	unsigned long io_addr, io_len;
	unsigned long mem_addr, mem_len;
	void __iomem *mem;
	struct wifi_mib *mib;

	priv = NULL;
	phw = NULL;
	ptxdesc = NULL;
	pshare_hw = NULL;

	phw = &hw_info;
	memset((void *)phw, 0, sizeof(struct rtl8190_hw));
	ptxdesc = &phw->tx_info;
	pshare_hw = &shared_hw;
	memset((void *)pshare_hw, 0, sizeof(struct priv_shared_hw));

	rc = pci_dev_init(pdev, priv, &mem_len, &io_len);
	mem = pci_iomap(pdev, 1, mem_len);
	if (!mem)
		mem = pci_iomap(pdev, 0, io_len);

	if (!mem) {
		printk(KERN_ERR "%s (rtl8192): Cannot map device memory\n",
		       pci_name(pdev));
		goto err_out_free;
	}

	pci_set_master(pdev);

	hw = ieee80211_alloc_hw(sizeof(struct mac80211_shared_priv), &rtl819x_ops);
	if (!hw) {
		printk(KERN_ERR "ieee80211_alloc_hw() error!\n");
		rc = -ENOMEM;
		goto err_out_res;
	}

	priv = hw->priv;
	priv->hw = hw;

	priv->pshare_hw = pshare_hw;
	priv->pshare_hw->phw = phw;
	priv->pshare_hw->pdesc_info = ptxdesc;
	priv->pshare_hw->pdev = pdev;
	/* copy MAC address */
	memcpy(priv->hwaddr, hwaddr, MACADDRLEN);
	
	spin_lock_init(&priv->pshare_hw->lock);
	spin_lock_init(&priv->rxlock);

	SET_IEEE80211_DEV(hw, &pdev->dev);
	pci_set_drvdata(pdev, hw);

	priv->pshare_hw->ioaddr = (unsigned long)mem;

	rtl8192se_ePhyInit(priv);

	pci_read_config_word(pdev, PCI_COMMAND, &pci_command);
	printk("pci command = %x\n", pci_command);
	//Always ignore Parity error
	if (pci_command & PCI_COMMAND_PARITY) {
		pci_command &= ~PCI_COMMAND_PARITY;
		pci_write_config_word(pdev, PCI_COMMAND, pci_command);
	}

	page_ptr = pci_alloc_consistent(priv->pshare_hw->pdev, DESC_DMA_PAGE_SIZE, (dma_addr_t *)&phw->ring_dma_addr);

	if (page_ptr == NULL) {
			printk(KERN_ERR "can't allocate descriptior page, abort!\n");
			goto err_out_iomap;
	}
#ifndef PRIV_STA_BUF
	phw->alloc_dma_buf = (unsigned long)page_ptr;	
#endif
	page_ptr = (unsigned char *)
		(((unsigned long)page_ptr) + (PAGE_SIZE - (((unsigned long)page_ptr) & (PAGE_SIZE-1))));
#ifndef PRIV_STA_BUF
	phw->ring_buf_len = ((unsigned long)phw->alloc_dma_buf) + DESC_DMA_PAGE_SIZE - ((unsigned long)page_ptr);
#endif
	phw->ring_dma_addr = 
		(((unsigned long)phw->ring_dma_addr) + (PAGE_SIZE - (((unsigned long)phw->ring_dma_addr) & (PAGE_SIZE-1))));
	//phw->ring_dma_addr = virt_to_bus(page_ptr);
	//page_ptr = (unsigned char *)KSEG1ADDR(page_ptr);
#ifndef PRIV_STA_BUF
	printk("alloc=%lx page_ptr=%lx, alloc_size=%lx, buf_size=%lx\n", (unsigned long) phw->alloc_dma_buf,
				(unsigned long)page_ptr, (unsigned long)DESC_DMA_PAGE_SIZE, (unsigned long) phw->ring_buf_len);
#else
	printk("page_ptr=%lx, alloc_size=%lx, buf_size=%lx\n", 
				(unsigned long)page_ptr, (unsigned long)DESC_DMA_PAGE_SIZE, (unsigned long) phw->ring_buf_len);
#endif
	phw->ring_virt_addr = (unsigned long)page_ptr;

	SET_IEEE80211_PERM_ADDR(hw, priv->hwaddr);

	rtl_set_hw_capab(priv, hw);

	rc = ieee80211_register_hw(hw);
	if (rc) {
		printk(KERN_ERR "%s (rtl8192se): %d Cannot register device\n",
		       pci_name(pdev), rc);
		goto err_out_iomap;
	}

	printk(KERN_INFO "Init %s, base_addr=%08x, irq=%d\n",
				wiphy_name(hw->wiphy), (UINT)priv->pshare_hw->ioaddr,  pdev->irq);	

	printk(KERN_INFO "%s: hwaddr %pM\n",
	       wiphy_name(hw->wiphy), hw->wiphy->perm_addr);
	
        mib = (struct wifi_mib *)kmalloc((sizeof(struct wifi_mib)), GFP_KERNEL);
	if( !mib ){
		rc = -ENOMEM;
		//printk(KERN_ERR "Can't kmalloc for wifi_mib (size %d)\n", sizeof(struct wifi_mib));
		printk("Can't kmalloc for wifi_mib (size %d)\n", sizeof(struct wifi_mib));
		return rc;
	}
	memset(mib, 0, sizeof(struct wifi_mib));
	priv->pmib = mib;

        /* for iwpriv */
	dev = alloc_etherdev(sizeof(struct rtl8190_priv));
	if (!dev) {
		printk(KERN_ERR "alloc_etherdev() error!\n");
       	        return -ENOMEM;
	}
	sprintf(dev->name, "wlan0-sh");
	dev->priv = priv;
	dev->netdev_ops = &rtl_wireless_netdev_ops;
	rc = register_netdev(dev);
	if (rc)
		printk("register legacy netdev error\n");
	
	return 0;


err_out_iomap:
	iounmap((void __iomem *)priv->pshare_hw->ioaddr);
	
err_out_free:
	pci_set_drvdata(pdev, NULL);
	ieee80211_free_hw(hw);

err_out_res:
	pci_release_regions(pdev);
	pci_disable_device(pdev);
	return rc;
	
}

extern int MDL_DEVINIT rtl8190_init_pci(struct pci_dev *, const struct pci_device_id *);
extern void MDL_DEVEXIT rtk_remove_one(struct pci_dev *);
#ifdef CONFIG_PM
extern int rtl8190_suspend(struct pci_dev *, pm_message_t );
extern int rtl8190_resume(struct pci_dev *);
#endif

#if 0	// move to 819x_pci.c
static int MDL_DEVINIT rtl8190_init_pci(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	int ret;

	ret = rtl8190_init_one(pdev, ent);
	return ret;
}

static void MDL_DEVEXIT rtk_remove_one(struct pci_dev *pdev)
{
    struct ieee80211_hw *hw = pci_get_drvdata(pdev);
    struct mac80211_shared_priv *priv;

    if (!hw) {
        BUG();
		return;
    }

	ieee80211_unregister_hw(hw);

	priv = hw->priv;

    pci_iounmap(pdev, (void __iomem *)priv->pshare_hw->ioaddr);
    pci_release_regions(pdev);
    pci_disable_device(pdev);
	ieee80211_free_hw(hw);
}

static struct pci_device_id MDL_DEVINITDATA rtl8190_pci_tbl[] =
{
	{ PCI_VENDOR_ID_REALTEK, 0x8190,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_REALTEK, 0x8192,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_REALTEK, 0x8171,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_REALTEK, 0x8172,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_REALTEK, 0x8173,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_REALTEK, 0x8174,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ },
};
//MODULE_DEVICE_TABLE(pci, rtl8190_pci_tbl);


#ifdef CONFIG_PM
static int rtl8190_suspend(struct pci_dev *pdev, pm_message_t state)
{
	pci_save_state(pdev);
	pci_set_power_state(pdev, pci_choose_state(pdev, state));
	return 0;
}

static int rtl8190_resume(struct pci_dev *pdev)
{
	pci_set_power_state(pdev, PCI_D0);
	pci_restore_state(pdev);
	return 0;
}
#endif /* CONFIG_PM */
#endif

static struct pci_device_id MDL_DEVINITDATA rtl8190_pci_tbl[] =
{
	{ PCI_VENDOR_ID_REALTEK, 0x8190,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_REALTEK, 0x8192,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_REALTEK, 0x8171,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_REALTEK, 0x8172,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_REALTEK, 0x8173,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_REALTEK, 0x8174,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ },
};


static struct pci_driver rtl8190_driver = {
	.name		=DRV_NAME,
	.id_table	=rtl8190_pci_tbl,
	.probe		=rtl8190_init_pci,
	.remove		=__devexit_p(rtk_remove_one),
#ifdef CONFIG_PM
	.suspend	= rtl8190_suspend,
	.resume 	= rtl8190_resume,
#endif /* CONFIG_PM */
};

int MDL_INIT __rtl8190_init(unsigned long base_addr)
{
	return pci_register_driver(&rtl8190_driver);
	
}

int MDL_INIT rtl8190_init(void)
{
	int err;
	printk(KERN_INFO "MAC80211: Driver(rtl8192se) loaded\n" );
	err = rtl_rate_control_register();
	if (err < 0) {
		printk(KERN_ERR
			"rtl8192se: rate control register failed.\n");
		err = -ENODEV;
		return err;
	}
	err = __rtl8190_init(0);
	if (err < 0) {
		printk(KERN_ERR
			"rtl8192se: No PCI devices found, driver not installed.\n");
		err = -ENODEV;
		return err;
	}
	return 0;
}

static void MDL_EXIT rtl8190_exit (void)
{
	printk(KERN_INFO "MAC80211: Driver(rtl8192se) unloaded\n" );
	pci_unregister_driver (&rtl8190_driver);
}
#endif

//MODULE_AUTHOR("Realtek");
//MODULE_DESCRIPTION("Support for Realtek 802.11n wireless LAN PCI/USB cards.");
//MODULE_SUPPORTED_DEVICE("Realtek 802.11n WLAN PCI/USB cards");
//MODULE_LICENSE("GPL");

module_init(rtl8190_init);
module_exit(rtl8190_exit);




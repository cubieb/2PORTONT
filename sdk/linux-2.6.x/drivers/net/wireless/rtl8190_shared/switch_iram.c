/*      @doc RTL819X Wireless driver 

        @module switch_iram.c - RTL819X wireless driver documentation       |
        This document explains the API interface of the wireless driver module. 
        @normal Hyking Liu (Hyking_liu@realsil.com.cn) <date>

        Copyright <cp>2009 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

        @head3 List of Symbols |
        Here is a list of all functions and variables in this module.
        
        @index | RTL819X Wireless driver
*/

#include <linux/netdevice.h>

#include <common/rtl_types.h>
#include <rtl865xc_swNic.h>
#include "8190n_util.h"

void (*p_handle_int)(void);
void (*p_free_rtl865x_eth_priv_buf)(unsigned char *head);
void (*__p_dev_kfree_skb_any)(struct sk_buff *skb);
int32 (*p_rtl8651_rxPktPreprocess)(void *pkt, unsigned int *vid);
void (*p_release_pkthdr)(struct sk_buff  *skb);
int32 (*p_swNic_receive)(void** input, uint32* pLen, unsigned int *vid, unsigned int *pid);
int32 (*p_swNic_send)(void *skb, void * output, uint32 len,rtl_nicTx_info *nicTx);
int (*p_interrupt_isr)(int irq, void *dev_instance);
int (*__p_netif_rx)(struct sk_buff *skb);
int (*__p_process_backlog)(struct net_device *backlog_dev, int *budget);
void (*p_rtl8192SE_signin_txdesc_shortcut)(struct rtl8190_priv *priv, struct tx_insn *txcfg);
void (*__p_rtl_netif_rx)(struct rtl8190_priv *priv, struct sk_buff *pskb, struct stat_info *pstat);

extern asmlinkage void handle_int(void);
extern asmlinkage void handle_int_wapi(void);
extern void free_rtl865x_eth_priv_buf(unsigned char *head);
extern void free_rtl865x_eth_priv_buf_wapi(unsigned char *head);
extern void __dev_kfree_skb_any(struct sk_buff *skb);
extern void __dev_kfree_skb_any_wapi(struct sk_buff *skb);
extern int32 rtl8651_rxPktPreprocess(void *pkt, unsigned int *vid);
extern int32 rtl8651_rxPktPreprocess_wapi(void *pkt, unsigned int *vid);
extern void release_pkthdr(struct sk_buff  *skb);
extern void release_pkthdr_wapi(struct sk_buff  *skb);
extern int32 swNic_receive(void** input, uint32* pLen, unsigned int *vid, unsigned int *pid);
extern int32 swNic_receive_wapi(void** input, uint32* pLen, unsigned int *vid, unsigned int *pid);
extern int32 swNic_send(void *skb, void * output, uint32 len,rtl_nicTx_info *nicTx);
extern int32 swNic_send_wapi(void *skb, void * output, uint32 len,rtl_nicTx_info *nicTx);
extern int interrupt_isr(int irq, void *dev_instance);
extern int interrupt_isr_wapi(int irq, void *dev_instance);
extern int __netif_rx(struct sk_buff *skb);
extern int __netif_rx_wapi(struct sk_buff *skb);
extern int __process_backlog(struct net_device *backlog_dev, int *budget);
extern int __process_backlog_wapi(struct net_device *backlog_dev, int *budget);
extern void rtl8192SE_signin_txdesc_shortcut(struct rtl8190_priv *priv, struct tx_insn *txcfg);
extern void rtl8192SE_signin_txdesc_shortcut_wapi(struct rtl8190_priv *priv, struct tx_insn *txcfg);
extern void __rtl_netif_rx(struct rtl8190_priv *priv, struct sk_buff *pskb, struct stat_info *pstat);
extern void __rtl_netif_rx_wapi(struct rtl8190_priv *priv, struct sk_buff *pskb, struct stat_info *pstat);


void switch_iram_init(void)
{
	p_free_rtl865x_eth_priv_buf = free_rtl865x_eth_priv_buf;
	__p_dev_kfree_skb_any = __dev_kfree_skb_any;
	p_rtl8651_rxPktPreprocess = rtl8651_rxPktPreprocess;
	p_release_pkthdr = release_pkthdr;
	p_swNic_receive = swNic_receive;
	p_swNic_send = swNic_send;
	p_interrupt_isr = interrupt_isr;
	__p_netif_rx = __netif_rx;
	__p_process_backlog = __process_backlog;
	p_rtl8192SE_signin_txdesc_shortcut = rtl8192SE_signin_txdesc_shortcut;
	__p_rtl_netif_rx = __rtl_netif_rx;
}

extern void _iram_switch(uint32 addr);
int switch_iram(unsigned int identity)
{
	extern char __iram_wapi[],__iram[];
	//unsigned long flags;

	switch(identity)
	{
		case 0:
			printk("====%s(%d) iram_wapi(0x%x) iram(%p)\nswitch to iram\n",__FUNCTION__,__LINE__,(uint32)__iram_wapi,__iram);
			//p_handle_int = handle_int;
			set_except_vector(0, handle_int);
			p_free_rtl865x_eth_priv_buf = free_rtl865x_eth_priv_buf;
			__p_dev_kfree_skb_any = __dev_kfree_skb_any;
			p_rtl8651_rxPktPreprocess = rtl8651_rxPktPreprocess;
			p_release_pkthdr = release_pkthdr;
			p_swNic_receive = swNic_receive;
			p_swNic_send = swNic_send;
			p_interrupt_isr = interrupt_isr;
			__p_netif_rx = __netif_rx;
			__p_process_backlog = __process_backlog;
			p_rtl8192SE_signin_txdesc_shortcut = rtl8192SE_signin_txdesc_shortcut;
			__p_rtl_netif_rx = __rtl_netif_rx;
			cli();
			//SAVE_INT_AND_CLI(flags)
			_iram_switch((uint32)__iram);
			sti();
			//RESTORE_INT(flags);
			break;
		case 1:
			printk("====%s(%d) iram_wapi(0x%x) iram(%p)\nswitch to iram_wapi\n",__FUNCTION__,__LINE__,(uint32)__iram_wapi,__iram);
			set_except_vector(0, handle_int_wapi);
			p_free_rtl865x_eth_priv_buf = free_rtl865x_eth_priv_buf_wapi;
			__p_dev_kfree_skb_any = __dev_kfree_skb_any_wapi;
			p_rtl8651_rxPktPreprocess = rtl8651_rxPktPreprocess_wapi;
			p_release_pkthdr = release_pkthdr_wapi;
			p_swNic_receive = swNic_receive_wapi;
			p_swNic_send = swNic_send_wapi;
			p_interrupt_isr = interrupt_isr_wapi;
			__p_netif_rx = __netif_rx_wapi;
			__p_process_backlog = __process_backlog_wapi;
			p_rtl8192SE_signin_txdesc_shortcut = rtl8192SE_signin_txdesc_shortcut_wapi;
			__p_rtl_netif_rx = __rtl_netif_rx_wapi;
			cli();
			//SAVE_INT_AND_CLI(flags)
			_iram_switch((uint32)__iram_wapi);
			sti();
			//RESTORE_INT(flags);
			break;
		default:			
			printk("wrong parameter!!! the para should be 0 or 1 \n");
	}

	return 0;
}
#if defined(CONFIG_RTL_WAPI_MODULE_SUPPORT)
EXPORT_SYMBOL(switch_iram);
#endif
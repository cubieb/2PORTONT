/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*/

#ifndef __RTK_RG_FWDENGINE_H__
#define __RTK_RG_FWDENGINE_H__

#ifdef __KERNEL__
#include <linux/skbuff.h>
#else
#define VLAN_HLEN       4 
#define VLAN_ETH_ALEN	6


struct sk_buff {	
	struct sk_buff		*next;
	struct sk_buff		*prev;
	unsigned int		len;	
	unsigned char		*data;
	unsigned short		vlan_tci;

};



#endif

#ifdef CONFIG_RTL8686NIC
#include <linux/interrupt.h>
#if defined(CONFIG_RTL9602C_SERIES)
	#include <re8686_rtl9602c.h>
#else
	#include <re8686.h>
#endif
extern struct net_device* nicRootDev;
extern struct re_private *nicRootDevCp;
extern int drv_nic_register_rxhook(int portmask,int priority,p2rfunc_t rx);
extern int drv_nic_unregister_rxhook(int portmask,int priority,p2rfunc_t rx);
//extern int re8670_start_xmit (struct sk_buff *skb, struct net_device *dev);
extern struct net_device *eth_net_dev;
#endif


#include <rtk_rg_liteRomeDriver.h>
#include <rtk_rg_internal.h>
//#include <rtk_rg_acl.h>
extern struct net_device* decideRxDevice(struct re_private *cp, struct rx_info *pRxInfo);


//#ifdef CONFIG_RTL8686NIC
//extern int rtk_rg_fwdEngine_xmit (struct sk_buff *skb, void *void_ptx, void *void_ptxMask);
//#endif
//extern int _rtk_rg_portBindingLookup(int srcPort, int srcExtPort, int vid, rtk_rg_sipDipClassification_t sipDipClass, rtk_rg_pktHdr_t *pPktHdr);

#if defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) && defined(CONFIG_XDSL_ROMEDRIVER)
extern int re8670_rx_skb (struct re_private *cp, struct sk_buff *skb, struct rx_info *pRxInfo);

#endif

extern int _rtk_rg_l3lookup(ipaddr_t ip);
extern int _rtk_rg_v6L3lookup(unsigned char *ipv6);



//typedef int (*p_nicTxCallBack)(void*, rtk_rg_txdesc_t*, int);
//typedef unsigned char (*p_nicRxCallBack)(void*, rtk_rg_rxdesc_t*);

extern void rg_proc_init(void);
extern int _rtk_rg_aclDecisionClear(rtk_rg_pktHdr_t *pPktHdr);


#ifdef __KERNEL__
rtk_rg_fwdEngineReturn_t rtk_rg_fwdEngineInput(struct re_private *cp, struct sk_buff *net_buf, void *pRxDescPtr);
int rtk_rg_fwdEngine_xmit(struct sk_buff *skb, struct net_device *dev);
int fwdEngine_rx_skb(struct re_private *cp, struct sk_buff *skb,struct rx_info *pRxInfo);

#ifdef CONFIG_SMP
void _rtk_rg_tasklet_queue_func(struct rg_private *rg_data);
#endif
#endif



int fwdEngine_pkt_tx_testing(void);


/* Function Name:
 *      rtk_rg_tx_setting
 * Description:
 *      Read tx information of netowrk buffer and set to tx descriptor data structure.
 * Input:
 * Output:
 *      buf - [in]<tab>Netowrk buffer.
 *		pTxInfo - [in]<tab>Tx descriptor data structure.<nl>[out]<tab>None.
 * Return:
 *      RT_ERR_RG_OK
 * Note:
 *      buf->l3cs - set to pTxInfo->opts1.bit.cputag_ipcs.<nl>
 *      buf->l4cs - set to pTxInfo->opts1.bit.cputag_l4cs.<nl>
 *      buf->txmsk - set to pTxInfo->opts3.bit.tx_portmask.<nl>
 *      buf->cpu_prisel - set to pTxInfo->opts2.bit.aspri.<nl>
 *      bufvcpu_priority - set to pTxInfo->opts2.bit.cputag_pri.<nl>
 *      buf->keep - set to pTxInfo->opts1.bit.keep.<nl>
 *      skb->l34keep - set to pTxInfo->opts3.bit.L34_keep.<nl>
 *      buf->extspa - set to pTxInfo->opts3.bit.extspa.<nl>
 *      buf->pppoeAct - set to pTxInfo->opts2.bit.tx_pppoe_action.<nl>
 *      buf->pppoeIdx - set to pTxInfo->opts2.bit.tx_pppoe_idx.<nl>
 *      buf->l2br - set to pTxInfo->opts1.bit.blu.<nl>
 */
void _rtk_rg_fwdEngineTxDescSetting(void *pTxInfo,void *ptx,void *ptxMask);

rtk_rg_successFailReturn_t rtk_rg_rome_driver_init(void);


#endif

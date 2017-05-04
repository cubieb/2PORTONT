/**************************************************
 * Realtek VoIP suite
 * Bruce (kfchang@realtek.com.tw)
 * ***********************************************/



//#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/if_ether.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/inet.h>
#include <linux/version.h>
#include <asm/uaccess.h>
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
#ifdef CONFIG_RTK_VOIP_RX_NETFILTER_HOOK
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/ip.h>
#include <net/ip.h>
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
#include <linux/in6.h>
#include <net/ipv6.h>
#include <linux/netfilter_ipv6.h>
#endif
#endif
#endif

#include "rtk_voip.h"
#include "rtk_trap.h"
#include "voip_types.h"
//#include "../voip_manager/voip_mgr_netfilter.h"
#include "voip_init.h"
#include "voip_proc.h"

#ifdef CONFIG_RTK_VOIP_SRTP
#include "cp3_profile.h"
#include "srtp.h"
#endif

void **udp_rtk_trap_profile_header = NULL;
int (*udp_rtk_trap_hook)(struct sk_buff *skb) = NULL;
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
int (*ip6_udp_rtk_trap_hook)(struct sk_buff *skb) = NULL;
#endif

#ifdef CONFIG_RTK_VOIP_QOS
extern int voip_rx_shortcut_enable;
extern int voip_tx_shortcut_enable;
#endif

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
int rtk_trap(struct sk_buff *skb);
int ip6_rtk_trap(struct sk_buff *skb);
#endif

#ifdef T38_STAND_ALONE_HANDLER
//#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#include "../voip_drivers/t38_handler.h"
//#else
//#include "../voip_manager/voip_mgr_help_host.h"
//#endif
#endif

struct RTK_TRAP_profile *filter[MAX_RTP_TRAP_SESSION]={0};

#define VOIP_RX_IN_UDP // move rtk_trap() from /net/core/dev.c to /net/ipv4/udp.c

#if defined(VOIP_RX_IN_UDP)
int (*rtk_trap_hook)(struct sk_buff *skb) = NULL;
#endif

//int (*rtk_trap_hook)(struct sk_buff *skb) = NULL;
extern int (*rtk_trap_hook)(struct sk_buff *skb);
struct RTK_TRAP_profile *header;

#ifndef AUDIOCODES_VOIP
uint32 nRxRtpStatsCountByte[MAX_DSP_RTK_SS_NUM];
uint32 nRxRtpStatsCountPacket[MAX_DSP_RTK_SS_NUM];
uint32 nRxRtpStatsLostPacket[MAX_DSP_RTK_SS_NUM];
#endif

#ifdef SUPPORT_VOIP_DBG_COUNTER
extern uint32 gVoipCounterEnable;
extern void RTP_rx_count(uint32 sid);
#endif

#ifdef CONFIG_RTK_VOIP_SRTP
err_status_t status;

// For kernel SRTP test only
/*char key[30] = { 0, 1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,  \
		20, 21, 22, 23, 24, 25, 26, 27, 28, 29};*/
void srtp_polity_init(struct RTK_TRAP_profile *ptr)
{
        ptr->applySRTP = 1;
	// TX
	crypto_policy_set_rtp_default(&ptr->tx_policy.rtp);
	crypto_policy_set_rtcp_default(&ptr->tx_policy.rtcp);
	ptr->tx_policy.ssrc.type  = ssrc_any_outbound;
	ptr->tx_policy.key = (uint8_t *)ptr->local_inline_key;
	//ptr->tx_policy.key  = (uint8_t *) key;
	
	ptr->tx_policy.next = NULL;
	ptr->tx_policy.rtp.sec_serv = sec_serv_conf | sec_serv_auth;
	//ptr->tx_policy.rtcp.sec_serv = sec_serv_none;  /* we don't do RTCP anyway */
	ptr->tx_policy.rtcp.sec_serv = sec_serv_conf_and_auth;
	status = srtp_create(&ptr->tx_srtp_ctx, &ptr->tx_policy);
	if (status) {
		PRINT_R("error: srtp_create() failed with code %d\n",status);
	}

	// RX
	crypto_policy_set_rtp_default(&ptr->rx_policy.rtp);
	crypto_policy_set_rtcp_default(&ptr->rx_policy.rtcp);
	ptr->rx_policy.ssrc.type  = ssrc_any_inbound;
	ptr->rx_policy.key = (uint8_t *)ptr->remote_inline_key;
	//ptr->rx_policy.key  = (uint8_t *) key;
	
	ptr->rx_policy.next = NULL;
	ptr->rx_policy.rtp.sec_serv = sec_serv_conf | sec_serv_auth;
	//ptr->rx_policy.rtcp.sec_serv = sec_serv_none;  /* we don't do RTCP anyway */
	ptr->rx_policy.rtcp.sec_serv = sec_serv_conf_and_auth;
	status = srtp_create(&ptr->rx_srtp_ctx, &ptr->rx_policy);
    	if (status) {
		PRINT_R("error: srtp_create() failed with code %d\n",status);
	}	

}
#endif /* CONFIG_RTK_VOIP_SRTP */

int32 rtk_trap_register(TstVoipMgrSession *stVoipMgrSession, uint8 c_id, uint32 m_id, uint32 s_id, int32(*callback)(uint8 ch_id, int32 m_id, void *ptr_data, uint32 data_len, uint32 flags))
{

	struct RTK_TRAP_profile *ptr;
#if 0
	PRINT_MSG("ip_src_addr = %d\n", stVoipMgrSession->ip_src_addr);
	PRINT_MSG("ip_dst_addr = %d\n", stVoipMgrSession->ip_dst_addr);
	PRINT_MSG("udp_src_port = %d\n", stVoipMgrSession->udp_src_port);
	PRINT_MSG("udp_dst_port = %d\n", stVoipMgrSession->udp_dst_port);
	PRINT_MSG("protocol = %d\n", stVoipMgrSession->protocol);
#endif
#ifdef REG_INIT_SOCKET //for UDP socket send
	TstUDPSession stUDPSession;
	stUDPSession.ip_src_addr = stVoipMgrSession->ip_src_addr;
	stUDPSession.ip_dst_addr = stVoipMgrSession->ip_dst_addr;
	stUDPSession.udp_src_port= stVoipMgrSession->udp_src_port;
	stUDPSession.udp_dst_port= stVoipMgrSession->udp_dst_port;
	udp_socket_establish( &stUDPSession);

#endif
	
//#ifndef AUDIOCODES_VOIP
	if(s_id <0 || s_id > 2*DSP_SS_NUM)
//#else
//	if(s_id <0 || s_id > SESS_NUM)
//#endif
	{
		PRINT_R("s_id %d non-support now!(support 0~%d)\n", s_id, DSP_SS_NUM-1);
		return -1;
	}
	
	if(filter[s_id]!=0) {
		PRINT_R("s_id %d in used, please unregister first\n", s_id);
		return -1;
	}

	if(header == NULL)
	{
		PRINT_MSG("path 0\n");
		header=(struct RTK_TRAP_profile *) kmalloc(sizeof(struct RTK_TRAP_profile), GFP_ATOMIC);	
#ifdef CONFIG_RTK_VOIP_SRTP
		header->applySRTP=0;
#endif
		if(header == NULL)
		{
			PRINT_R("RTK TRAP allocate memory error !!\n");
			return -1;
		}
		header->ip_src_addr = stVoipMgrSession->ip_src_addr;
		header->ip_dst_addr = stVoipMgrSession->ip_dst_addr;
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
		header->ip6_src_addr = stVoipMgrSession->ip6_src_addr;
		header->ip6_dst_addr = stVoipMgrSession->ip6_dst_addr;
		header->ipv6 = stVoipMgrSession->ipv6;
		//if (header->ipv6 == 1)
		//	printk("path0 ipv6\n");
#endif
		header->udp_src_port = stVoipMgrSession->udp_src_port;
		header->udp_dst_port = stVoipMgrSession->udp_dst_port;
		header->protocol = stVoipMgrSession->protocol;
#ifdef SUPPORT_VOICE_QOS
		header->tos = stVoipMgrSession->tos;
#endif
		header->c_id = c_id;
		header->m_id = m_id;
		header->s_id = s_id;
		header->rx_packets = header->rx_bytes = 0;
		header->tx_packets = header->tx_bytes = 0;
		header->rtk_trap_callback=(void *)callback;
#ifdef SUPPORT_QOS_REMARK_PER_SESSION
		memset(&header->qos,0,sizeof(TstQosCfg));
#endif
#ifdef CONFIG_RTK_VOIP_SRTP
		if(stVoipMgrSession->remoteCryptAlg == HMAC_SHA1){
			PRINT_MSG("+++++line:%d, function:%s+++++\n",__LINE__,__FUNCTION__);
			memcpy(header->local_inline_key, stVoipMgrSession->localSrtpKey, 30);
			memcpy(header->remote_inline_key, stVoipMgrSession->remoteSrtpKey, 30);
			srtp_polity_init(header);
		}
#endif		
		header->next=NULL;
		ptr = header;
		filter[s_id] = ptr;
	}	
	else
	{
		PRINT_MSG("path 1\n");
		ptr=header;

		while(ptr!=NULL && ptr->next!=NULL)
		{
			ptr=ptr->next;	
		}

		ptr->next=(struct RTK_TRAP_profile *)kmalloc(sizeof(struct RTK_TRAP_profile), GFP_ATOMIC);	
#ifdef CONFIG_RTK_VOIP_SRTP
		ptr->next->applySRTP=0;
#endif
		if(ptr->next == NULL) 
		{
			PRINT_R("RTK TRAP allocate memory error !!\n");
			return -1;
		}
		ptr->next->ip_src_addr = stVoipMgrSession->ip_src_addr;
		ptr->next->ip_dst_addr = stVoipMgrSession->ip_dst_addr;
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
		ptr->next->ip6_src_addr = stVoipMgrSession->ip6_src_addr;
		ptr->next->ip6_dst_addr = stVoipMgrSession->ip6_dst_addr;
		ptr->next->ipv6 = stVoipMgrSession->ipv6;
		//if (ptr->next->ipv6 == 1)
		//	printk("path1 ipv6\n");
#endif
		ptr->next->udp_src_port = stVoipMgrSession->udp_src_port;
		ptr->next->udp_dst_port = stVoipMgrSession->udp_dst_port;
		ptr->next->protocol = stVoipMgrSession->protocol;
#ifdef SUPPORT_VOICE_QOS
		ptr->next->tos = stVoipMgrSession->tos;
#endif
		ptr->next->c_id= c_id;
		ptr->next->m_id= m_id;
		ptr->next->s_id= s_id;
		ptr->next->rx_packets = ptr->next->rx_bytes = 0;
		ptr->next->tx_packets = ptr->next->tx_bytes = 0;
#ifdef SUPPORT_QOS_REMARK_PER_SESSION
		memset(&ptr->next->qos,0,sizeof(TstQosCfg));
#endif
#ifdef CONFIG_RTK_VOIP_SRTP
		if(stVoipMgrSession->remoteCryptAlg == HMAC_SHA1){
			PRINT_MSG("+++++line:%d, function:%s+++++\n",__LINE__,__FUNCTION__);
			memcpy(ptr->next->local_inline_key, stVoipMgrSession->localSrtpKey, 30);
			memcpy(ptr->next->remote_inline_key, stVoipMgrSession->remoteSrtpKey, 30);
			srtp_polity_init(ptr->next);
		}
#endif		
		ptr->next->rtk_trap_callback=(void *)callback;
		ptr->next->next=NULL;

	filter[s_id] = ptr->next;
	}
	PRINT_MSG("establish complete\n");
	return 0;
}


int32 rtk_trap_unregister(uint32 s_id)
{
	struct RTK_TRAP_profile *ptr, *ptr1;

	ptr=header;
	ptr1=header;

//#ifndef AUDIOCODES_VOIP
	if(s_id <0 || s_id > 2*DSP_SS_NUM)
//#else
//	if(s_id <0 || s_id > SESS_NUM)
//#endif
	{
		PRINT_MSG("s_id %d non-support now!(support 0~%d)\n", s_id, DSP_SS_NUM-1);
		return -1;
	}

	if(filter[s_id]==0) {
		PRINT_MSG("s_id %d non-used now, can't unregister\n", s_id);
		return -1;
	}
	while(ptr != NULL) {
		if(ptr->s_id==s_id) {
			filter[s_id]=0;
			if(ptr!=header) {
				ptr1->next=ptr->next;
			} else {
				header=header->next;
			}
#ifdef CONFIG_RTK_VOIP_SRTP
			if( 1 == ptr->applySRTP){
				status = srtp_dealloc(ptr->rx_srtp_ctx);
		    		if (status) {
					PRINT_MSG("error: srtp_dealloc() failed with code %d\n",status);
				}	
				status = srtp_dealloc(ptr->tx_srtp_ctx);
		    		if (status) {
					PRINT_MSG("error: srtp_dealloc() failed with code %d\n",status);
				}	
				memset(ptr, 0, sizeof(ptr));
			}
#endif			
			kfree(ptr);
			return 0;
		}
		ptr1 = ptr;
		ptr = ptr->next;
	}
	PRINT_MSG("no found\n");
	return -1;
}

int get_filter(uint8 ch_id, struct RTK_TRAP_profile *myptr)
{
	struct RTK_TRAP_profile *ptr;

        ptr=header;
	while( ptr !=NULL )
        {
                if(ptr->c_id!=ch_id){
                  ptr = ptr->next;
                }
		myptr = ptr;
                return 0;
        }
	return -1;
}

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
inline int rtk_trap_callback_trap( const struct RTK_TRAP_profile *ptr, 
								   const struct sk_buff *skb )
{
#ifdef T38_STAND_ALONE_HANDLER
	if( t38RunningState[ ptr->c_id ] == T38_START &&
		ptr ->s_id < RTCP_SID_OFFSET )
	{

#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
		uint32 udp_len;

		if (ptr->ipv6 == 1) // T.38 over IPv6
		{
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
    	struct ipv6hdr * const iph = ipv6_hdr(skb);
    		udp_len = (iph->payload_len) - 8;   
#else
			udp_len = ((skb->nh.ipv6h)->payload_len) - 8;
#endif
		}
		else	// T.38 over IPv4
		{
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
			struct iphdr * const iph = ip_hdr(skb);
			udp_len = (iph->tot_len) - 28;
#else
			udp_len = ((skb->nh.iph)->tot_len) - 28;
#endif
		}
#else
	#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
		struct iphdr * const iph = ip_hdr(skb);
		const uint32 udp_len = (iph->tot_len) - 28;
	#else
		const uint32 udp_len = ((skb->nh.iph)->tot_len) - 28;
	#endif
#endif // CONFIG_RTK_VOIP_IPV6_SUPPORT

		unsigned char * const udp_data = skb->data + sizeof(struct udphdr);
		
		const uint32 chid = ptr->c_id;
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
		const uint32 sid = ptr->s_id;
#endif
		
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
		extern void ipc_Host2DSP_T38Tx( uint32 chid_pkt, uint32 sid_pkt, void* packet, uint32 pktLen );

		//t.38 packet , remove udp header
		ipc_Host2DSP_T38Tx( chid, sid, udp_data, udp_len );
#else
	#if 1
		T38_API_PutPacket( chid, udp_data, udp_len );
	#else
    #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
		struct iphdr *iph = ip_hdr(skb);
		T38_API_PutPacket( ptr->c_id, skb->data + sizeof(struct udphdr), (iph->tot_len) - 28 );
    #else
		T38_API_PutPacket( ptr->c_id, skb->data + sizeof(struct udphdr), ((skb->nh.iph)->tot_len) - 28 );
    #endif
    #endif
#endif
		
		return 1;
	}
#endif /* T38_STAND_ALONE_HANDLER */

	return 0;
}
#endif //#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP

/****************************************************************/
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP

/*receive skb from netif_rx() */
int rtk_rtp_rx(struct sk_buff *skb)
{
	struct iphdr *iph;
	struct ipv6hdr *iphv6;

	//Change sdk->data from ip header to udp header

	if (skb->protocol!=ETH_P_IP){
		//printk("rtk receive skb->protocol is %x\n",skb->protocol);
		return RTK_TRAP_NONE;
	}

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	skb_pull(skb, ip_hdrlen(skb));
#endif
	iph = ip_hdr(skb);

	if(iph->version==4){
		//parse skb by ipv4
		
		if(rtk_trap(skb)==RTK_TRAP_NONE){
			/* not RTP packet, return to netif_rx */
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
			skb_push(skb, ip_hdrlen(skb));
#endif
			return RTK_TRAP_NONE;
		}else
			return RTK_TRAP_ACCEPT;

	}else if(iph->version==6){

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP 
		skb_push(skb, ip_hdrlen(skb));
		skb_pull(skb, 40); //IPv6 header len = 40 bytes
#endif		
	
		if(ip6_rtk_trap(skb)==RTK_TRAP_NONE){
			/* not RTP packet, return to netif_rx */
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
			skb_push(skb, 40);
#endif
			return RTK_TRAP_NONE;
		}else
			return RTK_TRAP_ACCEPT;
	
	}else{
		/* not ipv4 and ipv6 */
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
		skb_push(skb, ip_hdrlen(skb));
#endif
		return RTK_TRAP_NONE;
	}
}


/* ipv4 RTP session parse */
int rtk_trap(struct sk_buff *skb)
{
	struct RTK_TRAP_profile *ptr;
	//struct preprocess_result *rst;
	//struct net_device *rx_dev;

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	struct udphdr *uh;
	struct iphdr *iph;
#endif
	uint32 src_ip;
	uint32 dst_ip;
	uint16 src_port;
	uint16 dst_port;
	uint8 proto;
	//Tuint16 tmp;
	


	//Update transport header to let later used udp_hdr() macro get correct pointer
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	skb_reset_transport_header(skb);
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	iph = ip_hdr(skb);
	proto = iph->protocol;
	src_ip = iph->saddr;
	dst_ip = iph->daddr;
#else
	  proto = (skb->nh.iph)->protocol;
	  src_ip = ((skb->nh.iph)->saddr);
	  dst_ip = ((skb->nh.iph)->daddr);
#endif
	if(proto!=IPPROTO_UDP)
		return RTK_TRAP_NONE;

	uh = udp_hdr(skb);
	src_port =(uint16) (uh->source);
	dst_port =(uint16) (uh->dest);



	ptr = header;
	while(ptr!=NULL) 
	  {

 		if(ptr->ipv6==1){
			//this ptr is for ipv6
		  ptr=ptr->next;
	   		 continue;
	    }
	
		/* port not match */
		if ( dst_port != ptr->udp_dst_port ) {
		    ptr=ptr->next;
	   		 continue;
	    }
		

		/* ip not match */
	   	if ( dst_ip != ptr->ip_dst_addr ){
	    ptr=ptr->next;
	    continue;
	  }
	  
	  	/* ip and port match */
	  ptr ->rx_packets ++;
  #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	  ptr ->rx_bytes += (iph->tot_len) - 28;	/* UDP payload is excluded */
  #else
	  ptr ->rx_bytes += ((skb->nh.iph)->tot_len) - 28;	/* UDP payload is excluded */
  #endif
		  
#ifndef AUDIOCODES_VOIP
#ifdef SUPPORT_RTCP
	//extern int rtcp_sid_offset;
	if((ptr->s_id) >= RTCP_SID_OFFSET)
	{
		// This is RTCP packet
		// Must keep this sid check, because RTCP sid > MAX_DSP_RTK_SS_NUM
	}
	else
#endif
	{
  #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
		nRxRtpStatsCountByte[ ptr->s_id ] += (iph->tot_len) - 28;	/* UDP payload is excluded */
  #else
		nRxRtpStatsCountByte[ ptr->s_id ] += ((skb->nh.iph)->tot_len) - 28;	/* UDP payload is excluded */
  #endif
		nRxRtpStatsCountPacket[ ptr->s_id ] ++;
	}
#ifdef SUPPORT_VOIP_DBG_COUNTER
	if (gVoipCounterEnable)
		RTP_rx_count(ptr->s_id);
#endif
#endif // #ifndef AUDIOCODES_VOIP
	  
	  //PRINT_MSG("RTK TRAP catch one packet!!\n");
	  //rst = (struct preprocess_result *)preprocess(skb);
	  //PRINT_MSG("CH = %x\n");
	  //ptr->rtk_trap_callback(CH, 0, rst->ptr_data, rst->data_len, 0);
#if 1
	  /* thlin: use sid as input to call-back function, so the call-back function DSP_pktRx() in dsp_main.c gets sid instead of mid.*/
#ifdef VOIP_RX_IN_UDP	  
  #ifdef CONFIG_RTK_VOIP_SRTP
	  void *srtp_hdr = skb->data + sizeof(struct udphdr);
    #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	  int pkt_octet_len = (iph->tot_len) - 28;
    #else
	  int pkt_octet_len = ((skb->nh.iph)->tot_len) - 28;
    #endif
    #ifdef FEATURE_COP3_PROFILE	  
	  unsigned long flags;
	  save_flags(flags); cli();
	  ProfileEnterPoint(PROFILE_INDEX_TEMP0);
    #endif	  
	  /* apply srtp */
	  if (ptr->applySRTP){
    	#ifndef AUDIOCODES_VOIP
          	//extern int rtcp_sid_offset;
          	//if((dst_port == 9001) || (dst_port == 9003) || (dst_port == 9005) || (dst_port == 9007))
          	if(ptr->s_id >= RTCP_SID_OFFSET)
	#else
		if(((ptr->s_id)%2) == 1 ) // ACMW RTP sid = 2*CH, RTCP sid = 2*CH + 1
	#endif
          		status = srtp_unprotect_rtcp(ptr->rx_srtp_ctx,srtp_hdr,&pkt_octet_len);
		else
			status = srtp_unprotect(ptr->rx_srtp_ctx, srtp_hdr, &pkt_octet_len);
	  }
    #ifdef FEATURE_COP3_PROFILE	  
	  ProfileExitPoint(PROFILE_INDEX_TEMP0);
	  restore_flags(flags);
	  ProfilePerDump(PROFILE_INDEX_TEMP0, 1000);
    #endif	  
	  if (status) {
	  	PRINT_MSG(":src:%d-dest:%d\n",src_port,dst_port);
	    PRINT_MSG("error: srtp unprotection failed with code %d%s\n", status,
		    status == err_status_replay_fail ? " (replay check failed)" :
		    status == err_status_auth_fail ? " (auth check failed)" : "");	  
	  } else {	
    #if ! defined (AUDIOCODES_VOIP)		
		if( !rtk_trap_callback_trap( ptr, skb ) ) //  ??? bad packet ptr/size??
		#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	 		ptr->rtk_trap_callback(ptr->c_id, ptr->s_id, srtp_hdr, pkt_octet_len, iph->ttl);
		#else
	 		ptr->rtk_trap_callback(ptr->c_id, ptr->s_id, srtp_hdr, pkt_octet_len, skb->nh.iph->ttl);
		#endif
    #else
		if( !rtk_trap_callback_trap( ptr, skb ) ) // ??? bad packet ptr/size??
		#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
			ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, srtp_hdr, pkt_octet_len, iph->ttl); //TODO yg
		#else
			ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, srtp_hdr, pkt_octet_len, skb->nh.iph->ttl); //TODO yg
		#endif
    #endif
	  }
  #else // !CONFIG_RTK_VOIP_SRTP
    #if ! defined (AUDIOCODES_VOIP)
	  if( !rtk_trap_callback_trap( ptr, skb ) ) //check for T.38 stand alone
      #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->s_id, skb->data + sizeof(struct udphdr), (iph->tot_len) - 28, iph->ttl);
      #else
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->s_id, skb->data + sizeof(struct udphdr), ((skb->nh.iph)->tot_len) - 28, skb->nh.iph->ttl);
      #endif
    #else
	  if( !rtk_trap_callback_trap( ptr, skb ) ) //check for T.38 stand alone
      #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, skb->data + sizeof(struct udphdr), (iph->tot_len) - 28, iph->ttl); //TODO yg
      #else
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, skb->data + sizeof(struct udphdr), ((skb->nh.iph)->tot_len) - 28, skb->nh.iph->ttl); //TODO yg
      #endif
    #endif
  #endif // CONFIG_RTK_VOIP_SRTP
#else // !VOIP_RX_IN_UDP
    #if ! defined (AUDIOCODES_VOIP)
	  if( !rtk_trap_callback_trap( ptr, skb ) )  ??? bad packet ptr/size??
      #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	    ptr->rtk_trap_callback(ptr->c_id, ptr->s_id, skb->data + 28, (iph->tot_len) - 28, iph->ttl);
      #else
	    ptr->rtk_trap_callback(ptr->c_id, ptr->s_id, skb->data + 28, ((skb->nh.iph)->tot_len) - 28, skb->nh.iph->ttl);
      #endif
    #else
	  if( !rtk_trap_callback_trap( ptr, skb ) )  ??? bad packet ptr/size??
      #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, skb->data + 28, (iph->tot_len) - 28, iph->ttl); //TODO yg
      #else
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, skb->data + 28, ((skb->nh.iph)->tot_len) - 28, skb->nh.iph->ttl); //TODO yg
      #endif
    #endif
#endif // VOIP_RX_IN_UDP	  
#else
	  if( !rtk_trap_callback_trap( ptr, skb ) )  ??? bad packet ptr/size??
    #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, skb->data + 28, (iph->tot_len) - 28, iph->ttl);
    #else
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, skb->data + 28, ((skb->nh.iph)->tot_len) - 28, skb->nh.iph->ttl);
    #endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)
	  skb_unlink(skb);
#endif
	
	  kfree_skb(skb);
	  return RTK_TRAP_ACCEPT;
	}
	
	/* nomatch return none */
	return RTK_TRAP_NONE;	
}	

#endif //#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT

int ip6_rtk_trap(struct sk_buff *skb)
{
	struct RTK_TRAP_profile *ptr;

	struct udphdr *uh;
	struct ipv6hdr *iph;
	
	struct in6_addr src_ip;
	struct in6_addr dst_ip;
	uint16 src_port;
	uint16 dst_port;
	uint8 proto;

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	//Update transport header to let later used udp_hdr() macro get correct pointer
	skb_reset_transport_header(skb);
#endif

	//PRINT_MSG("skb enter rtk_trap= %x\n",skb);
#ifndef VOIP_RX_IN_UDP
	skb->mac.ethernet=((void *)skb->data -14);
#endif	
	//rx_dev = skb->dev;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
    iph = ipv6_hdr(skb);
#else
	iph = skb->nh.ipv6h;
#endif

	proto = iph->nexthdr;
	src_ip = iph->saddr;
	dst_ip = iph->daddr;


	if(proto!=IPPROTO_UDP)
		return RTK_TRAP_NONE;


	uh = udp_hdr(skb);
	src_port =(uint16) (uh->source);
	dst_port =(uint16) (uh->dest);
	

	ptr = header;
	while(ptr!=NULL) 
	{
	  //PRINT_MSG("packet enter filter\n");
		  if(ptr->ipv6!=1){
		  	//this ptr is for ipv4
			ptr=ptr->next;
			   continue;
		  }
		  


#ifndef RTP_SRC_IP_CHECK
	  	if ((!ipv6_addr_equal(&dst_ip, &ptr->ip6_dst_addr)) ||
	       proto != ptr->protocol
	     )
#else
	  if ( (!ipv6_addr_equal(&src_ip, &ptr->ip6_src_addr)) ||
	       (!ipv6_addr_equal(&dst_ip, &ptr->ip6_dst_addr)) ||
	       proto != ptr->protocol
	     )
#endif
	  {
	    //PRINT_MSG("src_ip = %x\n",src_ip);
	    //PRINT_MSG("dst_ip = %x\n",dst_ip);
	    //PRINT_MSG("proto = %x\n",proto);
	   // PRINT_Y("%d, %d\n", proto, ptr->protocol);
	    
#if 0
	    int i;

		for (i=0; i<8; i++)
			printk("0x%x, ", dst_ip.s6_addr16[i]);
		printk("\n");
	    
	    for (i=0; i<8; i++)
			printk("0x%x, ", ptr->ip6_dst_addr.s6_addr16[i]);
		printk("\n");
		printk("\n");
#endif
	    
	    //PRINT_R("src_ip/dst_ip not equal.\n");
	    ptr=ptr->next;
	    continue;
	  }


#ifndef RTP_SRC_PORT_CHECK
	  if ( dst_port != ptr->udp_dst_port )
#else /*RTP_SRC_PORT_CHECK*/
	  if ( src_port != ptr->udp_src_port ||
	       dst_port != ptr->udp_dst_port 
	     )
#endif /*RTP_SRC_PORT_CHECK*/
	  {
	    //PRINT_MSG("src_port = %x\n",src_port);
	    //PRINT_MSG("dst_port = %x\n",dst_port);
	    //PRINT_MSG("src_port = %x\n",ptr->udp_src_port);
	    //PRINT_R("src: %d, %d\n", src_port, ptr->udp_src_port);
	    //PRINT_R("dst: %d, %d\n", dst_port, ptr->udp_dst_port);
	    ptr=ptr->next;
	    continue;
	  }
	  
	  ptr ->rx_packets ++;
	  ptr ->rx_bytes += (iph->payload_len) - 8;	/* UDP payload is excluded */
		  
#ifndef AUDIOCODES_VOIP
#ifdef SUPPORT_RTCP
	//extern int rtcp_sid_offset;
	if((ptr->s_id) >= RTCP_SID_OFFSET)
	{
		// This is RTCP packet
		// Must keep this sid check, because RTCP sid > MAX_DSP_RTK_SS_NUM
	}
	else
#endif
	{
		nRxRtpStatsCountByte[ ptr->s_id ] += (iph->payload_len) - 8;	/* UDP payload is excluded */
		nRxRtpStatsCountPacket[ ptr->s_id ] ++;
	}
#ifdef SUPPORT_VOIP_DBG_COUNTER
	if (gVoipCounterEnable)
		RTP_rx_count(ptr->s_id);
#endif
#endif // #ifndef AUDIOCODES_VOIP
	  
	  //PRINT_MSG("RTK TRAP catch one packet!!\n");
	  //rst = (struct preprocess_result *)preprocess(skb);
	  //PRINT_MSG("CH = %x\n");
	  //ptr->rtk_trap_callback(CH, 0, rst->ptr_data, rst->data_len, 0);
#if 1
	  /* thlin: use sid as input to call-back function, so the call-back function DSP_pktRx() in dsp_main.c gets sid instead of mid.*/
#ifdef VOIP_RX_IN_UDP	  
  #ifdef CONFIG_RTK_VOIP_SRTP
	  void *srtp_hdr = skb->data + sizeof(struct udphdr);
    #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	  int pkt_octet_len = (iph->tot_len) - 28;
    #else
	  int pkt_octet_len = ((skb->nh.iph)->tot_len) - 28;
    #endif
    #ifdef FEATURE_COP3_PROFILE	  
	  unsigned long flags;
	  save_flags(flags); cli();
	  ProfileEnterPoint(PROFILE_INDEX_TEMP0);
    #endif	  
	  /* apply srtp */
	  if (ptr->applySRTP){
    	#ifndef AUDIOCODES_VOIP
          	//extern int rtcp_sid_offset;
          	//if((dst_port == 9001) || (dst_port == 9003) || (dst_port == 9005) || (dst_port == 9007))
          	if(ptr->s_id >= RTCP_SID_OFFSET)
	#else
		if(((ptr->s_id)%2) == 1 ) // ACMW RTP sid = 2*CH, RTCP sid = 2*CH + 1
	#endif
          		status = srtp_unprotect_rtcp(ptr->rx_srtp_ctx,srtp_hdr,&pkt_octet_len);
		else
			status = srtp_unprotect(ptr->rx_srtp_ctx, srtp_hdr, &pkt_octet_len);
	  }
    #ifdef FEATURE_COP3_PROFILE	  
	  ProfileExitPoint(PROFILE_INDEX_TEMP0);
	  restore_flags(flags);
	  ProfilePerDump(PROFILE_INDEX_TEMP0, 1000);
    #endif	  
	  if (status) {
	  	PRINT_MSG(":src:%d-dest:%d\n",src_port,dst_port);
	    PRINT_MSG("error: srtp unprotection failed with code %d%s\n", status,
		    status == err_status_replay_fail ? " (replay check failed)" :
		    status == err_status_auth_fail ? " (auth check failed)" : "");	  
	  } else {	
    #if ! defined (AUDIOCODES_VOIP)		
		if( !rtk_trap_callback_trap( ptr, skb ) ) //  ??? bad packet ptr/size??
	 		ptr->rtk_trap_callback(ptr->c_id, ptr->s_id, srtp_hdr, pkt_octet_len, iph->ttl);
    #else
		if( !rtk_trap_callback_trap( ptr, skb ) ) // ??? bad packet ptr/size??
			ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, srtp_hdr, pkt_octet_len, iph->ttl); //TODO yg
    #endif
	  }
  #else // !CONFIG_RTK_VOIP_SRTP
    #if ! defined (AUDIOCODES_VOIP)
	  if( !rtk_trap_callback_trap( ptr, skb ) ) //check for T.38 stand alone
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->s_id, skb->data + sizeof(struct udphdr), (iph->payload_len) - 8, iph->hop_limit);
    #else
	  if( !rtk_trap_callback_trap( ptr, skb ) ) //check for T.38 stand alone
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, skb->data + sizeof(struct udphdr), (iph->payload_len) - 8, iph->hop_limit); //TODO yg
    #endif
  #endif // CONFIG_RTK_VOIP_SRTP
#else // !VOIP_RX_IN_UDP
    #if ! defined (AUDIOCODES_VOIP)
	  if( !rtk_trap_callback_trap( ptr, skb ) )  ??? bad packet ptr/size??
	    ptr->rtk_trap_callback(ptr->c_id, ptr->s_id, skb->data + 28, (iph->tot_len) - 28, iph->ttl);
    #else
	  if( !rtk_trap_callback_trap( ptr, skb ) )  ??? bad packet ptr/size??
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, skb->data + 28, (iph->tot_len) - 28, iph->ttl); //TODO yg
    #endif
#endif // VOIP_RX_IN_UDP	  
#else
	  if( !rtk_trap_callback_trap( ptr, skb ) )  ??? bad packet ptr/size??
    #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, skb->data + 28, (iph->tot_len) - 28, iph->ttl);
    #else
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, skb->data + 28, ((skb->nh.iph)->tot_len) - 28, skb->nh.iph->ttl);
    #endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)
	  skb_unlink(skb);
#endif
	  kfree_skb(skb);
	  //dev_put(rx_dev);
	  return RTK_TRAP_ACCEPT;
	}

	return RTK_TRAP_NONE;	
}	
#endif //CONFIG_RTK_VOIP_IPV6_SUPPORT
#endif //#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP

#ifndef AUDIOCODES_VOIP
void ResetRtpStatsCount( uint32 sid )
{
	extern uint32 nTxRtpStatsCountByte[MAX_DSP_RTK_SS_NUM];
	extern uint32 nTxRtpStatsCountPacket[MAX_DSP_RTK_SS_NUM];
	
	if( sid >= DSP_RTK_SS_NUM )
		return;

	/* Rx RTP statistics */
	nRxRtpStatsCountByte[ sid ] = 0;
	nRxRtpStatsCountPacket[ sid ] = 0;

	/* Tx RTP statistics */
	nTxRtpStatsCountByte[ sid ] = 0;
	nTxRtpStatsCountPacket[ sid ] = 0;

	/* Rx RTP lost */
	nRxRtpStatsLostPacket[ sid ] = 0;
}
#endif

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
#ifdef CONFIG_RTK_VOIP_RX_NETFILTER_HOOK

static unsigned int voip_rx_netfilter_local_in_hook( unsigned int hooknum, struct sk_buff *skb,
        const struct net_device *in, const struct net_device *out,
        int(*okfn)( struct sk_buff * ) )
{
    struct ethhdr *eth = eth_hdr(skb); /* Get a pointer to the Ethernet header */
#if 0
    u_int16_t etype;
#endif

    //printk( "%s: Received a packet %p, device in = %s\n", __func__,
    //            skb, in ? in->name : "<NONE>" );
    
    //printk( "%s\n", in ? in->name : "<NONE>" );



#ifdef CONFIG_RTK_VOIP_QOS
	if(voip_rx_shortcut_enable)
		  return NF_ACCEPT;
#endif


#if 0
    /* Is this a multicast packet ? */
    if( is_multicast_ether_addr(eth->h_dest) )
    {
        /* Do something */
        printk( "Packet is multicast\n" );
    }

    if( is_broadcast_ether_addr(eth->h_dest) )
    {
        /* Do something else */
        printk( "Packet is broadcast\n");
    }
#endif

#if 0
    /* Get EtherType field */
    etype = ntohs( eth->h_proto );

    if( etype == ETH_P_IP ) //this will block the ppp packets
#endif
    {
        struct iphdr *ip = NULL;

        /* This is an IP packet */
        ip = ip_hdr(skb);
        if (ip == NULL)
        {
            return NF_ACCEPT;
        }

        if (ip->protocol == IPPROTO_UDP)
        {
#if 1

#if 1
			// skip check for udp_rtk_trap_profile_header and udp_rtk_trap_hook,
			// they are init before netfilter hook register
			if( *udp_rtk_trap_profile_header != NULL )
#else
			if( udp_rtk_trap_profile_header != NULL
				&& *udp_rtk_trap_profile_header != NULL
				&& udp_rtk_trap_hook != NULL)
#endif
			{
				if ( udp_rtk_trap_hook(skb) == 0)
					return NF_STOLEN;
			}
#else
			// Testing
			struct udphdr *udp = NULL;

            /* UDP packet */
            udp = (struct udphdr *)(skb_network_header(skb) + ip_hdrlen(skb));
            
            //printk("%d, %d\n", udp->source, udp->dest);
            
            unsigned char* pl;
            //pl = udp + sizeof(struct udphdr) -2;
            pl = skb->data; // point to IP header
            pl += ip_hdrlen(skb);
            printk("%x %x %x %x ", pl[0], pl[1], pl[2], pl[3]);
            pl += sizeof(struct udphdr);
            printk("%x %x %x %x\n", pl[0], pl[1], pl[2], pl[3]);

            /* Do something here */
            //printk( "UDP packet\n");
#endif
        }
#if 0
        else if (ip->protocol == IPPROTO_TCP)
        {
			struct tcphdr *tcp = NULL;

            /* TCP packet */
            tcp = (struct tcphdr *)(skb_network_header(skb) + ip_hdrlen(skb));

            /* Do something here */
            printk( "TCP packet\n");
        }
#endif
    }

   /* We might decide to drop this packet, so we need to:
      * return NF_DROP;
      *
      * If we want to handle this packet ourselves, we need to:
      * return NF_STOLEN;
      */

    return NF_ACCEPT;
}


#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
static unsigned int voip_rx_netfilter_local_in_hook_ip6( unsigned int hooknum, struct sk_buff *skb,
        const struct net_device *in, const struct net_device *out,
        int(*okfn)( struct sk_buff * ) )
{
    struct ethhdr *eth = eth_hdr(skb); /* Get a pointer to the Ethernet header */
#if 0
    u_int16_t etype;
#endif

#ifdef CONFIG_RTK_VOIP_QOS
		if(voip_rx_shortcut_enable)
			  return NF_ACCEPT;
#endif

    //printk( "%s: Received a packet %p, device in = %s\n", __func__,
      //          skb, in ? in->name : "<NONE>" );
    
    //printk( "%s\n", in ? in->name : "<NONE>" );

#if 0
    /* Is this a multicast packet ? */
    if( is_multicast_ether_addr(eth->h_dest) )
    {
        /* Do something */
        printk( "Packet is multicast\n" );
    }

    if( is_broadcast_ether_addr(eth->h_dest) )
    {
        /* Do something else */
        printk( "Packet is broadcast\n");
    }
#endif

#if 0
    /* Get EtherType field */
    etype = ntohs( eth->h_proto );

	//printk("eth proto = 0x%x\n", etype);

    if( etype == ETH_P_IPV6 )	//this will block ppp packets
#endif
    {
        struct ipv6hdr *ip = NULL;
	
		//printk("Packet is IPv6 eth protocal\n");

        /* This is an IP packet */
        ip = ipv6_hdr(skb);
        if (ip == NULL)
        {
			printk("ip is NULL\n");
            return NF_ACCEPT;
        }

#if 0
		int i;
		printk("saddr = ");
		for (i=0; i<8; i++)
			printk("0x%x, ", ip->saddr.s6_addr16[i]);
		printk("\n");
		
		printk("daddr = ");
		for (i=0; i<8; i++)
			printk("0x%x, ", ip->daddr.s6_addr16[i]);
		printk("\n");
#endif

        if (ip->nexthdr == IPPROTO_UDP)
        {

			//printk("Receive UDP packet\n");
#if 1
			// skip check for udp_rtk_trap_profile_header and udp_rtk_trap_hook,
			// they are init before netfilter hook register
			if( *udp_rtk_trap_profile_header != NULL )
#else
			if( udp_rtk_trap_profile_header != NULL
				&& *udp_rtk_trap_profile_header != NULL
				&& ip6_udp_rtk_trap_hook != NULL)
#endif
			{
				if ( ip6_udp_rtk_trap_hook(skb) == 0)
					return NF_STOLEN;
			}
        }
#if 0		
		else if (ip->nexthdr == IPPROTO_TCP)
		{
			printk("Receive TCP packet\n");
		}
		else if (ip->nexthdr == IPPROTO_ICMPV6)
		{
			printk("Receive ICMPv6 packet\n");
		}
#endif
    }

   /* We might decide to drop this packet, so we need to:
      * return NF_DROP;
      *
      * If we want to handle this packet ourselves, we need to:
      * return NF_STOLEN;
      */

    return NF_ACCEPT;
}
#endif

static struct nf_hook_ops voip_rx_hook_ops __read_mostly =
{
	.pf = PF_INET,
	.priority = 1,
	.hooknum = NF_INET_LOCAL_IN,
	.hook = voip_rx_netfilter_local_in_hook,
	.owner = THIS_MODULE,
};

#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
static struct nf_hook_ops voip_rx_hook_ip6_ops __read_mostly =
{
	.pf = PF_INET6,
	.priority = 1,
	.hooknum = NF_INET_LOCAL_IN,
	.hook = voip_rx_netfilter_local_in_hook_ip6,
	.owner = THIS_MODULE,
};

#endif

int voip_rx_netfilter_init( void )
{
    int ret;

    /* Register netfilter hooks */
    ret = nf_register_hook(&voip_rx_hook_ops);
    if (ret < 0)
	{
    	PRINT_R("VoIP RX netfilter hook init fail!\n");
    	return ret;
	}
    else
    	printk("VoIP RX netfilter hook init done!\n");
	

#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
    ret = nf_register_hook(&voip_rx_hook_ip6_ops);
    if (ret < 0)
	{
    	PRINT_R("VoIP RX netfilter hook for IPv6 init fail!\n");
    	return ret;
	}
    else
    	printk("VoIP RX netfilter hook for IPv6 init done!\n");
#endif

    return ret;
}

int voip_rx_netfilter_exit( void )
{
    /* Unregister hooks */
    nf_unregister_hook(&voip_rx_hook_ops);
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
    nf_unregister_hook(&voip_rx_hook_ip6_ops);
#endif
	return 0;
}

//voip_initcall_proc( voip_rx_netfilter_init );
//voip_exitcall( voip_rx_netfilter_exit );

#endif //CONFIG_RTK_VOIP_RX_NETFILTER_HOOK
#endif //CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP

int __init rtk_trap_init_module(void)
{
#ifndef AUDIOCODES_VOIP
#if !defined( CONFIG_RTK_VOIP_IPC_ARCH ) || defined( CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP )
	extern void ResetSessionRxStatistics( uint32 sid );
	extern void ResetSessionTxStatistics( uint32 sid );
#endif
#endif

	int i;
	
#if 0
	PRINT_MSG("============= RTK VoIP SUITE ============\n");	
	PRINT_MSG("INITIAL RTP TRAP\n");
	PRINT_MSG("=========================================\n");    	
#endif
	//PRINT_MSG("1.rtk_trap_hook = %x\n", rtk_trap_hook);
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	rtk_trap_hook = rtk_trap;
#endif
	header = NULL;

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	udp_rtk_trap_hook = rtk_rtp_rx;
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
	ip6_udp_rtk_trap_hook = ip6_rtk_trap;
#endif
#endif
	udp_rtk_trap_profile_header = (void*)&header;	
	
	//PRINT_MSG("2. rtk_trap_hook = %x\n", rtk_trap_hook);

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
#ifdef CONFIG_RTK_VOIP_RX_NETFILTER_HOOK
	voip_rx_netfilter_init();
#endif
#endif

#ifdef CONFIG_RTK_VOIP_SRTP

#ifdef CONFIG_RTK_VOIP_SRTP_HW
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
#ifndef CONFIG_IPSEC
	rtl8186_crypto_init(); 	
#endif	
#endif
#endif // CONFIG_RTK_VOIP_SRTP_HW

  	/* initialize srtp library */
  	status = srtp_init();
  	if (status) {
    		PRINT_MSG("error: srtp initialization failed with error code %d\n", status);
	}	
#endif	

#ifndef AUDIOCODES_VOIP
	for( i = 0; i < DSP_RTK_SS_NUM; i ++ )
		ResetRtpStatsCount( i );
#endif

#ifndef AUDIOCODES_VOIP
#if !defined( CONFIG_RTK_VOIP_IPC_ARCH ) || defined( CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP )
	for( i = 0; i < DSP_RTK_SS_NUM; i ++ ) {
		ResetSessionRxStatistics( i );
		ResetSessionTxStatistics( i );
	}
#endif
#endif
	
	return 0;
}	


void __exit rtk_trap_cleanup_module(void)
{
	rtk_trap_hook = NULL;
	header = NULL;
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
#ifdef CONFIG_RTK_VOIP_RX_NETFILTER_HOOK
	voip_rx_netfilter_exit();
#endif
#endif

	PRINT_MSG("============= RTK VoIP SUITE ============\n");	
        PRINT_MSG("Remove RTK TRAP\n");
        PRINT_MSG("=========================================\n");          

}

#ifndef CONFIG_RTK_VOIP_MODULE
voip_initcall(rtk_trap_init_module);
voip_exitcall(rtk_trap_cleanup_module);
#endif

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
int voip_trap_read_proc(struct seq_file *f, void *v)
{
	int n, i;

	struct RTK_TRAP_profile *ptr;

	seq_printf( f, "* VoIP-RX trap method 2: " );
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
#ifdef CONFIG_RTK_VOIP_RX_LINUX_UDP_TRAP
	seq_printf( f, "Linux kernel UDP Trap.\n\n" );
#elif defined (CONFIG_RTK_VOIP_RX_NETFILTER_HOOK)
#ifdef CONFIG_RTK_VOIP_QOS
//	extern int voip_shortcut_enable;
	if( voip_rx_shortcut_enable )
		seq_printf( f, "VoIP Short Cut.(RX)\n\n" );
	else
		seq_printf( f, "Netfilter Local In Hook.\n\n" );
	seq_printf( f, "VoIP TX short cut is %d\n\n", voip_tx_shortcut_enable);
#else
	seq_printf( f, "Netfilter Local In Hook.\n\n" );
#endif
#else
	seq_printf( f, "??\n\n" );
#endif	
#else
	seq_printf( f, "IPC RX\n\n" );
#endif
	ptr = header;

#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
	seq_printf( f, "* IP Protocal: ");
	if (ptr)
	{
		if (ptr->ipv6 == 1)
			seq_printf( f, "* IPv6\n");
		else if (ptr->ipv6 == 0)
			seq_printf( f, "* IPv4\n");
		else
			seq_printf( f, "* Unknown (error!) ip type=%d\n" , ptr->ipv6);
	}
	else
		seq_printf( f, "\n");
#endif

	seq_printf( f, "src 		dst proto	cid mid sid rx		tx\n" );
	
	
	while( ptr ) {
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
		if (ptr->ipv6 == 1)
		{
			for (i=0; i<8; i++)
				seq_printf( f, "0x%x:", ptr->ip6_src_addr.s6_addr16[i]);
			seq_printf( f, "port %u", ptr ->udp_src_port);
			seq_printf( f, "\t");
			
			for (i=0; i<8; i++)
				seq_printf( f, "0x%x:", ptr->ip6_dst_addr.s6_addr16[i]);
			seq_printf( f, "port %u", ptr ->udp_dst_port);
			seq_printf( f, "\t");
		}
		else
#endif
		{
			seq_printf( f, "%u.%u.%u.%u:%u\t", 
									( ptr ->ip_src_addr >> 24 ) & 0xFF, 
									( ptr ->ip_src_addr >> 16 ) & 0xFF, 
									( ptr ->ip_src_addr >> 8  ) & 0xFF, 
									( ptr ->ip_src_addr >> 0  ) & 0xFF, 
									ptr ->udp_src_port );
			seq_printf( f, "%u.%u.%u.%u:%u\t", 
									( ptr ->ip_dst_addr >> 24 ) & 0xFF,  
									( ptr ->ip_dst_addr >> 16 ) & 0xFF,  
									( ptr ->ip_dst_addr >> 8  ) & 0xFF,  
									( ptr ->ip_dst_addr >> 0  ) & 0xFF,  
									ptr ->udp_dst_port );
		}
		seq_printf( f, "%u\t", ptr ->protocol );
		seq_printf( f, "%u\t%u\t%u\t", ptr ->c_id, ptr ->m_id, ptr ->s_id );
		seq_printf( f, "%u(%u)\t%u(%u)\n", 
								ptr ->rx_bytes, ptr ->rx_packets, 
								ptr ->tx_bytes, ptr ->tx_packets );
	
		ptr = ptr ->next;
	}

	return n;
}
#else
int voip_trap_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int n, i;

	struct RTK_TRAP_profile *ptr;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}

	n = sprintf( buf, "* VoIP-RX trap method 2: " );
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
#ifdef CONFIG_RTK_VOIP_RX_LINUX_UDP_TRAP
	n += sprintf( buf + n, "Linux kernel UDP Trap.\n\n" );
#elif defined (CONFIG_RTK_VOIP_RX_NETFILTER_HOOK)
#ifdef CONFIG_RTK_VOIP_QOS
//	extern int voip_shortcut_enable;
	if( voip_rx_shortcut_enable )
		n += sprintf( buf + n, "VoIP Short Cut.(RX)\n\n" );
	else
		n += sprintf( buf + n, "Netfilter Local In Hook.\n\n" );
	n += sprintf( buf + n, "VoIP TX short cut is %d\n\n", voip_tx_shortcut_enable);
#else
	n += sprintf( buf + n, "Netfilter Local In Hook.\n\n" );
#endif
#else
	n += sprintf( buf + n, "??\n\n" );
#endif	
#else
	n += sprintf( buf + n, "IPC RX\n\n" );
#endif
	ptr = header;

#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
	n += sprintf( buf + n, "* IP Protocal: ");
	if (ptr)
	{
		if (ptr->ipv6 == 1)
			n += sprintf( buf + n, "* IPv6\n");
		else if (ptr->ipv6 == 0)
			n += sprintf( buf + n, "* IPv4\n");
		else
			n += sprintf( buf + n, "* Unknown (error!) ip type=%d\n" , ptr->ipv6);
	}
	else
		n += sprintf( buf + n, "\n");
#endif

	n += sprintf( buf + n, "src			dst	proto	cid	mid	sid	rx		tx\n" );
	
	
	while( ptr ) {
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
		if (ptr->ipv6 == 1)
		{
			for (i=0; i<8; i++)
				n += sprintf( buf + n, "0x%x:", ptr->ip6_src_addr.s6_addr16[i]);
			n += sprintf( buf + n, "port %u", ptr ->udp_src_port);
			n += sprintf( buf + n, "\t");
			
			for (i=0; i<8; i++)
				n += sprintf( buf + n, "0x%x:", ptr->ip6_dst_addr.s6_addr16[i]);
			n += sprintf( buf + n, "port %u", ptr ->udp_dst_port);
			n += sprintf( buf + n, "\t");
		}
		else
#endif
		{
			n += sprintf( buf + n, "%u.%u.%u.%u:%u\t", 
									( ptr ->ip_src_addr >> 24 ) & 0xFF, 
									( ptr ->ip_src_addr >> 16 ) & 0xFF, 
									( ptr ->ip_src_addr >> 8  ) & 0xFF, 
									( ptr ->ip_src_addr >> 0  ) & 0xFF, 
									ptr ->udp_src_port );
			n += sprintf( buf + n, "%u.%u.%u.%u:%u\t", 
									( ptr ->ip_dst_addr >> 24 ) & 0xFF,  
									( ptr ->ip_dst_addr >> 16 ) & 0xFF,  
									( ptr ->ip_dst_addr >> 8  ) & 0xFF,  
									( ptr ->ip_dst_addr >> 0  ) & 0xFF,  
									ptr ->udp_dst_port );
		}
		n += sprintf( buf + n, "%u\t", ptr ->protocol );
		n += sprintf( buf + n, "%u\t%u\t%u\t", ptr ->c_id, ptr ->m_id, ptr ->s_id );
		n += sprintf( buf + n, "%u(%u)\t%u(%u)\n", 
								ptr ->rx_bytes, ptr ->rx_packets, 
								ptr ->tx_bytes, ptr ->tx_packets );
	
		ptr = ptr ->next;
	}
	
	*eof = 1;
	return n;
}
#endif

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
int voip_rtpstats_read_proc(struct seq_file *f, void *v)
{
#ifndef AUDIOCODES_VOIP
	extern uint32 nTxRtpStatsCountByte[MAX_DSP_RTK_SS_NUM];
	extern uint32 nTxRtpStatsCountPacket[MAX_DSP_RTK_SS_NUM];

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern RtpSessionState sessionState[MAX_DSP_RTK_SS_NUM];
	
	int SessNum;
	int sid;
	int i = 0;
#endif

	//int ch;
	int ss;
	int n = 0;
	
	if( IS_CH_PROC_DATA( v ) ) {
		//ch = CH_FROM_PROC_DATA( data );
		//n = sprintf( buf, "channel=%d\n", ch );
	} else {
		ss = SS_FROM_PROC_DATA( v );
		seq_printf( f, "session=%d\n", ss );
		/* Rx RTP statistics */
		seq_printf( f, "RxByte: %u\n", nRxRtpStatsCountByte[ ss ] );
		seq_printf( f, "RxPacket: %u\n", nRxRtpStatsCountPacket[ ss ] );
		
		/* Tx RTP statistics */
		seq_printf( f, "TxByte: %u\n", nTxRtpStatsCountByte[ ss ] );
		seq_printf( f, "TxPacket: %u\n", nTxRtpStatsCountPacket[ ss ] );
		
		/* Rx RTP lost */
		seq_printf( f, "RxLostPacket: %u\n", nRxRtpStatsLostPacket[ ss ] );
		
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
		if (sessionState[ss] == rtp_session_sendrecv)
			seq_printf( f, "RTP session state: send_receive\n");
		else if (sessionState[ss] == rtp_session_recvonly)
			seq_printf( f, "RTP session state: receive only\n");
		else if (sessionState[ss] == rtp_session_sendonly)
			seq_printf( f, "RTP session state: send only\n");
		else if (sessionState[ss] == rtp_session_inactive)
			seq_printf( f, "RTP session state: inactive\n");
#endif

	}
	
	return n;
#else
	return 0;
#endif
}
#else
int voip_rtpstats_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
#ifndef AUDIOCODES_VOIP
	extern uint32 nTxRtpStatsCountByte[MAX_DSP_RTK_SS_NUM];
	extern uint32 nTxRtpStatsCountPacket[MAX_DSP_RTK_SS_NUM];

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern RtpSessionState sessionState[MAX_DSP_RTK_SS_NUM];
	
	int SessNum;
	int sid;
	int i = 0;
#endif

	//int ch;
	int ss;
	int n = 0;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	if( IS_CH_PROC_DATA( data ) ) {
		//ch = CH_FROM_PROC_DATA( data );
		//n = sprintf( buf, "channel=%d\n", ch );
	} else {
		ss = SS_FROM_PROC_DATA( data );
		n = sprintf( buf, "session=%d\n", ss );
		/* Rx RTP statistics */
		n += sprintf( buf + n, "RxByte: %u\n", nRxRtpStatsCountByte[ ss ] );
		n += sprintf( buf + n, "RxPacket: %u\n", nRxRtpStatsCountPacket[ ss ] );
		
		/* Tx RTP statistics */
		n += sprintf( buf + n, "TxByte: %u\n", nTxRtpStatsCountByte[ ss ] );
		n += sprintf( buf + n, "TxPacket: %u\n", nTxRtpStatsCountPacket[ ss ] );
		
		/* Rx RTP lost */
		n += sprintf( buf + n, "RxLostPacket: %u\n", nRxRtpStatsLostPacket[ ss ] );
		
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
		if (sessionState[ss] == rtp_session_sendrecv)
			n += sprintf( buf + n, "RTP session state: send_receive\n");
		else if (sessionState[ss] == rtp_session_recvonly)
			n += sprintf( buf + n, "RTP session state: receive only\n");
		else if (sessionState[ss] == rtp_session_sendonly)
			n += sprintf( buf + n, "RTP session state: send only\n");
		else if (sessionState[ss] == rtp_session_inactive)
			n += sprintf( buf + n, "RTP session state: inactive\n");
#endif
	}
	

	*eof = 1;
	return n;
#else
	return 0;
#endif
}
#endif

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int trap_open(struct inode *inode, struct file *file)
{
	return single_open(file, voip_trap_read_proc, NULL);
}

struct file_operations proc_trap_fops = {
	.owner	= THIS_MODULE,
	.open	= trap_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
//read:   voip_trap_read_proc
};

static int rtpstats_open(struct inode *inode, struct file *file)
{
	return single_open(file, voip_rtpstats_read_proc, NULL);
}

struct file_operations proc_rtpstats_fops = {
	.owner	= THIS_MODULE,
	.open	= rtpstats_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
//read:   voip_rtpstats_read_proc
};
#endif


int __init voip_trap_proc_init( void )
{
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	proc_create_data( PROC_VOIP_DIR "/trap", 0, NULL, &proc_trap_fops, NULL );

	create_voip_session_proc_read_entry( "rtpstats", &proc_rtpstats_fops );
#else
	create_proc_read_entry( PROC_VOIP_DIR "/trap", 0, NULL, voip_trap_read_proc, NULL );
	
	create_voip_session_proc_read_entry( "rtpstats", voip_rtpstats_read_proc );
#endif
	
	return 0;
}

void __exit voip_trap_proc_exit( void )
{
	remove_voip_proc_entry( PROC_VOIP_DIR "/trap", NULL );
	
	remove_voip_channel_proc_entry( "rtpstats" );
}

voip_initcall_proc( voip_trap_proc_init );
voip_exitcall( voip_trap_proc_exit );


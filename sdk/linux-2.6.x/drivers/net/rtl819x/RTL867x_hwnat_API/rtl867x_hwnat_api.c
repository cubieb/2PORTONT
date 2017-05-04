#include <net/rtl/rtl867x_hwnat_api.h>
#ifdef CONFIG_RTL_8367B
#include "../../rtl8367b/port.h"
#endif

//#define DBG_HWACCAPI
#ifdef DBG_HWACCAPI
#define DBG_HWACCAPI_PRK printk
#else
#define DBG_HWACCAPI_PRK(format, args...)
#endif

//#define DBG_HWACCAPI_SuccessAdd
#ifdef DBG_HWACCAPI_SuccessAdd
#define DBG_HWACCAPI_SuccessAdd_PRK printk
#else
#define DBG_HWACCAPI_SuccessAdd_PRK(format, args...)
#endif

//#define DBG_HWACCAPI_SuccessDel
#ifdef DBG_HWACCAPI_SuccessDel
#define DBG_HWACCAPI_SuccessDel_PRK printk
#else
#define DBG_HWACCAPI_SuccessDel_PRK(format, args...)
#endif

//#define DBG_HWIPQOSAPI
#ifdef DBG_HWIPQOSAPI
#define DBG_HWIPQOSAPI_PRK printk
#else
#define DBG_HWIPQOSAPI_PRK(format, args...)
#endif

#ifdef CONFIG_RTL_HARDWARE_MULTICAST
/********************************************************         
	RTL8676 API  - Multicast hardware acceleration
***********************************************************/

/* 
 *  Function Name:		rtl8676_set_Mutlicast_acc
 *  Description:			Let ipv4 multicast stream accelerate in 8676 hwnat switch
 *
 *  Input:				group_ip : 			Ipv4 multicast group ip
 *					source_ip:			Ipv4 multicast source ip
 *					source_vid:			the vlan id of multicast packet (if none, pass 0)
 *					source_port:			the source port (in 8676 switch view, start from 0) of multicast packet 
 *					gourp_member:		the members which the multicast packet forward to (in 8676 switch view , bit0 : port0 , bit1 : port1 ....)
 *
 * Return:				SUCCESS:				set 8676hwnat successfully 
 *       				FAILED				set 8676hwnat failed
 */

int rtl8676_set_Mutlicast_acc(unsigned int group_ip, unsigned int source_ip, unsigned short source_vid, unsigned short source_port,unsigned int gourp_member)
{
	return rtl865x_addMulticastEntry(group_ip,source_ip,source_vid,source_port,gourp_member);
}

/* 
 *  Function Name:		rtl8676_reset_Mutlicast_acc
 *  Description:			change accelerated multicast stream's group member in 8676 hwnat switch
 *
 *  Input:				group_ip : 			Ipv4 multicast group ip
 *					gourp_member:		the members which the multicast packet forward to (in 8676 switch view , bit0 : port0 , bit1 : port1 ....)
 *
 * Return:				SUCCESS:				set 8676hwnat successfully 
 *       				FAILED				set 8676hwnat failed 
 */

int rtl8676_reset_Mutlicast_acc(unsigned int source_ip,unsigned int group_ip,unsigned int gourp_member)
{
	return rtl865x_resetMulticastEntry(source_ip,group_ip,gourp_member);
}

/* 
 *  Function Name:		rtl8676_cancel_Mutlicast_acc
 *  Description:			cancle accelerated multicast stream in 8676 hwnat switch
 *
 *  Input:				group_ip : 			Ipv4 multicast group ip
 * 
 * Return:				SUCCESS:				set 8676hwnat successfully 
 *       				FAILED				set 8676hwnat failed 
 */

int rtl8676_cancel_Mutlicast_acc(unsigned int source_ip,unsigned int group_ip)
{
	return rtl865x_delMulticastEntry(source_ip,group_ip);
}
#endif

#ifdef CONFIG_RTL_MULTI_ETH_WAN
/********************************************************         
	RTL8676 API  - Multiwan
***********************************************************/

/* 
 *	Function Name:	rtl8676_register_Multiwan_dev
 *	Description:	set netif to vlanconfig
 *
 *	Input:		ifname		:	network interface name
 *				proto		:	0: pppoe, 1: ipoe, 2: bridge
 *				vid			:	vlan id
 *				napt			:	0: not support napt, 1: support napt
 * 
 *	Return:		SUCCESS	:	set 8676hwnat successfully 
 * 				FAILED		:	set 8676hwnat failed 
 */

int rtl8676_register_Multiwan_dev(char *ifname, int proto, int vid, int napt)
{
	return rtl_set_wanport_vlanconfig(ifname, proto, vid, napt);
}

/* 
 *	Function Name:	rtl8676_update_portmapping_Multiwan_dev
 *	Description:	set group member of netif to vlanconfig
 *
 *	Input:		ifname		:	network interface name
 *				member 	:	network interface group member
 *
 *	Return:		SUCCESS	:	set 8676hwnat successfully 
 *				FAILED		:	set 8676hwnat failed 
 */

int rtl8676_update_portmapping_Multiwan_dev(char *ifname,unsigned int member)
{
	return rtl_set_wanport_portmapping(ifname,member);
}

/* 
 *	Function Name:	rtl8676_unregister_Multiwan_dev
 *	Description:	reset vlanconfig & netif/acl in 8676 hwnat switch
 *
 *	Input:		ifname		:	network interface name
 * 
 *	Return:		SUCCESS	:	set 8676hwnat successfully 
 *				FAILED		:	set 8676hwnat failed 
 */

int rtl8676_unregister_Multiwan_dev(char *ifname)
{	
	return rtl865x_unregisterDev(ifname);
}

/* 
 *	Function Name:	rtl8676_setNetifMacAddr
 *	Description:	update netif mac address in 8676 hwnat switch
 *
 *	Input:		ifname		:	network interface name
 *				addr			:	mac address
 *
 *	Return:		SUCCESS	:	set 8676hwnat successfully 
 *				FAILED		:	set 8676hwnat failed  
 */

int rtl8676_set_Multiwan_NetifMacAddr(char *ifname, char *addr)
{
	return rtl865x_setNetifMacAddr(ifname, addr);
}
#endif
#if defined( CONFIG_RTL_PPPOE_HWACC) && defined(CONFIG_RTL_LAYERED_DRIVER) && defined (CONFIG_RTL_LAYERED_DRIVER_L3)
/* 
 *	Function Name:	rtl8676_ppp_connect_channel
 *	Description:	attach ppp dev to master dev, and add ppp entry into 8676 hwnat switch
 *
 *	Input:		ppp_dev_name		:	ppp interface name
 *				sk 					:	ppp channel's socket
 *				SE_TYPE				:	connect type. 
 *
 *	Return:		none  
 */

void rtl8676_ppp_connect_channel(char* ppp_dev_name, struct sock* sk, int SE_TYPE)
{
	struct pppox_sock *po = pppox_sk(sk);
	struct net_device *local_dev = po->pppoe_dev;
	DBG_HWACCAPI_PRK("Enter %s (ppp_dev_name=%s)\n",__func__,ppp_dev_name);
					
	rtl865x_attachMasterNetif(ppp_dev_name, local_dev->name);
	rtl865x_addPpp(ppp_dev_name , (ether_addr_t*)po->pppoe_pa.remote, po->pppoe_pa.sid, SE_TYPE);
}
/* 
 *	Function Name:	rtl8676_ppp_disconnect_channel
 *	Description:	detach ppp device and delete entry in 8676 hwnat switch
 *
 *	Input:		ifname		:	ppp interface name
 *
 *	Return:		none  
 */

void rtl8676_ppp_disconnect_channel(char* ppp_dev_name)
{					
	DBG_HWACCAPI_PRK("Enter %s (ppp_dev_name=%s)\n",__func__,ppp_dev_name);
	rtl865x_detachMasterNetif(ppp_dev_name);					
	rtl865x_delPppbyIfName(ppp_dev_name);
	rtl865x_delNetif(ppp_dev_name);
}

#endif

#if defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL_LAYERED_DRIVER_L2)
/* 
 *	Function Name:	rtl8676_fdb_delete
 *	Description:	delete hwnat fdb entry
 *
 *	Input:		l2Type		:	entry type in L2 table
 *				addr			:	mac address for the fdb entry
 *
 *	Return:		none  
 */
void rtl8676_fdb_delete(unsigned short l2Type,  const unsigned char *addr)
{
#ifdef CONFIG_RTL865X_LANPORT_RESTRICTION
	rtl865x_delAuthLanFDBEntry(RTL865x_L2_TYPEII, addr);
#else
	rtl865x_delLanFDBEntry(RTL865x_L2_TYPEII, addr);
#endif
}

/* 
 *	Function Name:	rtl8676_fdb_new
 *	Description:	add hwnat fdb entry
 *
 *	Input:		is_static		:	check if the fdb entry in kernel is static
 *				addr			:	mac address for the fdb entry
 *
 *	Return:		none  
 */
void rtl8676_fdb_new(unsigned char is_static, const unsigned char *addr)
{
	if (is_static)
		return;

#ifdef CONFIG_RTL865X_LANPORT_RESTRICTION
	rtl865x_new_AuthFDB(addr);
#else
	rtl865x_addFDBEntry(addr);
#endif
}

/* 
 *	Function Name:	rtl8676_update_L2_check
 *	Description:	delete hwnat fdb entry when the source bridge port is changed for wlan interface
 *
 *	Input:		orig_br_port		:	original bridge port
 *				update_entry		:	updated bridge port
 *				name			:	net device name of new source
 *				addr				:	mac address for the fdb entry
 *
 *	Return:		none  
 */
void rtl8676_update_L2_check(void *orig_br_port, void *updated_br_port, const char *name, const unsigned char *addr)
{
	if (orig_br_port != updated_br_port)
		update_hw_l2table(name, addr);
}

#endif

#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER_L3)
/* 
 *	Function Name:	rtl8676_add_route
 *	Description:	add hwnat l3 table
 *
 *	Input:		ifindex		:	interface index
 *				ifname		:	interface name
 *				ipDst		:	destination IP
 *				ipMask		:	destination Mask
 				ipGw			:	gateway
 *
 *	Return:		SUCCESS		:	add 8676 route successfully 
 *				FAILED		:	add 8676 route failed    
 */
int32 rtl8676_add_route(int ifindex, char *ifname, unsigned int ipDst, unsigned int ipMask, unsigned ipGw)
{
	return rtl_fn_hash_insert(ifindex, ifname, ipDst, ipMask, ipGw);
}

/* 
 *	Function Name:	rtl8676_del_route
 *	Description:	delete hwnat l3 table
 *
 *	Input:		ipDst		:	destination IP
 *				ipMask		:	destination Mask
 *
 *	Return:		SUCCESS		:	add 8676 route successfully 
 *				FAILED		:	add 8676 route failed    
 */
int32 rtl8676_del_route(unsigned int IpDst, unsigned int IpMask){
	return rtl_fn_hash_delete(IpDst, IpMask);
}
#endif

#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER)  && defined(CONFIG_RTL_LAYERED_DRIVER_L3)
/********************************************************         
	RTL8676 API  - ARP
***********************************************************/
/* 
 *	Function Name:	rtl8676_setArp
 *	Description:	delete/check arp table
 *
 *	Input:		n			:	neighbour struct
 *				cmd			:	ARP_DEL 		del arp 
 *								ARP_CHECK 		check if arp is in use by napt connection
 *	Return:		ret			:	ARP_DEL			SUCCESS: del 8676 arp successfully; FAILED: del 8676 arp failed
 *								ARP_CHECK		0: in use; -1: not in use
 */
int rtl8676_setArp(struct neighbour *n, int cmd)
{

	u32 arp_ip = htonl(*((u32 *)n->primary_key));

	switch(cmd){
		
		case ARP_DEL: //del arp
			return rtl865x_delArp(arp_ip);
			
		//replace by using netevent notifier	
		//case ARP_ADD: //add arp
		//	rtl865x_addArp(arp_ip, (void *)n->ha);
		//	break;

		//case ARP_SYNC: //sync arp
		//	ret = rtl865x_arpSync(arp_ip, 0);
			
		case ARP_CHECK:
			return rtl865x_checkNaptConnection(arp_ip);
	}

}

#endif


#ifdef CONFIG_RTL_HARDWARE_NAT
#include <net/rtl/rtl_nic.h>
#include <net/rtl/rtl865x_fdb_api.h>
#include <net/rtl/rtl865x_ip_api.h>
#include <net/rtl/rtl865x_arp_api.h>
#include <net/rtl/rtl865x_nat.h>
#include "../acl_control/rtl865x_acl_control.h"
#include "../l3Driver/rtl865x_route.h"
#ifdef CONFIG_RTL_HW_QOS_SUPPORT
#include <net/rtl/rtl865x_outputQueue.h>
#endif
static int _rtl8676_analysis_ct(struct nf_conn *ct,uint32* intIP,uint32* remIP,uint32* extIP,
	uint16* intPort,uint16* remPort,uint16* extPort,uint8* proto,uint8* isNAPT)
{

	struct nf_conntrack_tuple* upstream_tuple 	= NULL;
	struct nf_conntrack_tuple* downstream_tuple = NULL;	

	/* This conntrack has to be checked whether ro do src/dst napt */
	if ( !(test_bit(IPS_SRC_NAT_DONE_BIT, &ct->status)) || !(test_bit(IPS_DST_NAT_DONE_BIT, &ct->status)) ) 
	{
		DBG_HWACCAPI_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return FAILED;
	}
	
	if(test_bit(IPS_SRC_NAT_BIT, &ct->status)) /* It is a src napt conntrack*/
	{
		*isNAPT = 1;
		upstream_tuple 	= &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
		downstream_tuple= &ct->tuplehash[IP_CT_DIR_REPLY].tuple;
		DBG_HWACCAPI_PRK("(%s)is a src napt conntrack\n",__func__);
	}
	else if(test_bit(IPS_DST_NAT_BIT, &ct->status)) /* It is a dst napt conntrack*/
	{
		*isNAPT = 1;
		upstream_tuple 	= &ct->tuplehash[IP_CT_DIR_REPLY].tuple;
		downstream_tuple= &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;	
		DBG_HWACCAPI_PRK("(%s)dst napt conntrack\n",__func__);
	}
	else /* Maybe it is pure routing,  we sw_l3 (maintained by ourselves) to distinguish up/down stream */
	{
		rtl865x_route_t rt_ori_src;
		rtl865x_route_t rt_rep_src;	
		
		DBG_HWACCAPI_PRK("(%s)Maybe it is pure routing\n",__func__);
		
		if((SUCCESS==rtl865x_getRouteEntry(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip, &rt_ori_src)) &&  
			!strncmp(rt_ori_src.dstNetif,ALIASNAME_BR,strlen(ALIASNAME_BR))) /* may be the ori tuple is upstream */
		{
			*isNAPT = 0;
			upstream_tuple 	= &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
			downstream_tuple= &ct->tuplehash[IP_CT_DIR_REPLY].tuple;
			DBG_HWACCAPI_PRK("(%s)ori tuple is upstream\n",__func__);			
		}
		else if((SUCCESS==rtl865x_getRouteEntry(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip, &rt_rep_src)) && 
			!strncmp(rt_rep_src.dstNetif,ALIASNAME_BR,strlen(ALIASNAME_BR)))  /* may be the rep tuple is upstream */
		{
			*isNAPT = 0;
			upstream_tuple 	= &ct->tuplehash[IP_CT_DIR_REPLY].tuple;
			downstream_tuple= &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
			DBG_HWACCAPI_PRK("(%s)ori tuple is downstream\n",__func__);		
		}		
	}
	
	if(upstream_tuple==NULL && downstream_tuple==NULL)
	{
		DBG_HWACCAPI_PRK("Leave %s @ %d  (upstream_tuple==NULL && downstream_tuple==NULL)\n",__func__,__LINE__);
		return FAILED;
	}

	*proto  = upstream_tuple->dst.protonum;
	*intIP	= upstream_tuple->src.u3.ip;
	*remIP 	= upstream_tuple->dst.u3.ip;
	*extIP 	= downstream_tuple->dst.u3.ip;
	*intPort	= upstream_tuple->src.u.all;
	*remPort	= upstream_tuple->dst.u.all;
	*extPort	= downstream_tuple->dst.u.all;

	return SUCCESS;
}

#ifdef CONFIG_RTL8676_Static_ACL
/********************************************************         
	RTL8676 API  - TCP/IP
***********************************************************/
int rtl8676_add_napt(struct nf_conn *ct)
{	
	u_int32_t	intIP, remIP, extIP;
	u_int16_t	intPort, remPort, extPort;
	u_int8_t	isNAPT ,proto;
	int ret;

	DBG_HWACCAPI_PRK("Enter %s ,  ori : %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u   rep : %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u \n",__func__
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip),ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all 
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip),ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip),ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.ip),ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all);/


	ret = _rtl8676_analysis_ct(ct, &intIP, &remIP, &extIP, &intPort, &remPort, &extPort, &proto, &isNAPT);
	if (ret != SUCCESS)
	{
		DBG_HWACCAPI_PRK("Leave %s @ %d  (_rtl8676_analysis_ct failed)\n",__func__,__LINE__);
		return FAILED;
	}

	/* we only accept napt case 
	    (In static ACL , pure routing is not concerned with napt table ) */
	if(isNAPT!=1)
	{
		DBG_HWACCAPI_PRK("Leave %s @ %d  (isNAPT!=1)\n",__func__,__LINE__);
		return FAILED;	
	}
	

	/* check protocol , we only handle TCP or UDP */
	if( proto!=IPPROTO_TCP && proto!=IPPROTO_UDP )
	{
		DBG_HWACCAPI_PRK("Leave %s @ %d  (proto!=IPPROTO_TCP && proto!=IPPROTO_UDP)\n",__func__,__LINE__);
		return FAILED;
	}	


	/* Add TCP/UDP table */	
	ret = rtl865x_addNaptConnection	(intIP,intPort,remIP,remPort,extIP,extPort,
		((proto==IPPROTO_TCP)?RTL865X_PROTOCOL_TCP:RTL865X_PROTOCOL_UDP),1);

	if(ret!=SUCCESS)		
		return FAILED;

	ret = rtl865x_addNaptConnection	(remIP,remPort,extIP,extPort,intIP,intPort,
		((proto==IPPROTO_TCP)?RTL865X_PROTOCOL_TCP:RTL865X_PROTOCOL_UDP),0);

	if(ret!=SUCCESS)	
	{
		rtl865x_delNaptConnection	(intIP,intPort,remIP,remPort,extIP,extPort,
			((proto==IPPROTO_TCP)?RTL865X_PROTOCOL_TCP:RTL865X_PROTOCOL_UDP),1);
		return FAILED;
	}

	return SUCCESS;
}
int rtl8676_del_napt(struct nf_conn *ct)
{
	u_int32_t	intIP, remIP, extIP;
	u_int16_t	intPort, remPort, extPort;
	u_int8_t	isNAPT ,proto;
	int ret;

	DBG_HWACCAPI_PRK("Enter %s ,  ori : %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u   rep : %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u \n",__func__
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip),ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all 
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip),ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip),ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.ip),ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all); 

	ret = _rtl8676_analysis_ct(ct, &intIP, &remIP, &extIP, &intPort, &remPort, &extPort, &proto, &isNAPT);
	if (ret != SUCCESS)
	{ 
		DBG_HWACCAPI_PRK("Leave %s @ %d  (_rtl8676_analysis_ct failed)\n",__func__,__LINE__);
		return FAILED;
	} 

	/* we only accept napt case 
	    (In static ACL , pure routing is not concerned with napt table ) */
	if(isNAPT!=1)
	{ 
		DBG_HWACCAPI_PRK("Leave %s @ %d  (isNAPT!=1)\n",__func__,__LINE__);
		return FAILED;
	}	

	/* check protocol , we only handle TCP or UDP */
	if( proto!=IPPROTO_TCP && proto!=IPPROTO_UDP )
	{ 
		DBG_HWACCAPI_PRK("Leave %s @ %d  (proto!=IPPROTO_TCP && proto!=IPPROTO_UDP)\n",__func__,__LINE__);
		return FAILED;
	} 	

	/* Delete TCP/UDP table */	
	ret = rtl865x_delNaptConnection	(intIP,intPort,remIP,remPort,extIP,extPort,
		((proto==IPPROTO_TCP)?RTL865X_PROTOCOL_TCP:RTL865X_PROTOCOL_UDP),1);

	if(ret!=SUCCESS)		
		return FAILED;

	ret = rtl865x_delNaptConnection	(remIP,remPort,extIP,extPort,intIP,intPort,
		((proto==IPPROTO_TCP)?RTL865X_PROTOCOL_TCP:RTL865X_PROTOCOL_UDP),0);

	if(ret!=SUCCESS)	
		return FAILED;	

	return SUCCESS;
}
int rtl8676_query_napt(struct nf_conn *ct)
{	
	u_int32_t	intIP, remIP, extIP;
	u_int16_t	intPort, remPort, extPort;
	u_int8_t	isNAPT ,proto;
	int ret,ret1,ret2;

	DBG_HWACCAPI_PRK("Enter %s ,  ori : %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u   rep : %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u \n",__func__
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip),ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all 
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip),ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all 
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip),ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all 
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.ip),ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all); 


	ret = _rtl8676_analysis_ct(ct, &intIP, &remIP, &extIP, &intPort, &remPort, &extPort, &proto, &isNAPT);
	if (ret != SUCCESS)
	{ 
		DBG_HWACCAPI_PRK("Leave %s @ %d  (_rtl8676_analysis_ct failed)\n",__func__,__LINE__);
		return FAILED;
	} 

	/* we only accept napt case 
	    (In static ACL , pure routing is not concerned with napt table ) */
	if(isNAPT!=1)
	{ 
		DBG_HWACCAPI_PRK("Leave %s @ %d  (isNAPT!=1)\n",__func__,__LINE__);
		return FAILED;
	} 		

	/* check protocol , we only handle TCP or UDP */
	if( proto!=IPPROTO_TCP && proto!=IPPROTO_UDP )
	{ 
		DBG_HWACCAPI_PRK("Leave %s @ %d  (proto!=IPPROTO_TCP && proto!=IPPROTO_UDP)\n",__func__,__LINE__);
		return FAILED;
	} 	

	/* Delete TCP/UDP table */	
	ret1 = rtl865x_lookupNaptConnection	(intIP,intPort,remIP,remPort,extIP,extPort,
		((proto==IPPROTO_TCP)?RTL865X_PROTOCOL_TCP:RTL865X_PROTOCOL_UDP),1);

	ret2 = rtl865x_lookupNaptConnection	(remIP,remPort,extIP,extPort,intIP,intPort,
		((proto==IPPROTO_TCP)?RTL865X_PROTOCOL_TCP:RTL865X_PROTOCOL_UDP),0);

	if(ret1 == RTL865x_EXISTED_IN_HW_ALIVE || ret2 == RTL865x_EXISTED_IN_HW_ALIVE)	
		return 1;
	else
		return 0;
}


#else /* CONFIG_RTL8676_Dynamic_ACL */
/********************************************************         
	RTL8676 API  - Unicast hardware acceleration
***********************************************************/
int rtl8676_add_L2Unicast_hwacc(unsigned char* src_mac, unsigned char* dst_mac, char *in_ifname, char *out_ifname)
{
	struct net_device *in_dev;
	struct net_device *out_dev;
	int lan_pvid;

	DBG_HWACCAPI_PRK("Enter %s  (%2X:%2X:%2X:%2X:%2X:%2X(%s) -> %2X:%2X:%2X:%2X:%2X:%2X(%s))\n"	,__func__
		,src_mac[0],src_mac[1],src_mac[2],src_mac[3],src_mac[4],src_mac[5],in_ifname 
		,dst_mac[0],dst_mac[1],dst_mac[2],dst_mac[3],dst_mac[4],dst_mac[5],out_ifname); 

	#if defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL_LAYERED_DRIVER_L2)
	if (FAILED == rtl865x_Lookup_L2_by_MAC(src_mac) || FAILED == rtl865x_Lookup_L2_by_MAC(dst_mac))
	{ 
		DBG_HWACCAPI_PRK("Leave %s @ %d  (cannot mac in hw_l2)\n",__func__,__LINE__);
		return FAILED;
	} 
	#endif

	/* get net device */
	in_dev = __dev_get_by_name(&init_net,in_ifname);
	if(!in_dev)
	{ 
		DBG_HWACCAPI_PRK("Leave %s @ %d  (cannot find in_dev)\n",__func__,__LINE__);
		return FAILED;
	} 
	
	out_dev = __dev_get_by_name(&init_net,out_ifname);
	if(!out_dev)
	{ 
		DBG_HWACCAPI_PRK("Leave %s @ %d  (cannot find out_dev)\n",__func__,__LINE__);
		return FAILED;
	} 


	/*  case 1.  LAN to WAN (upstream) 	
	                    if  the pkts from someone LAN dev only forward to only one WAN dev 
	                    (ex. port mapping or there is only one bridged interface)
                           we set L2 permit acl rule at WAN interface (because the pvid of LAN port is WAN's vid)
	*/
	if( (in_dev->priv_flags & IFF_DOMAIN_ELAN) && rtl865x_netif_is_wan(out_ifname)  )
	{
		/* check whether wan netif exists in asic netif table (sw_netif) */
		if(!rtl865x_netifExist(out_ifname))
		{ 
			DBG_HWACCAPI_PRK("Leave %s @ %d\n",__func__,__LINE__);
			return FAILED;
		} 

		/* check whether we should add L2Unicast hwacc*/
		lan_pvid = rtl865x_getLANpvid(in_ifname);
		if(lan_pvid == -1 || lan_pvid == RTL_LANVLANID){
			DBG_HWACCAPI_PRK("(%s) ERROR LAN & WAN does not have same vid\n",__func__);
			return FAILED;
		}
		
		DBG_HWACCAPI_PRK("(%s) LAN to WAN \n",__func__);
		//rtl865x_modify_DmacRule(dst_mac,m_1p,out_ifname,1);  //add port-based acl in boa, so, no need add dmac acl
		return rtl865x_acl_control_L2_permit_add(src_mac,dst_mac,out_ifname);
	}
	/*  case 2.  WAN to LAN (downstream) 	
	                    set L2 permit acl rule at WAN interface directly
	*/
	else if( rtl865x_netif_is_wan(in_ifname) && (out_dev->priv_flags & IFF_DOMAIN_ELAN) )
	{
		/* check whether wan netif exists in asic netif table (sw_netif) */
		if(!rtl865x_netifExist(in_ifname))	
		{ 
			DBG_HWACCAPI_PRK("Leave %s @ %d\n",__func__,__LINE__);
			return FAILED;
		}
		
		/* check whether we should add L2Unicast hwacc*/
		lan_pvid = rtl865x_getLANpvid(out_ifname);
		if(lan_pvid == -1 || lan_pvid == RTL_LANVLANID){
			DBG_HWACCAPI_PRK("(%s) ERROR WAN & LAN does not have same vid\n",__func__);
			return FAILED;
		} 
		
		DBG_HWACCAPI_PRK("(%s) WAN to LAN \n",__func__);
		return rtl865x_acl_control_L2_permit_add(src_mac,dst_mac,in_ifname);
	}
	/*  case3. Lan to Lan traffic
		each lan port maybe port-mapping to different wan netif (so its pvid is equal to the wanif's pvid) 
		we have to add the permit acl rule at the corresponding wan vid   
	*/							
	else if( (in_dev->priv_flags & IFF_DOMAIN_ELAN) && (out_dev->priv_flags & IFF_DOMAIN_ELAN) )
	{
		int in_pvid = rtl865x_getLANpvid(in_ifname);		
		int out_pvid = rtl865x_getLANpvid(out_ifname);	

		/* check whether we should add L2Unicast hwacc*/
		if(in_pvid != out_pvid){
			DBG_HWACCAPI_PRK("(%s) ERROR LAN & LAN does not have same pvid\n",__func__);
			return FAILED;
		}	

		DBG_HWACCAPI_PRK("(%s) LAN to LAN \n",__func__);
	
		if(in_pvid!=-1)
		{
			char wan_netif[IFNAMSIZ];			
			rtl865x_getMasterNetifByVid(in_pvid,wan_netif);			
			return rtl865x_acl_control_L2_permit_add(src_mac,dst_mac,wan_netif);
		}
		else
		{ 
			DBG_HWACCAPI_PRK("Leave %s @ %d\n",__func__,__LINE__);
			return FAILED;	
		} 
		
	}
	else /* other case */
	{ 
		DBG_HWACCAPI_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return FAILED;
	} 
	
}
int rtl8676_del_L2Unicast_hwacc(unsigned char* src_mac, unsigned char* dst_mac)
{
	return rtl865x_acl_control_L2_permit_del_pair(src_mac,dst_mac);
}


int rtl8676_del_L2Unicast_hwacc_agingout_mac(unsigned char* del_mac)
{
	DBG_HWACCAPI_PRK("Enter %s  (%2X:%2X:%2X:%2X:%2X:%2X)\n"	,__func__,
		 del_mac[0],del_mac[1],del_mac[2],del_mac[3],del_mac[4],del_mac[5]); 
			 
	return rtl865x_acl_control_L2_permit_del(del_mac);
}

/* It will tell you does this mac addr still alive in 8676 switch ? */
int rtl8676_query_L2Unicast_hwacc(unsigned char* query_mac)
{
	//DBG_HWACCAPI_PRK("Enter %s  (%2X:%2X:%2X:%2X:%2X:%2X)\n"	,__func__,
	//	 query_mac[0],query_mac[1],query_mac[2],query_mac[3],query_mac[4],query_mac[5]);
	#if defined (CONFIG_RTL_IVL_SUPPORT)
	if(rtl_get_hw_fdb_age(RTL_LAN_FID,ether_addr_t *query_mac,FDB_DYNAMIC)!=FAILED)			
		return 1;
	else if(rtl_get_hw_fdb_age(RTL_WAN_FID,ether_addr_t *query_mac,FDB_DYNAMIC)!=FAILED)			
		return 1;
	else
		return 0;
	#else
	if(rtl_get_hw_fdb_age(RTL_LAN_FID,(ether_addr_t *)query_mac,FDB_DYNAMIC)!=FAILED)
	{ 
		//DBG_HWACCAPI_PRK("Leave %s @ (still alive)\n",__func__);
		return 1;
	} 
	else
	{ 
		//DBG_HWACCAPI_PRK("Leave %s @ (timeout or not exitsed)\n",__func__);
		return 0;
	} 
	#endif
}

int rtl8676_clean_L2Unicast_hwacc(void)
{
	return rtl865x_acl_control_L2_permit_clean();
}


static int _rtl8676_del_L34Unicast_hwacc
	(uint32 scr_ip,uint16 scr_port,uint32 dst_ip,uint16 dst_port,uint32 naptIp ,uint16 naptPort,uint8 protocol,int isUpstream)
{

	/* check protocol , we only handle TCP or UDP */
	if( protocol!=IPPROTO_TCP && protocol!=IPPROTO_UDP )
	{ 
		DBG_HWACCAPI_PRK("Leave %s @ %d  (proto!=IPPROTO_TCP && proto!=IPPROTO_UDP)\n",__func__,__LINE__);
		return FAILED;
	} 

	if(isUpstream)
	{
		if(scr_ip == naptIp)
			rtl865x_delIp(scr_ip);
	}
	else
	{
		if(dst_ip == naptIp)
			rtl865x_delIp(dst_ip);
	}

	/* Delete TCP/UDP table */
	rtl865x_delNaptConnection(scr_ip,scr_port,dst_ip,dst_port,naptIp,naptPort
		,((protocol==IPPROTO_TCP)?RTL865X_PROTOCOL_TCP:RTL865X_PROTOCOL_UDP),isUpstream);

	/* Delete  ACL rule */
	if(isUpstream)	
	{
		rtl865x_acl_control_L34_redirect_del(scr_ip,scr_port,dst_ip,dst_port,protocol);	
	}
	
	rtl865x_acl_control_L34_permit_del(scr_ip,scr_port,dst_ip,dst_port,protocol);		

	DBG_HWACCAPI_PRK("Leave %s \n",__func__);
	return SUCCESS;	

}


/*  isUpstream=1   =>  Upstream		 :  Src NAPT or Pure routing ,  naptIp/naptPort = extIp/extPort
      isUpstream=0   =>  Downstream	 :  Dst NAPT or Pure routing ,  naptIp/naptPort = privateIp/privatePort */
static int _rtl8676_add_L34Unicast_hwacc
	(uint32 scr_ip,uint16 scr_port,uint32 dst_ip,uint16 dst_port,uint32 naptIp ,uint16 naptPort,uint8 protocol,uint8 pri_valid,uint8 pri_value,int isUpstream
	,uint32 nexthop_ip,char* in_netif,char* out_netif)
{
	int ret;

	/* check protocol , we only handle TCP or UDP */
	if( protocol!=IPPROTO_TCP && protocol!=IPPROTO_UDP )
	{ 
		DBG_HWACCAPI_PRK("Leave %s @ %d  (proto!=IPPROTO_TCP && proto!=IPPROTO_UDP)\n",__func__,__LINE__);
		return FAILED;
	} 
		
	if(!rtl865x_netifExist(in_netif) ||!rtl865x_netifExist(out_netif) )
	{ 
		DBG_HWACCAPI_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	} 	


	/*  we only hw-acc forwarding pkts (two different subnet ) */
	if(!strcmp(in_netif,out_netif))
	{
		DBG_HWACCAPI_PRK("Leave %s @ %d  (we only hw-acc forwarding pkts)\n",__func__,__LINE__);
		return FAILED;
	} 

	
	if(isUpstream)
	{	
		/* if the pkt is pure routing (without napt)
		     we have to add this scr_ip into asic IP table first.
		     (Because the nexthop entry the acl redirect to will look up IP table to refill the pkt's scrip) 
	         		Note. If it is not pure routing (i.e. NAPT) , the entry in IP table is added when ip_tables.c prase iptables's nat table   */			     
		if(scr_ip == naptIp)
			//rtl865x_addIp(scr_ip, scr_ip,IP_TYPE_NAT,d_rt.dstNetif->name); 
			//rtl865x_addIp(0, scr_ip,IP_TYPE_NAPT,d_rt.dstNetif); 
			rtl865x_addIp(0, scr_ip,IP_TYPE_NAPT,out_netif);
	}
	else
	{
		//for UDP single direction only test (WAN->LAN)
		if(dst_ip == naptIp)
			rtl865x_addIp(0, dst_ip,IP_TYPE_NAPT,in_netif);
	}


	/* Add ACL rule */	
	if(isUpstream)
	{
		/*   1. rediect rule */
		ret = rtl865x_acl_control_L34_redirect_add(scr_ip,scr_port,dst_ip,dst_port,protocol,in_netif
												,naptIp,nexthop_ip,out_netif);
		if(ret!=SUCCESS)
		{
			DBG_HWACCAPI_PRK("Leave %s @ %d\n",__func__,__LINE__);
			goto clean_up;
		}

		/*   2. permit rule */
		ret = rtl865x_acl_control_L34_permit_add(scr_ip,scr_port,dst_ip,dst_port,protocol,in_netif);	
		if(ret!=SUCCESS)
		{
			DBG_HWACCAPI_PRK("Leave %s @ %d\n",__func__,__LINE__);
			goto clean_up;
		}
	}
	else
	{
		/*   permit rule */
		int vid;
		ret = rtl865x_acl_control_L34_permit_add(scr_ip,scr_port,dst_ip,dst_port,protocol,in_netif);
		if(ret!=SUCCESS)
		{
			DBG_HWACCAPI_PRK("Leave %s @ %d\n",__func__,__LINE__);
			goto clean_up;
		}


		if(rtl865x_getNetifVid(in_netif,&vid)==SUCCESS)			
		{
			if(vid == RTL_WANVLANID)
			{
				char vid_7_netif_name[30];
				DBG_HWACCAPI_PRK("(%s) this netif's vid =8 ,  set vid 7 either... \n",__func__);

				rtl865x_getMasterNetifByVid(RTL_BridgeWANVLANID,vid_7_netif_name);

				if(strcmp(vid_7_netif_name,""))
				{			
					ret = rtl865x_acl_control_L34_permit_add(scr_ip,scr_port,dst_ip,dst_port,protocol,vid_7_netif_name);
					if(ret!=SUCCESS)
					{
						DBG_HWACCAPI_PRK("Leave %s @ %d\n",__func__,__LINE__);
						goto clean_up;
					}
				}	
			}
		}
		else
		{
			DBG_HWACCAPI_PRK("Leave %s @ %d \n",__func__,__LINE__);
			goto clean_up;
		}
		
	}

	/* Add TCP/UDP table */	
	ret = rtl865x_addNaptConnection
	(scr_ip,scr_port,dst_ip,dst_port,naptIp,naptPort,((protocol==IPPROTO_TCP)?RTL865X_PROTOCOL_TCP:RTL865X_PROTOCOL_UDP),pri_valid,pri_value,isUpstream);

	if(ret!=SUCCESS)
	{
		DBG_HWACCAPI_PRK("Leave %s @ %d\n",__func__,__LINE__);
		goto clean_up;	
	}
	
	DBG_HWACCAPI_PRK("Leave %s \n",__func__);
	return SUCCESS;	

clean_up: /* clean all established entry in naot or acl table */
	_rtl8676_del_L34Unicast_hwacc(scr_ip,scr_port,dst_ip,dst_port,naptIp,naptPort,protocol,isUpstream);
	return FAILED;	
	
}

static int _rtl8676_query_L34Unicast_hwacc
	(uint32 scr_ip,uint16 scr_port,uint32 dst_ip,uint16 dst_port,uint32 naptIp ,uint16 naptPort,uint8 protocol,int isUpstream)
{
	int proto = (protocol==IPPROTO_TCP)? RTL865X_PROTOCOL_TCP: RTL865X_PROTOCOL_UDP;

	return rtl865x_lookupNaptConnection(scr_ip, scr_port, dst_ip, dst_port, naptIp, naptPort,proto,isUpstream);

}

int rtl8676_add_L34Unicast_hwacc_upstream
	(uint32 scr_ip,uint16 scr_port,uint32 dst_ip,uint16 dst_port,uint32 src_naptIp ,uint16 src_naptPort,uint8 protocol
	,uint32 nexthop_ip,char* in_netif,char* out_netif,uint8 priority,uint8 pri_flag)
{
	int ret;

	DBG_HWACCAPI_PRK("Enter %s  (scr: %u.%u.%u.%u:%d   dst:%u.%u.%u.%u:%d   ext:%u.%u.%u.%u:%d  protocol:%d   nexthop:%u.%u.%u.%u   out_netif:%s)\n"
		,__func__,NIPQUAD(scr_ip),scr_port,NIPQUAD(dst_ip),dst_port,NIPQUAD(src_naptIp),src_naptPort,protocol,NIPQUAD(nexthop_ip),out_netif);	 

	ret = _rtl8676_add_L34Unicast_hwacc(scr_ip,scr_port,dst_ip,dst_port,src_naptIp,src_naptPort,protocol,pri_flag,priority,1,nexthop_ip,in_netif,out_netif);

	if(ret==SUCCESS) 
		DBG_HWACCAPI_SuccessAdd_PRK("Leave %s with  SUCCESS(scr: %u.%u.%u.%u:%d   dst:%u.%u.%u.%u:%d   ext:%u.%u.%u.%u:%d  protocol:%d   nexthop:%u.%u.%u.%u   out_netif:%s)\n"
		,__func__,NIPQUAD(scr_ip),scr_port,NIPQUAD(dst_ip),dst_port,NIPQUAD(src_naptIp),src_naptPort,protocol,NIPQUAD(nexthop_ip),out_netif); 

	return ret;
	
}

int rtl8676_add_L34Unicast_hwacc_downstream
	(uint32 scr_ip,uint16 scr_port,uint32 dst_ip,uint16 dst_port,uint32 dst_naptIp, uint16 dst_naptPort,uint8 protocol
	,char* in_netif,char* out_netif)
{
	int ret;	
	
	DBG_HWACCAPI_PRK("Enter %s  (scr: %u.%u.%u.%u:%d   dst:%u.%u.%u.%u:%d   pri:%u.%u.%u.%u:%d  protocol:%d)\n"
		,__func__,NIPQUAD(scr_ip),scr_port,NIPQUAD(dst_ip),dst_port,NIPQUAD(dst_naptIp),dst_naptPort,protocol); 


	ret = _rtl8676_add_L34Unicast_hwacc(scr_ip,scr_port,dst_ip,dst_port,dst_naptIp,dst_naptPort,protocol,0,0,0,dst_naptIp,in_netif,out_netif);
	
	if(ret==SUCCESS) 
		DBG_HWACCAPI_SuccessAdd_PRK("Leave %s  SUCCESS(scr: %u.%u.%u.%u:%d   dst:%u.%u.%u.%u:%d   pri:%u.%u.%u.%u:%d  protocol:%d)\n"
		,__func__,NIPQUAD(scr_ip),scr_port,NIPQUAD(dst_ip),dst_port,NIPQUAD(dst_naptIp),dst_naptPort,protocol); 
	
	return ret;
}


#ifdef CONFIG_RTL_HW_QOS_SUPPORT
int rtl8676_add_L34Unicast_hwacc_ct(struct nf_conn *ct,uint32 upstream_nexthop_ip,char* upstream_out_netif
												,int32 ipqos_enable,int32 ipqos_swQid,int32 ipqos_8021p_remark,int32 ipqos_dscp_remark)
#else
int rtl8676_add_L34Unicast_hwacc_ct(struct nf_conn *ct,uint32 upstream_nexthop_ip,char* upstream_out_netif)
#endif
{
	rtl865x_route_t	intIP_rt;
	u_int32_t	intIP, remIP, extIP;
	u_int16_t	intPort, remPort, extPort;
	u_int8_t	isNAPT ,proto;
	int ret;

	DBG_HWACCAPI_PRK("Enter %s ,  ori : %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u   rep : %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u \n",__func__
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip),ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all 
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip),ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all 
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip),ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all 
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.ip),ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all); 
	DBG_HWACCAPI_PRK("(%s)upstream_nexthop_ip : %u.%u.%u.%u   upstream_out_netif:%s \n",__func__,
		NIPQUAD(upstream_nexthop_ip),upstream_out_netif); 


	#ifdef CONFIG_RTL_HW_QOS_SUPPORT
	DBG_HWACCAPI_PRK("(%s)ipqos_enable:%d  ipqos_swQid:%d   ipqos_8021p_remark:%d   ipqos_dscp_remark:0x%X\n",__func__,
		ipqos_enable,ipqos_swQid,ipqos_8021p_remark,ipqos_dscp_remark); 
	#endif
	ret = _rtl8676_analysis_ct(ct, &intIP, &remIP, &extIP, &intPort, &remPort, &extPort, &proto, &isNAPT);
	if (ret != SUCCESS)
	{ 
		DBG_HWACCAPI_PRK("Leave %s @ %d  (_rtl8676_analysis_ct failed)\n",__func__,__LINE__);
		return FAILED;
	} 

	if (rtl865x_getRouteEntry(intIP, &intIP_rt) != SUCCESS) 
	{ 
		DBG_HWACCAPI_PRK("Leave %s @ %d  (rtl865x_getRouteEntry sip failed)\n",__func__,__LINE__);
		return FAILED;
	} 
	

	/* Step1. Add DOWNstream */
	ret = _rtl8676_add_L34Unicast_hwacc(remIP,remPort,extIP,extPort,intIP,intPort,proto,0,0,0,intIP,upstream_out_netif,intIP_rt.dstNetif);	
	if (ret != SUCCESS)
	{	
		
		DBG_HWACCAPI_PRK("Leave %s @ %d  (add failed)\n",__func__,__LINE__);
		return FAILED;
	}

	/* Step2. Add UPstream */
#ifdef CONFIG_RTL_HW_QOS_SUPPORT
	if(ipqos_enable)
	{		
		int priority = 	rtl865x_qosPriorityMappingGet(ipqos_swQid,ipqos_8021p_remark,ipqos_dscp_remark);
		if(priority==-1)
		{ 
			DBG_HWACCAPI_PRK("Leave %s @ %d (get priority failed)\n",__func__,__LINE__);
			return FAILED;
		} 
		
		ret = _rtl8676_add_L34Unicast_hwacc(intIP,intPort,remIP,remPort,extIP,extPort,proto,1,priority,1,upstream_nexthop_ip,intIP_rt.dstNetif,upstream_out_netif);	
	}
	else		
#endif
		ret = _rtl8676_add_L34Unicast_hwacc(intIP,intPort,remIP,remPort,extIP,extPort,proto,0,0,1,upstream_nexthop_ip,intIP_rt.dstNetif,upstream_out_netif);
	if (ret != SUCCESS)
	{ 
		_rtl8676_del_L34Unicast_hwacc(remIP,remPort,extIP,extPort,intIP,intPort,proto,0);
		DBG_HWACCAPI_PRK("Leave %s @ %d  (add failed)\n",__func__,__LINE__);
		return FAILED;
	}

	DBG_HWACCAPI_SuccessAdd_PRK("Leave %s SUCCESS ,  ori : %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u   rep : %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u \n",__func__
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip),ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all 
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip),ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all 
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip),ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all 
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.ip),ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all); 

	return SUCCESS;
	
}

int rtl8676_del_L34Unicast_hwacc_upstream
	(uint32 scr_ip,uint16 scr_port,uint32 dst_ip,uint16 dst_port,uint32 src_naptIp ,uint16 src_naptPort,uint8 protocol)
{
	int ret;	
	DBG_HWACCAPI_PRK("Enter %s  (scr: %u.%u.%u.%u:%d   dst:%u.%u.%u.%u:%d   ext:%u.%u.%u.%u:%d  protocol:%d)\n"
		,__func__,NIPQUAD(scr_ip),scr_port,NIPQUAD(dst_ip),dst_port,NIPQUAD(src_naptIp),src_naptPort,protocol); 
	
	ret = _rtl8676_del_L34Unicast_hwacc(scr_ip,scr_port,dst_ip,dst_port,src_naptIp,src_naptPort,protocol,1);
	
	if(ret == SUCCESS) 
		DBG_HWACCAPI_SuccessDel_PRK("Leave %s  SUCCESS (scr: %u.%u.%u.%u:%d   dst:%u.%u.%u.%u:%d   ext:%u.%u.%u.%u:%d  protocol:%d)\n"
		,__func__,NIPQUAD(scr_ip),scr_port,NIPQUAD(dst_ip),dst_port,NIPQUAD(src_naptIp),src_naptPort,protocol); 

	return ret;
}

int rtl8676_del_L34Unicast_hwacc_downstream
	(uint32 scr_ip,uint16 scr_port,uint32 dst_ip,uint16 dst_port,uint32 dst_naptIp, uint16 dst_naptPort,uint8 protocol)
{	
	int ret;
	DBG_HWACCAPI_PRK("Enter %s  (scr: %u.%u.%u.%u:%d   dst:%u.%u.%u.%u:%d   pri:%u.%u.%u.%u:%d  protocol:%d  )\n"
		,__func__,NIPQUAD(scr_ip),scr_port,NIPQUAD(dst_ip),dst_port,NIPQUAD(dst_naptIp),dst_naptPort,protocol); 
	ret = _rtl8676_del_L34Unicast_hwacc(scr_ip,scr_port,dst_ip,dst_port,dst_naptIp,dst_naptPort,protocol,0);	
	if(ret == SUCCESS) 
		DBG_HWACCAPI_SuccessDel_PRK("Leave %s  SUCCESS(scr: %u.%u.%u.%u:%d   dst:%u.%u.%u.%u:%d   pri:%u.%u.%u.%u:%d  protocol:%d  )\n"
		,__func__,NIPQUAD(scr_ip),scr_port,NIPQUAD(dst_ip),dst_port,NIPQUAD(dst_naptIp),dst_naptPort,protocol); 
	return ret;
}

int rtl8676_del_L34Unicast_hwacc_ct(struct nf_conn *ct)
{
	u_int32_t	intIP, remIP, extIP;
	u_int16_t	intPort, remPort, extPort;
	u_int8_t	isNAPT ,proto;
	int ret;

	DBG_HWACCAPI_PRK("Enter %s ,  ori : %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u   rep : %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u \n",__func__
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip),ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all 
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip),ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all 
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip),ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all 
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.ip),ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all); 

	ret = _rtl8676_analysis_ct(ct, &intIP, &remIP, &extIP, &intPort, &remPort, &extPort, &proto, &isNAPT);
	if (ret != SUCCESS)
	{ 
		DBG_HWACCAPI_PRK("Leave %s @ %d  (_rtl8676_analysis_ct failed)\n",__func__,__LINE__);
		return FAILED;
	} 

	ret = _rtl8676_del_L34Unicast_hwacc(intIP,intPort,remIP,remPort,extIP,extPort,proto,1);
	if (ret != SUCCESS)
	{ 
		DBG_HWACCAPI_PRK("Leave %s @ %d  (add failed)\n",__func__,__LINE__);
		return FAILED;
	} 

	ret = _rtl8676_del_L34Unicast_hwacc(remIP,remPort,extIP,extPort,intIP,intPort,proto,0);
	if (ret != SUCCESS)
	{ 
		DBG_HWACCAPI_PRK("Leave %s @ %d  (add failed)\n",__func__,__LINE__);
		return FAILED;
	} 

	DBG_HWACCAPI_SuccessDel_PRK("Leave %s SUCCESS ,  ori : %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u   rep : %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u \n",__func__
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip),ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all 
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip),ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all 
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip),ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all 
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.ip),ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all); 

	return SUCCESS;	
}


/* It will tell you does this flow still alive in hw-acc ? 
 return 	-1   	: the entry does not exist 
		 0    	: aging out  
		 >0 	:  its aging time left                                        */
int rtl8676_query_L34Unicast_hwacc_upstream
	(uint32 scr_ip,uint16 scr_port,uint32 dst_ip,uint16 dst_port,uint32 src_naptIp ,uint16 src_naptPort,uint8 protocol)
{	
	DBG_HWACCAPI_PRK("Enter %s  (scr: %u.%u.%u.%u:%d   dst:%u.%u.%u.%u:%d   ext:%u.%u.%u.%u:%d  protocol:%d)\n"
		,__func__,NIPQUAD(scr_ip),scr_port,NIPQUAD(dst_ip),dst_port,NIPQUAD(src_naptIp),src_naptPort,protocol); 
	
	return _rtl8676_query_L34Unicast_hwacc(scr_ip,scr_port,dst_ip,dst_port,src_naptIp,src_naptPort,protocol,1);
}

int rtl8676_query_L34Unicast_hwacc_downstream
	(uint32 scr_ip,uint16 scr_port,uint32 dst_ip,uint16 dst_port,uint32 dst_naptIp, uint16 dst_naptPort,uint8 protocol)
{	
	DBG_HWACCAPI_PRK("Enter %s  (scr: %u.%u.%u.%u:%d   dst:%u.%u.%u.%u:%d   ext:%u.%u.%u.%u:%d  protocol:%d)\n"
		,__func__,NIPQUAD(scr_ip),scr_port,NIPQUAD(dst_ip),dst_port,NIPQUAD(dst_naptIp),dst_naptPort,protocol); 
	
	return _rtl8676_query_L34Unicast_hwacc(scr_ip,scr_port,dst_ip,dst_port,dst_naptIp,dst_naptPort,protocol,0);
}

int rtl8676_query_L34Unicast_hwacc_ct(struct nf_conn *ct)
{
	u_int32_t	intIP, remIP, extIP;
	u_int16_t	intPort, remPort, extPort;
	u_int8_t	isNAPT ,proto;
	int ret,ret1,ret2;

	DBG_HWACCAPI_PRK("Enter %s ,  ori : %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u   rep : %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u \n",__func__
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip),ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all 
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip),ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all 
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip),ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all 
		,NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.ip),ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all); 

	ret = _rtl8676_analysis_ct(ct, &intIP, &remIP, &extIP, &intPort, &remPort, &extPort, &proto, &isNAPT);
	if (ret != SUCCESS)
	{ 
		DBG_HWACCAPI_PRK("Leave %s @ %d  (_rtl8676_analysis_ct failed)\n",__func__,__LINE__);
		return -1;	
	} 

	ret1 = _rtl8676_query_L34Unicast_hwacc(intIP,intPort,remIP,remPort,extIP,extPort,proto,1);
	ret2 = _rtl8676_query_L34Unicast_hwacc(remIP,remPort,extIP,extPort,intIP,intPort,proto,0);
	DBG_HWACCAPI_PRK("(%s) ret1:%d  ret2:%d\n",__func__,ret1,ret2);
	return ((ret1>ret2)?ret1:ret2);	
}


int rtl8676_clean_L34Unicast_hwacc(void)
{
	int ret1,ret2,ret3;

	ret1 = rtl865x_flushNapt();
	ret2 = rtl865x_acl_control_L34_permit_clean();
	ret3 = rtl865x_acl_control_L34_redirect_clean();

	if( ret1!=SUCCESS || ret2!=SUCCESS || ret3!=SUCCESS )
	{ 
		DBG_HWACCAPI_PRK(" %s failed\n",__func__);
		return FAILED;
	} 
	else
	{ 	
		DBG_HWACCAPI_PRK(" %s SUCCESS\n",__func__);
		return SUCCESS;
	} 
}
#endif
#endif


#ifdef CONFIG_RTL_HW_QOS_SUPPORT
#include "../common/rtl865x_netif_local.h"
#include <net/rtl/rtl865x_outputQueue.h>
#ifdef CONFIG_RTL_8367B
#include "../AsicDriver/rtl865x_asicCom.h"
#include "../rtl8367b/rtl867x_8367b_qos.h"
#endif
#define 	IPQos_Disable 	0
#define 	IPQos_SP		1
#define 	IPQos_WRR 		2
#define 	IPQos_SP_WRR 	3

static int	IPQos_Mode = IPQos_Disable;
/********************************************************         
	RTL8676 API  - IPQoS
***********************************************************/
int rtl8676_IPQos_Disable(void)
{
	int ret;

	DBG_HWIPQOSAPI_PRK("Enter %s\n",__func__);

	if(IPQos_Mode == IPQos_Disable)
	{
		DBG_HWIPQOSAPI_PRK("Leave %s SUCCESS\n",__func__);
		return SUCCESS;	
	}


#ifdef CONFIG_RTL_8367B
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8367B)
	{
		/*   8676 PN  */
		ret = rtl867x_8367b_closeQos();
		if (ret != SUCCESS)
		{
			DBG_HWIPQOSAPI_PRK("Leave %s @ %d  (rtl867x_8367b_closeQos failed)\n",__func__,__LINE__);
			return FAILED;
		}

		ret = rtl865x_closeQos();
		if (ret != SUCCESS)
		{
			DBG_HWIPQOSAPI_PRK("Leave %s @ %d  (rtl865x_closeQos failed)\n",__func__,__LINE__);
			return FAILED;
		}
	}
	else
#endif
	{
		/*   8676 P  */
		ret = rtl865x_closeQos();
		if (ret != SUCCESS)
		{
			DBG_HWIPQOSAPI_PRK("Leave %s @ %d  (rtl865x_closeQos failed)\n",__func__,__LINE__);
			return FAILED;
		}
	}	

	IPQos_Mode = IPQos_Disable;
	return SUCCESS;
}

/* total_bandwidth:-1  : no limit */
int rtl8676_IPQos_Enable(int sp_queue_num, int wrr_queue_num ,int ceil[],int rate[], int default_qid)
{
	int ret;
	int band_num = sp_queue_num + wrr_queue_num;
	
	DBG_HWIPQOSAPI_PRK("Enter %s\n",__func__);
	if(default_qid<0 || default_qid>=band_num)
	{
		DBG_HWIPQOSAPI_PRK("Leave %s @ %d (arg error)\n",__func__,__LINE__);
		return FAILED;
	}	

	/* If IPQos is not disabled before, reset it first */
	if(IPQos_Mode != IPQos_Disable)
	{ 
		ret = rtl8676_IPQos_Disable();
		if (ret != SUCCESS)
		{
			DBG_HWIPQOSAPI_PRK("Leave %s @ %d  (rtl865x_closeQos failed)\n",__func__,__LINE__);
			return FAILED;	
		}
	}
	if(sp_queue_num!=0 && wrr_queue_num!=0)
		IPQos_Mode = IPQos_SP_WRR;
	else if(sp_queue_num!=0)
		IPQos_Mode = IPQos_SP;
	else if(wrr_queue_num!=0)
		IPQos_Mode = IPQos_WRR;

#ifdef CONFIG_RTL_8367B
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8367B)
	{
		/*   8676 PN  
			8676 embeded switch 	: only do remarking	, so just create one queue
			8367b 				: do classcification
		*/
		ret = rtl865x_enableQos(1,0,NULL,NULL,0);
		if (ret != SUCCESS)
		{
			DBG_HWIPQOSAPI_PRK("Leave %s @ %d  (rtl865x_enableQos failed)\n",__func__,__LINE__);
			goto set_fail;
		}

		ret = rtl867x_8367b_enableQos(sp_queue_num,wrr_queue_num,ceil,rate,default_qid);
		if (ret != SUCCESS)
		{
			DBG_HWIPQOSAPI_PRK("Leave %s @ %d  (rtl867x_8367b_enableQos failed)\n",__func__,__LINE__);
			goto set_fail;
		}
	}
	else	
#endif
	{
		/*   8676 P  */
		ret = rtl865x_enableQos(sp_queue_num,wrr_queue_num,ceil,rate,default_qid);
		if (ret != SUCCESS)
		{
			DBG_HWIPQOSAPI_PRK("Leave %s @ %d  (rtl865x_enableQos failed)\n",__func__,__LINE__);
			goto set_fail;
		}

		/* if it has wrr queue, it MUST set total_bandwidth on 8676 embeded switch */
		if(wrr_queue_num>0)
		{
			ret = rtl865x_qosSetBandwidth(RTL_WANPORT_MASK , 100000000); /* 100mbps */
			if (ret != SUCCESS)	
			{
				DBG_HWIPQOSAPI_PRK("Leave %s @ %d  (set 8676 bandwidth failed)\n",__func__,__LINE__);
				goto set_fail;	
			}
		}
	}
	return SUCCESS;
	
set_fail:
	rtl8676_IPQos_Disable();
	return FAILED;
	
}

int rtl8676_IPQos_Remarking(int enable_8021p,int enable_dscp)
{
	DBG_HWIPQOSAPI_PRK("Enter %s\n",__func__);

	if(IPQos_Mode == IPQos_Disable)
	{ 		
		DBG_HWIPQOSAPI_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;			
	}

	return rtl865x_Qos_SetRemarking(enable_8021p,enable_dscp);
}

int 	rtl8676_IPQos_RateLimit_Enable(int total_bandwidth)
{
	int ret;
	
#ifdef CONFIG_RTL_8367B
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8367B)
	{
		ret = RL6000_wanport_bandwidth_set(total_bandwidth);		
		if (ret != 0)
		{
			DBG_HWIPQOSAPI_PRK("Leave %s @ %d  (set 8367b bandwidth failed)\n",__func__,__LINE__);
			return FAILED;
		}
	}
	else
#endif
	{
		ret = rtl865x_qosSetBandwidth(RTL_WANPORT_MASK , total_bandwidth);
		if (ret != SUCCESS)	
		{
			DBG_HWIPQOSAPI_PRK("Leave %s @ %d  (set 8676 bandwidth failed)\n",__func__,__LINE__);
			return FAILED;
		}
	}

	return SUCCESS;
}

int 	rtl8676_IPQos_RateLimit_Disable(void)
{
	int ret;
	
#ifdef CONFIG_RTL_8367B
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8367B)
	{
		ret = RL6000_wanport_bandwidth_flush();		
		if (ret != 0)
		{
			DBG_HWIPQOSAPI_PRK("Leave %s @ %d  (set 8367b bandwidth failed)\n",__func__,__LINE__);
			return FAILED;
		}
	}
	else
#endif
	{
		if(IPQos_Mode == IPQos_SP_WRR || IPQos_Mode == IPQos_WRR)
		{
			ret = rtl865x_qosSetBandwidth(RTL_WANPORT_MASK , 100000000);
			if (ret != SUCCESS)	
			{
				DBG_HWIPQOSAPI_PRK("Leave %s @ %d  (set 8676 bandwidth failed)\n",__func__,__LINE__);
				return FAILED;
			}
		}
		else
		{
			ret = rtl865x_qosFlushBandwidth(RTL_WANPORT_MASK);
			if (ret != SUCCESS)	
			{
				DBG_HWIPQOSAPI_PRK("Leave %s @ %d  (set 8676 bandwidth failed)\n",__func__,__LINE__);
				return FAILED;
			}
		}
	}

	return SUCCESS;
}


int rtl8676_IPQos_AddRule(rtl867x_hwnat_qos_rule_t* input_rule,uint32 q_index,uint32 remark_1p,uint32 remark_dscp)
{
	int ret;	
	DBG_HWIPQOSAPI_PRK("Enter %s\n",__func__);


	if(input_rule->rule_type == RTL867x_IPQos_Format_8367Ext_IPMask ||
		input_rule->rule_type == RTL867x_IPQos_Format_8367Ext_IPRange )
	{
		DBG_HWIPQOSAPI_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return FAILED;
	}

#ifdef CONFIG_RTL_8367B
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8367B)
	{
		/*   8676 PN  
			8676 embeded switch 	: only do remarking	, so just create one queue
			8367b 				: do classcification
		*/
		ret = rtl865x_qosAddRule(input_rule,0,remark_1p,remark_dscp);
		if (ret != SUCCESS)
		{
			DBG_HWIPQOSAPI_PRK("Leave %s @ %d  (rtl865x_qosAddRule failed)\n",__func__,__LINE__);
			return FAILED;
		}

		ret = rtl867x_8367b_add_qosrule(input_rule,q_index);
		if (ret != SUCCESS)
		{
			DBG_HWIPQOSAPI_PRK("Leave %s @ %d  (rtl867x_8367b_add_qosrule failed)\n",__func__,__LINE__);
			return FAILED;
		}
	}
	else
#endif
	{
		/* 8676 P */
		ret = rtl865x_qosAddRule(input_rule,q_index,remark_1p,remark_dscp);
		if (ret != SUCCESS)
		{
			DBG_HWIPQOSAPI_PRK("Leave %s @ %d  (rtl865x_qosAddRule failed)\n",__func__,__LINE__);
			return FAILED;
		}
	}	

	return SUCCESS;
}

int rtl8676_IPQos_AddRule_Classification(rtl867x_hwnat_qos_rule_t* input_rule,uint32 q_index)
{
	int ret;	
	DBG_HWIPQOSAPI_PRK("Enter %s\n",__func__);
	

#ifdef CONFIG_RTL_8367B
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8367B)
	{
		/*   8676 PN  */
		ret = rtl867x_8367b_add_qosrule(input_rule,q_index);
		if (ret != SUCCESS)
		{
			DBG_HWIPQOSAPI_PRK("Leave %s @ %d  (rtl867x_8367b_add_qosrule failed)\n",__func__,__LINE__);
			return FAILED;
		}
	}
	else
#endif
	{
		/* 8676 P */		
		if(input_rule->rule_type == RTL867x_IPQos_Format_8367Ext_IPMask ||
			input_rule->rule_type == RTL867x_IPQos_Format_8367Ext_IPRange )
		{
			DBG_HWIPQOSAPI_PRK("Leave %s @ %dn",__func__,__LINE__);
			return FAILED;
		}
		
		ret = rtl865x_qosAddRule(input_rule,q_index,-1,-1);
		if (ret != SUCCESS)
		{
			DBG_HWIPQOSAPI_PRK("Leave %s @ %d  (rtl865x_qosAddRule failed)\n",__func__,__LINE__);
			return FAILED;
		}
	}	

	return SUCCESS;
}

int rtl8676_IPQos_AddRule_Remarking(rtl867x_hwnat_qos_rule_t* input_rule,uint32 remark_1p,uint32 remark_dscp)
{
	int ret;	
	DBG_HWIPQOSAPI_PRK("Enter %s\n",__func__);

	if(input_rule->rule_type == RTL867x_IPQos_Format_8367Ext_IPMask ||
		input_rule->rule_type == RTL867x_IPQos_Format_8367Ext_IPRange )
	{
		DBG_HWIPQOSAPI_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return FAILED;
	}	

#ifdef CONFIG_RTL_8367B
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8367B)
	{
		/*   8676 PN  */
		ret = rtl865x_qosAddRule(input_rule,0,remark_1p,remark_dscp);
		if (ret != SUCCESS)
		{
			DBG_HWIPQOSAPI_PRK("Leave %s @ %d  (rtl865x_qosAddRule failed)\n",__func__,__LINE__);
			return FAILED;
		}
	}
	else
#endif
	{
		/* 8676 P */				
		ret = rtl865x_qosAddRule(input_rule,-1,remark_1p,remark_dscp);
		if (ret != SUCCESS)
		{
			DBG_HWIPQOSAPI_PRK("Leave %s @ %d  (rtl865x_qosAddRule failed)\n",__func__,__LINE__);
			return FAILED;
		}
	}	

	return SUCCESS;
}



void rtl8676_IPQos_CleanRule(void)
{
	rtl865x_qosFlushRule();
#ifdef CONFIG_RTL_8367B	
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8367B)	
		rtl867x_8367b_flush_qosrule();	
#endif
}
#endif
#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER_L3)
/********************************************************         
	RTL8676 API  - External IP table
***********************************************************/
/* 
 *	Function Name:	rtl8676_add_napt_extIp
 *	Description:		add ext IP table
 *
 *	Input:		name		:	interface name
 *				ip			:	local ip for napt intf
 *
 *	Return:		SUCCESS		:	add 8676 ext IP table successfully 
 *				FAILED		:	add 8676 ext IP table failed    
 */
int rtl8676_add_napt_extIp(char *name, unsigned int ip){
	int rc = FAILED;
	if(ip != 0){
		rc = rtl865x_addIp(0, ip, IP_TYPE_NAPT, name);
		if(rc == FAILED){
			printk("rtl865x_addIp failed! %s  %d\n", __FUNCTION__, __LINE__);
		}
	}
	rc = add_masq_intf(name);
	if(rc == FAILED)
		printk("add masq intf failed! %s  %d\n", __FUNCTION__, __LINE__);

	return rc;
}

/* 
 *	Function Name:	rtl8676_del_napt_extIp
 *	Description:		del ext IP table
 *
 *	Input:		name		:	interface name
 *				ip			:	local ip for napt intf
 *
 *	Return:		SUCCESS		:	del 8676 ext IP table successfully 
 *				FAILED		:	del 8676 ext IP table failed    
 */
int rtl8676_del_napt_extIp(char *name, unsigned int ip){
	int rc = FAILED;

	if(ip != 0){
		rc = rtl865x_delIp(ip);
		if(rc == FAILED)
			printk("rtl865x_delIp failed! %s  %d\n", __FUNCTION__, __LINE__);
	}
	rc = del_masq_intf(name);
	if(rc == FAILED)
		printk("del masq intf failed! %s  %d\n", __FUNCTION__, __LINE__);

	return rc;
}

int rtl8676_napt_extip_handle(char *name, int dev_event, unsigned int addr){
		
	int rc=0;
	if(check_intf_in_masq_intf(name) == SUCCESS){
		if (dev_event == NETDEV_UP ) {
			rc = rtl865x_addIp(0,addr,IP_TYPE_NAPT,name);
		}
		if (dev_event == NETDEV_DOWN) {
			rc = rtl865x_delIp(addr);
			#if 0
			if(rc==SUCCESS)
			{
			#ifdef CONFIG_RTL8676_Static_ACL
				rtl865x_nat_init();
			#else /* CONFIG_RTL8676_Dynamic_ACL */
				rtl8676_clean_L34Unicast_hwacc();
			#endif
			}	
			#endif
		}
	}
	return rc;
}
#endif


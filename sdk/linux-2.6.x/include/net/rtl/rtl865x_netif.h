/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.
* 
* Program : network interface driver header file
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/

#ifndef RTL865X_NETIF_H
#define RTL865X_NETIF_H
#include <linux/if.h>
#include "rtl_types.h"
#include "rtl_alias.h"

//#define DBG_ACL
#ifdef DBG_ACL
#define DBG_ACL_PRK printk
#else
#define DBG_ACL_PRK(format, args...)
#endif
#if defined(CONFIG_RTL8196_RTL8366)
        #define RTL_WANPORT_MASK                0x1C1
        #define RTL_LANPORT_MASK                        0x1C1
        #define RTL8366RB_GMIIPORT              0x20
        #define RTL8366RB_LANPORT               0xCf
        #define RTL8366RB_WANPORT               0x10
#elif (defined(CONFIG_RTL_819X) || defined(CONFIG_RTL_8676HWNAT))&& (defined(CONFIG_RTK_VLAN_SUPPORT) || defined (CONFIG_RTL_MULTI_LAN_DEV))
#if defined (CONFIG_POCKET_ROUTER_SUPPORT)
        #define RTL_WANPORT_MASK                0x10
        #define RTL_LANPORT_MASK                        0x10
#elif defined(CONFIG_RTL_PUBLIC_SSID)
        #define RTL_WANPORT_MASK                        0x110     //port 4/port 8
        #define RTL_LANPORT_MASK                        0x10f
#elif defined(CONFIG_8198_PORT5_RGMII)
        #define RTL_WANPORT_MASK                0x10
        #define RTL_LANPORT_MASK                        0x12f
#else
        #if defined (CONFIG_RTL_8196C_iNIC)
        #define RTL_WANPORT_MASK                0x01
        #define RTL_LANPORT_MASK                        0x110 //mark_inic , only port4 connect to MII
        #elif defined (CONFIG_8198_PORT5_GMII)
        #define RTL_WANPORT_MASK                0x10  //port0
        #define RTL_LANPORT_MASK                        0x12f
        #else
#ifndef CONFIG_RTL_HW_PURE_SWITCH
        #define RTL_WANPORT_MASK                0x01  //port0
        #define RTL_LANPORT_MASK                0x11e //cpu port,port1,2,3,4
#else
	#define RTL_WANPORT_MASK		0x0
	#define RTL_LANPORT_MASK		0x11f
#endif
        #endif
#endif
        #define         RTL_LANPORT_MASK_1              0x10    //port 4
        #define         RTL_LANPORT_MASK_2              0x08    //port 3
        #define         RTL_LANPORT_MASK_3              0x04    //port 2
        #define         RTL_LANPORT_MASK_4              0x02    //port 1
        
        #ifdef CONFIG_8198_PORT5_GMII
        #define         RTL_LANPORT_MASK_5              0x20    //port 5
        #endif
#elif defined(CONFIG_RTL_8198_NFBI_BOARD)
        #define RTL_WANPORT_MASK                0x1e0  //port 5, port 6,port 7,port 8
        #define RTL_LANPORT_MASK                        0x1df //port 0~4 , port 6~8  , need port4 ??
#elif defined(CONFIG_8198_PORT5_GMII)
        #define RTL_WANPORT_MASK                0x110
        #define RTL_LANPORT_MASK                        0x1ef
#elif defined (CONFIG_POCKET_ROUTER_SUPPORT)
        #define RTL_WANPORT_MASK                0x10
        #define RTL_LANPORT_MASK                        0x10
#elif defined(CONFIG_RTL_PUBLIC_SSID)
        #define RTL_WANPORT_MASK                0x110     //port 4/port 8
#elif defined(CONFIG_RTL8186_KB_N) || defined(CONFIG_RTL_819X) || defined(CONFIG_RTL_8676HWNAT) /*      defined(CONFIG_RTL8196_RTL8366) */
        #ifdef CONFIG_RTL_8196C_iNIC
        #define RTL_WANPORT_MASK                0x01
        #define RTL_LANPORT_MASK                        0x110 //mark_inic , only port4 connect to MII
        #else
        //#define       RTL_WANPORT_MASK                0x10
        //#define       RTL_LANPORT_MASK                        0x10f
        #define RTL_WANPORT_MASK                0x00
        #define RTL_LANPORT_MASK                0x11f //cpu port,port 0,1,2,3,4
        #endif
#else
        #define RTL_WANPORT_MASK                0x01
        #define RTL_LANPORT_MASK                        0x11e           /* port1/2/3/4/cpu port(port 8) */
        #if defined(CONFIG_RTK_VLAN_SUPPORT) || defined (CONFIG_RTL_MULTI_LAN_DEV)
        #define         RTL_LANPORT_MASK_1              0x2             //port 1
        #define RTL_LANPORT_MASK_2              0x4             //port 2
        #define         RTL_LANPORT_MASK_3              0x8             //port 3
        #define         RTL_LANPORT_MASK_4              0x10    //port 4
        #endif
#endif  /*      defined(CONFIG_RTL8186_KB_N) || defined(CONFIG_RTL_819X)        */



#define 	IF_NONE 0
#define	IF_ETHER 1
#define	IF_PPPOE 2
#define	IF_PPTP 3
#define	IF_L2TP 4

#ifdef CONFIG_RTL8196E_IPCHECKSUM_ERROR_PATCH
#define RTL865X_ACL_ICBUG_PATCH				-40000		/* trap the packets if rtl8196d hwacc leads to ip checksum error when enable l2 dscp remarking  */
#endif
#define RTL865X_ACL_IPV6_USED				-30000
#define RTL865X_ACL_QOS_USED0				-20001		/* for user add queue */
#define RTL865X_ACL_IPv4MUL					-10002		/* for multicast pkts whose dmac = 01:00:5e... */
#define RTL865X_ACL_IPv4MUL_PPPoE			-10001		/* for multicast pkts whose dmac = localhost */
#define RTL865X_ACL_PREPROCESS_LOCALHOST  	-10000		/*  for pkts to localhost */
#define RTL865X_ACL_USER_USED				0
#define RTL865X_ACL_DEFAULT					10000

#if defined (CONFIG_RTL_IGMP_SNOOPING) && defined(CONFIG_RTL_HW_L2_ONLY)
#define RTL865X_ACL_IGMP_USED				-30001
#endif

#if 1 //def CONFIG_RTL_LAYERED_DRIVER_ACL
typedef union _rtl865x_AclRuleData_s
{
		/* MAC ACL rule */
		struct {
			ether_addr_t _dstMac, _dstMacMask;
			ether_addr_t _srcMac, _srcMacMask;
			uint16 _typeLen, _typeLenMask;//_typeLen means type pattern here.
		} MAC;
		
		/* IP Group ACL rule */
		struct
		{
			ipaddr_t _srcIpAddr, _srcIpAddrMask;
			ipaddr_t _dstIpAddr, _dstIpAddrMask;
			uint8 _tos, _tosMask;
			union
			{
				/* IP ACL rle */
				struct
				{
					uint8 _proto, _protoMask, _flagMask;// flag & flagMask only last 3-bit is meaning ful
					uint32 _FOP:1, _FOM:1, _httpFilter:1, _httpFilterM:1, _identSrcDstIp:1, _identSrcDstIpM:1;
					union 
					{
						uint8 _flag;
						struct 
						{
							uint8 pend1:5,
								 pend2:1,
								 _DF:1,	//don't fragment flag
								 _MF:1;	//more fragments flag
						} s;
					} un;							
				} ip; 
				
				/* ICMP ACL rule */
				struct 
				{
					uint8 _type, _typeMask, _code, _codeMask;
				} icmp; 
				
				/* IGMP ACL rule */
				struct
				{
					uint8 _type, _typeMask;
				} igmp; 
				
				/* TCP ACL rule */
				struct
				{
					uint8 _flagMask;
					uint16 _srcPortUpperBound, _srcPortLowerBound;
					uint16 _dstPortUpperBound, _dstPortLowerBound;
					union
					{
						uint8 _flag;
						struct
						{
							uint8 _pend:2,
								  _urg:1, //urgent bit
								  _ack:1, //ack bit
								  _psh:1, //push bit
								  _rst:1, //reset bit
								  _syn:1, //sync bit
								  _fin:1; //fin bit
						}s;
					}un;					
				}tcp;
				
				/* UDP ACL rule */
				struct 
				{
					uint16 _srcPortUpperBound, _srcPortLowerBound;
					uint16 _dstPortUpperBound, _dstPortLowerBound;										
				}udp; 
			}is;			
		}L3L4; 

		/* Source filter ACL rule */
		struct 
		{
			ether_addr_t _srcMac, _srcMacMask;
			uint16 _srcPort, _srcPortMask;
			uint16 _srcVlanIdx, _srcVlanIdxMask;
			ipaddr_t _srcIpAddr, _srcIpAddrMask;
			uint16 _srcPortUpperBound, _srcPortLowerBound;
			uint32 _ignoreL4:1, //L2 rule
				  	 _ignoreL3L4:1; //L3 rule
		} SRCFILTER;
		
		/* Destination filter ACL rule */
		struct 
		{
			ether_addr_t _dstMac, _dstMacMask;
			uint16 _vlanIdx, _vlanIdxMask;
			ipaddr_t _dstIpAddr, _dstIpAddrMask;
			uint16 _dstPortUpperBound, _dstPortLowerBound;
			uint32 _ignoreL4:1, //L3 rule
				   _ignoreL3L4:1; //L2 rule
		} DSTFILTER;


		/* 802.1p */
		struct {
			uint8	vlanTagPri;
		} VLANTAG;


}rtl865x_AclRuleData_t;


typedef struct _rtl865x_AclRule_s
{
	/* The Data Field   */
	rtl865x_AclRuleData_t un_ty;
	
	uint32 ruleType_;	

	/* Other Fields */
	uint32	actionType_:4,
			pktOpApp_:3,
			priority_:3,
			direction_:2,
#if	defined(CONFIG_RTL_HW_QOS_SUPPORT)
			upDown_:1,//0: uplink acl rule for hw qos; 1: downlink acl rule for hw qos
#endif
			nexthopIdx_:5, /* Index of nexthop table (NOT L2 table) */	/* used as network interface index for 865xC qos system */
			ratelimtIdx_:4; /* Index of rate limit table */	/* used as outputQueue index for 865xC qos system */		

	
	uint32	netifIdx_:3, /*for redirect*/
			pppoeIdx_:3, /*for redirect*/
			L2Idx_:10, /* Index of L2 table */
			inv_flag:8, /*mainly for iptables-->acl rule, when iptables rule has invert netif flag, this acl rule is added to other netifs*/
			aclIdx:7;	/* aisc entry idx */
	
	struct list_head AclRuleList;	
	//struct _rtl865x_AclRule_s *pre,*next;
	
	#ifdef DBG_ACL
	int __dbg_touch__; /* internal use */
	#endif
	
}rtl865x_AclRule_t;


/* MAC ACL rule Definition */
#define dstMac_				MAC._dstMac
#define dstMacMask_			MAC._dstMacMask
#define srcMac_				MAC._srcMac
#define srcMacMask_			MAC._srcMacMask
#define typeLen_				MAC._typeLen
#define typeLenMask_			MAC._typeLenMask

/* Common IP ACL Rule Definition */
#define srcIpAddr_			L3L4._srcIpAddr
#define srcIpAddrMask_		L3L4._srcIpAddrMask
#define srcIpAddrUB_			L3L4._srcIpAddr
#define srcIpAddrLB_			L3L4._srcIpAddrMask
#define dstIpAddr_			L3L4._dstIpAddr
#define dstIpAddrMask_		L3L4._dstIpAddrMask
#define dstIpAddrUB_			L3L4._dstIpAddr
#define dstIpAddrLB_			L3L4._dstIpAddrMask
#define tos_					L3L4._tos
#define tosMask_				L3L4._tosMask
/* IP Rrange */
/*Hyking:Asic use Addr to srore Upper address
	and use Mask to store Lower address
*/
#define srcIpAddrStart_		L3L4._srcIpAddrMask
#define srcIpAddrEnd_		L3L4._srcIpAddr
#define dstIpAddrStart_		L3L4._dstIpAddrMask
#define dstIpAddrEnd_		L3L4._dstIpAddr

/* IP ACL Rule Definition */
#define ipProto_				L3L4.is.ip._proto
#define ipProtoMask_			L3L4.is.ip._protoMask
#define ipFlagMask_			L3L4.is.ip._flagMask
#define ipFOP_      			L3L4.is.ip._FOP
#define ipFOM_      			L3L4.is.ip._FOM
#define ipHttpFilter_      	L3L4.is.ip._httpFilter
#define ipHttpFilterM_		L3L4.is.ip._httpFilterM
#define ipIdentSrcDstIp_   	L3L4.is.ip._identSrcDstIp
#define ipIdentSrcDstIpM_	L3L4.is.ip._identSrcDstIpM
#define ipFlag_				L3L4.is.ip.un._flag
#define ipDF_				L3L4.is.ip.un.s._DF
#define ipMF_				L3L4.is.ip.un.s._MF

/* ICMP ACL Rule Definition */
#define icmpType_			L3L4.is.icmp._type
#define icmpTypeMask_		L3L4.is.icmp._typeMask	
#define icmpCode_			L3L4.is.icmp._code
#define icmpCodeMask_		L3L4.is.icmp._codeMask

/* IGMP ACL Rule Definition */
#define igmpType_			L3L4.is.igmp._type
#define igmpTypeMask_		L3L4.is.igmp._typeMask

/* TCP ACL Rule Definition */
#define tcpSrcPortUB_		L3L4.is.tcp._srcPortUpperBound
#define tcpSrcPortLB_		L3L4.is.tcp._srcPortLowerBound
#define tcpDstPortUB_		L3L4.is.tcp._dstPortUpperBound
#define tcpDstPortLB_		L3L4.is.tcp._dstPortLowerBound
#define tcpFlagMask_			L3L4.is.tcp._flagMask
#define tcpFlag_				L3L4.is.tcp.un._flag
#define tcpURG_				L3L4.is.tcp.un.s._urg
#define tcpACK_				L3L4.is.tcp.un.s._ack
#define tcpPSH_				L3L4.is.tcp.un.s._psh
#define tcpRST_				L3L4.is.tcp.un.s._rst
#define tcpSYN_				L3L4.is.tcp.un.s._syn
#define tcpFIN_				L3L4.is.tcp.un.s._fin

/* UDP ACL Rule Definition */
#define udpSrcPortUB_		L3L4.is.udp._srcPortUpperBound
#define udpSrcPortLB_		L3L4.is.udp._srcPortLowerBound
#define udpDstPortUB_		L3L4.is.udp._dstPortUpperBound
#define udpDstPortLB_		L3L4.is.udp._dstPortLowerBound

/* Source Filter ACL Rule Definition */
#define srcFilterMac_			SRCFILTER._srcMac
#define srcFilterMacMask_		SRCFILTER._srcMacMask
#define srcFilterPort_			SRCFILTER._srcPort
#define srcFilterPortMask_		SRCFILTER._srcPortMask
#define srcFilterVlanIdx_		SRCFILTER._srcVlanIdx
#define srcFilterVlanId_			SRCFILTER._srcVlanIdx
#define srcFilterVlanIdxMask_	SRCFILTER._srcVlanIdxMask
#define srcFilterVlanIdMask_		SRCFILTER._srcVlanIdxMask
#define srcFilterIpAddr_			SRCFILTER._srcIpAddr
#define srcFilterIpAddrMask_		SRCFILTER._srcIpAddrMask
#define srcFilterIpAddrUB_		SRCFILTER._srcIpAddr
#define srcFilterIpAddrLB_		SRCFILTER._srcIpAddrMask
#define srcFilterPortUpperBound_	SRCFILTER._srcPortUpperBound
#define srcFilterPortLowerBound_	SRCFILTER._srcPortLowerBound
#define srcFilterIgnoreL3L4_		SRCFILTER._ignoreL3L4
#define srcFilterIgnoreL4_		SRCFILTER._ignoreL4

/* Destination Filter ACL Rule Definition */
#define dstFilterMac_			DSTFILTER._dstMac
#define dstFilterMacMask_		DSTFILTER._dstMacMask
#define dstFilterVlanIdx_		DSTFILTER._vlanIdx
#define dstFilterVlanIdxMask_	DSTFILTER._vlanIdxMask
#define dstFilterVlanId_			DSTFILTER._vlanIdx
#define dstFilterVlanIdMask_		DSTFILTER._vlanIdxMask
#define dstFilterIpAddr_			DSTFILTER._dstIpAddr
#define dstFilterIpAddrMask_		DSTFILTER._dstIpAddrMask
#define dstFilterPortUpperBound_	DSTFILTER._dstPortUpperBound
#define dstFilterIpAddrUB_		DSTFILTER._dstIpAddr
#define dstFilterIpAddrLB_		DSTFILTER._dstIpAddrMask
#define dstFilterPortLowerBound_	DSTFILTER._dstPortLowerBound
#define dstFilterIgnoreL3L4_		DSTFILTER._ignoreL3L4
#define dstFilterIgnoreL4_		DSTFILTER._ignoreL4


/* 802.1p */
#define vlanTagPri_			VLANTAG.vlanTagPri

#endif //CONFIG_RTL_LAYERED_DRIVER_ACL

/* ACL Rule Action type Definition */
#define RTL865X_ACL_PERMIT				0x00
#define RTL865X_ACL_REDIRECT_ETHER	0x01
#define RTL865X_ACL_DROP				0x02
#define RTL865X_ACL_TOCPU				0x03
#define RTL865X_ACL_LEGACY_DROP		0x04
#define RTL865X_ACL_DROPCPU_LOG		0x05
#define RTL865X_ACL_MIRROR				0x06
#define RTL865X_ACL_REDIRECT_PPPOE	0x07
#define RTL865X_ACL_DEFAULT_REDIRECT			0x08
#define RTL865X_ACL_MIRROR_KEEP_MATCH		0x09
#define RTL865X_ACL_DROP_RATE_EXCEED_PPS		0x0a
#define RTL865X_ACL_LOG_RATE_EXCEED_PPS		0x0b
#define RTL865X_ACL_DROP_RATE_EXCEED_BPS		0x0c
#define RTL865X_ACL_LOG_RATE_EXCEED_BPS		0x0d
#define RTL865X_ACL_PRIORITY					0x0e

/* ACL Rule type Definition */
#define RTL865X_ACL_MAC				0x00
#define RTL865X_ACL_DSTFILTER_IPRANGE 0x01
#define RTL865X_ACL_IP					0x02
#define RTL865X_ACL_ICMP				0x04
#define RTL865X_ACL_IGMP				0x05
#define RTL865X_ACL_TCP					0x06
#define RTL865X_ACL_UDP				0x07
#define RTL865X_ACL_SRCFILTER			0x08
#define RTL865X_ACL_DSTFILTER			0x09
#define RTL865X_ACL_IP_RANGE			0x0A
#define RTL865X_ACL_SRCFILTER_IPRANGE 0x0B
#define RTL865X_ACL_ICMP_IPRANGE		0x0C
#define RTL865X_ACL_IGMP_IPRANGE 		0x0D
#define RTL865X_ACL_TCP_IPRANGE		0x0E
#define RTL865X_ACL_UDP_IPRANGE		0x0F
#define RTL865X_ACL_802D1P			0x1f /*	dummy acl type for qos	*/






/* For PktOpApp */
#define RTL865X_ACL_ONLY_L2				1 /* Only for L2 switch */
#define RTL865X_ACL_ONLY_L3				2 /* Only for L3 routing (including IP multicast) */
#define RTL865X_ACL_L2_AND_L3			3 /* Only for L2 switch and L3 routing (including IP multicast) */
#define RTL865X_ACL_ONLY_L4				4 /* Only for L4 translation packets */
#define RTL865X_ACL_L3_AND_L4			6 /* Only for L3 routing and L4 translation packets (including IP multicast) */
#define RTL865X_ACL_ALL_LAYER			7 /* No operation. Don't apply this rule. */

#define RTL865X_ACL_MAX_NUMBER		125
//#define RTL865X_ACL_MAX_NUMBER		64
#define RTL865X_ACL_RESERVED_NUMBER	3

#define RTL865X_ACLTBL_ALL_TO_CPU		127  // This rule is always "To CPU"
#define RTL865X_ACLTBL_DROP_ALL		126 //This rule is always "Drop"
#define RTL865X_ACLTBL_PERMIT_ALL		125     // This rule is always "Permit"

#define MAX_IFNAMESIZE 16
#define RTL865XC_NETIFTBL_SIZE			8

#define NETIF_NUMBER 		RTL865XC_NETIFTBL_SIZE
#define NETIF_SW_NUMBER		16

/*invert flag*/
#define RTL865X_INVERT_IN_NETIF	0x01
#define RTL865X_INVERT_OUT_NETIF	0x02

/*ingress or egress flag*/
#define RTL865X_ACL_INGRESS	0 /*ingress acl*/
#define RTL865X_ACL_EGRESS		1 /*egress acl*/



#define RTL_DEV_NAME_NUM(name,num)	name#num


#ifdef CONFIG_RTL_ALIASNAME
#define RTL_BR_NAME ALIASNAME_BR0
#define RTL_WLAN_NAME ALIASNAME_WLAN

#else
#define RTL_BR_NAME "br0"//
#define RTL_WLAN_NAME "wlan"//

#endif


//flowing name in driver DO NOT duplicate
#ifdef CONFIG_RTL_ALIASNAME

#if defined(CONFIG_BRIDGE)
#define RTL_DRV_LAN_NETIF_NAME ALIASNAME_BR0
#else
#define RTL_DRV_LAN_NETIF_NAME ALIASNAME_ETH0
#endif

#ifdef CONFIG_ETHWAN
#ifndef CONFIG_RTL_MULTI_ETH_WAN
#define RTL_DRV_WAN0_NETIF_NAME ALIASNAME_NAS0
#else
#define RTL_DRV_WAN0_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_MWNAS,0)//"nas0_0"
#endif
#else
#define RTL_DRV_WAN0_NETIF_NAME ALIASNAME_ETH1
#endif


#else

    
#if defined(CONFIG_BRIDGE)

#define RTL_DRV_LAN_NETIF_NAME "br0"

#else
#define RTL_DRV_LAN_NETIF_NAME "eth0"

#endif

#ifdef CONFIG_ETHWAN
#ifndef CONFIG_RTL_MULTI_ETH_WAN

#define RTL_DRV_WAN0_NETIF_NAME "nas0"


#else
#define RTL_DRV_WAN0_NETIF_NAME "nas0_0"
#endif

#else

#define RTL_DRV_WAN0_NETIF_NAME "eth1"
#endif

#endif


#ifdef CONFIG_RTL_ALIASNAME

//#define RTL_DRV_WAN1_NETIF_NAME "eth2"
#ifndef CONFIG_RTL_MULTI_ETH_WAN
#define RTL_DRV_PPP_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_PPP,0)//"ppp0"
//#define RTL_DRV_PPP_NETIF_NAME CONFIG_ALIASNAME_PPP

#else
#define RTL_DRV_WAN1_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_MWNAS,1)//"nas0_1"
#define RTL_DRV_WAN2_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_MWNAS,2)//"nas0_2"
#define RTL_DRV_WAN3_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_MWNAS,3)//"nas0_3"
#define RTL_DRV_WAN4_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_MWNAS,4)//"nas0_4"
#define RTL_DRV_WAN5_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_MWNAS,5)//"nas0_5"
#define RTL_DRV_WAN6_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_MWNAS,6)//"nas0_6"

#define RTL_DRV_PPP0_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_PPP,0)//"ppp0"
#define RTL_DRV_PPP1_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_PPP,1)//"ppp1"
#define RTL_DRV_PPP2_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_PPP,2)//"ppp2"
#define RTL_DRV_PPP3_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_PPP,3)//"ppp3"
#define RTL_DRV_PPP4_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_PPP,4)//"ppp4"
#define RTL_DRV_PPP5_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_PPP,5)//"ppp5"
#define RTL_DRV_PPP6_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_PPP,6)//"ppp6"

#endif

#define RTL_DRV_LAN_P0_NETIF_NAME RTL_DRV_LAN_NETIF_NAME
#define RTL_DRV_LAN_P1_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_ETH,2)//"eth2"
#define RTL_DRV_LAN_P2_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_ETH,3)// "eth3"
#define RTL_DRV_LAN_P3_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_ETH,4)// "eth4"
#define RTL_DRV_LAN_P4_NETIF_NAME RTL_DRV_WAN0_NETIF_NAME
#define RTL_DRV_LAN_P5_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_ETH,5)// "eth5"

#else




//#define RTL_DRV_WAN1_NETIF_NAME "eth2"
#ifndef CONFIG_RTL_MULTI_ETH_WAN


#define RTL_DRV_PPP_NETIF_NAME "ppp0"

#else

#define RTL_DRV_WAN1_NETIF_NAME "nas0_1"
#define RTL_DRV_WAN2_NETIF_NAME "nas0_2"
#define RTL_DRV_WAN3_NETIF_NAME "nas0_3"
#define RTL_DRV_WAN4_NETIF_NAME "nas0_4"
#define RTL_DRV_WAN5_NETIF_NAME "nas0_5"
#define RTL_DRV_WAN6_NETIF_NAME "nas0_6"


#define RTL_DRV_PPP0_NETIF_NAME "ppp0"
#define RTL_DRV_PPP1_NETIF_NAME "ppp1"
#define RTL_DRV_PPP2_NETIF_NAME "ppp2"
#define RTL_DRV_PPP3_NETIF_NAME "ppp3"
#define RTL_DRV_PPP4_NETIF_NAME "ppp4"
#define RTL_DRV_PPP5_NETIF_NAME "ppp5"
#define RTL_DRV_PPP6_NETIF_NAME "ppp6"
#endif


#ifdef CONFIG_RTL_HW_PURE_SWITCH
#define RTL_DRV_LAN_P0_NETIF_NAME RTL_DRV_LAN_NETIF_NAME
#define RTL_DRV_LAN_P1_NETIF_NAME "eth1"
#define RTL_DRV_LAN_P2_NETIF_NAME "eth2"
#define RTL_DRV_LAN_P3_NETIF_NAME "eth3"
#define RTL_DRV_LAN_P4_NETIF_NAME RTL_DRV_WAN0_NETIF_NAME
#define RTL_DRV_LAN_P5_NETIF_NAME "eth4"
#else
#define RTL_DRV_LAN_P0_NETIF_NAME RTL_DRV_LAN_NETIF_NAME
#define RTL_DRV_LAN_P1_NETIF_NAME "eth2"
#define RTL_DRV_LAN_P2_NETIF_NAME "eth3"
#define RTL_DRV_LAN_P3_NETIF_NAME "eth4"
#define RTL_DRV_LAN_P4_NETIF_NAME RTL_DRV_WAN0_NETIF_NAME
#define RTL_DRV_LAN_P5_NETIF_NAME "eth5"
#endif
#endif


typedef struct rtl865x_netif_s
{
	uint16 	vid; /*netif->vid*///QL: change vid to vid index.
	uint16 	mtu; /*netif's MTU*/	
	uint32 	if_type:5; /*interface type, 0:ether,1:pppoe....*/			
	ether_addr_t macAddr;
	uint32	is_wan:1, /*wan interface?*/
			dmz:1,	/*DMZ/routing lan*/
			is_slave:1; /*is slave interface?*/
	uint8	name[MAX_IFNAMESIZE];	
	uint16	enableRoute;
#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	uint16	forMacBasedMCast;
#endif		
}rtl865x_netif_t;

/*internal...*/
int32 _rtl865x_getAclFromAsic(int32 index, rtl865x_AclRule_t *rule);

int32 rtl865x_init_acl(void);
int32 rtl865x_reinit_acl(void);
int32 rtl865x_add_acl(rtl865x_AclRule_t *rule, char *netifName,int32 priority
								,int must_add_tail,int allow_change_layout);
int32 rtl865x_del_acl(rtl865x_AclRule_t *rule, char *netifName,int32 priority);
int32 rtl865x_regist_aclChain(char *netifName, int32 priority, uint32 minumun_reserved_acl_num);
int32 rtl865x_aclChain_change_minumun_reserved(char *netifName, int32 priority, uint32 new_minumun_reserved);
int32 rtl865x_unRegist_aclChain(char *netifName, int32 priority);
int32 rtl865x_flush_allAcl_fromChain(char *netifName, int32 priority);
int32 rtl865x_show_allAclChains(void);

int32 rtl865x_setNetifMtu(rtl865x_netif_t *netif);
int32 rtl865x_setNetifMac(rtl865x_netif_t *netif);


int32 rtl865x_addNetif(rtl865x_netif_t *netif);
int32 rtl865x_delNetif(char *ifName);
int32 rtl865x_initNetifTable(void);
int32 rtl865x_reinitNetifTable(void);
int32 rtl865x_attachMasterNetif(char *slave, char *master);
int32 rtl865x_detachMasterNetif(char *slave);

int32 rtl865x_setPortToNetif(char *name,uint32 port);
int32 rtl865x_netifExist(char *ifName);
int32 rtl865x_netif_is_wan(char *ifName);
int32 rtl865x_netif_is_master(char *ifName);
void rtl865x_getMasterNetifByVid(int vid, char* netif_name);


#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
int32 rtl865x_getNetifVid(char *name, uint32 *vid);
int32 rtl865x_getNetifType(char *name,uint32 *type);
uint32 rtl865x_getExternalPortMask(void);
#endif

#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
void rtl865x_showACL(rtl865x_AclRule_t *rule);
void rtl865x_showACL_DataField(rtl865x_AclRuleData_t *ruleData,int rule_type);
int8 rtl865x_sameAclRuleDataField(
	rtl865x_AclRuleData_t *rule1_data, rtl865x_AclRuleData_t *rule2_data, int rule1_datatype, int rule2_datatype);
#endif
#if  defined (CONFIG_RTL_LOCAL_PUBLIC)
//int32 rtl865x_getNetifFid(char *name,  uint16 *fid);
int32 rtl865x_addVirtualNetif(rtl865x_netif_t *netif);
int32 rtl865x_delVirtualNetif(char *ifName);
#endif

#endif


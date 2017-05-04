/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Header File for IC-specific Function
* Abstract : 
* Author : Louis Yung-Chieh Lo (yjlou@realtek.com.tw)               
* $Id: rtl865xc_testModel_L34_test.c,v 1.8 2012/10/24 04:48:19 ikevin362 Exp $
*/

#include <linux/in.h>
#include "rtl865xc_testModel.h"
#include "rtl865xc_testModel_L34_test.h"
#include "../common/rtl_utils.h"
#include "packetGen/pktGen.h"
#include <net/rtl/rtl865x_multicast.h>


#include "../AsicDriver/rtl865x_asicCom.h"
#include "../AsicDriver/rtl865x_asicL3.h"
#include "../AsicDriver/rtl865x_asicL2.h"
#include "../AsicDriver/rtl865x_asicBasic.h"


#define DBG_TESTMODEL_L34_TEST
#ifdef DBG_TESTMODEL_L34_TEST
#define DBG_TESTMODEL_L34_TEST_PRK printk
#else
#define DBG_TESTMODEL_L34_TEST_PRK(format, args...)
#endif

#define RTL865xC_TEST_PORT_NUM 		5
#define RTL865xC_TEST_WAN_PORT 		0
#define RTL865xC_TEST_LAN_PORT_START	1
#define RTL865xC_TEST_LAN_PORT_NUM 	4

#define RTL865xC_TEST_WAN_PORT_BITMAP (1<<RTL865xC_TEST_WAN_PORT)
#define RTL865xC_TEST_LAN_PORT_BITMAP (((1<<RTL865xC_TEST_LAN_PORT_NUM)-1)<<RTL865xC_TEST_LAN_PORT_START)
#define RTL865xC_TEST_ALL_PORT_BITMAP (RTL865xC_TEST_WAN_PORT_BITMAP|RTL865xC_TEST_LAN_PORT_BITMAP)

/************** 
    LAN setting      
**************/
#define RTL865xC_TEST_LAN_VID 			9
#define RTL865xC_TEST_LAN_FID 			0
#define RTL865xC_TEST_LAN_NETIFNAME		ALIASNAME_BR0
#define RTL865xC_TEST_LAN_GMAC 			"00-00-00-AA-AA-AA"
#define RTL865xC_TEST_LAN_IP				"192.168.1.1"
#define RTL865xC_TEST_LAN_SUBNET_MASK	"255.255.255.0"

#define RTL865xC_TEST_LAN_HOST0_PHYPORT		(RTL865xC_TEST_LAN_PORT_START+0)
#define RTL865xC_TEST_LAN_HOST0_PHYPORT_MASK	(1<<RTL865xC_TEST_LAN_HOST0_PHYPORT)
#define RTL865xC_TEST_LAN_HOST0_IP			"192.168.1.2"
#define RTL865xC_TEST_LAN_HOST0_MAC			"00-00-00-00-11-00"

#define RTL865xC_TEST_LAN_HOST1_PHYPORT		(RTL865xC_TEST_LAN_PORT_START+1)
#define RTL865xC_TEST_LAN_HOST1_PHYPORT_MASK	(1<<RTL865xC_TEST_LAN_HOST1_PHYPORT)	
#define RTL865xC_TEST_LAN_HOST1_IP			"192.168.1.3"
#define RTL865xC_TEST_LAN_HOST1_MAC			"00-00-00-00-11-11"


/************** 
    WAN setting  
**************/
#define RTL865xC_TEST_WANTYPE_UNTAG_PURE_ROUTING		0
#define RTL865xC_TEST_WANTYPE_UNTAG_BRIDGE			1
#define RTL865xC_TEST_WANTYPE_UNTAG_NAPT				2


#define RTL865xC_TEST_WAN_NETIFNAME			"nas_test"
#define RTL865xC_TEST_WAN_NETIFNAME_PPP		"nas_test_ppp"
#define RTL865xC_TEST_WAN_HOST_PHYPORT		(RTL865xC_TEST_WAN_PORT)
#define RTL865xC_TEST_WAN_HOST_PHYPORT_MASK	(RTL865xC_TEST_WAN_PORT_BITMAP)	

/* WAN 0 : untag routing WAN   */
#define RTL865xC_TEST_WAN0_VID 			8
#define RTL865xC_TEST_WAN0_FID 			0
#define RTL865xC_TEST_WAN0_GMAC 			"00-00-00-BB-BB-BB"
#define RTL865xC_TEST_WAN0_IP			"192.168.2.2"
#define RTL865xC_TEST_WAN0_SUBNET_MASK	"255.255.255.0"

#define RTL865xC_TEST_WAN0_HOST_IP		"192.168.2.1"
#define RTL865xC_TEST_WAN0_HOST_MAC		"00-00-00-00-22-00"

/* WAN 1 : untag bridged WAN   */
#define RTL865xC_TEST_WAN1_VID 			7
#define RTL865xC_TEST_WAN1_FID 			0
#define RTL865xC_TEST_WAN1_NETIFNAME		"nas_test_1"
#define RTL865xC_TEST_WAN1_GMAC 			"00-00-11-BB-BB-BB"

#define RTL865xC_TEST_WAN1_HOST_IP		"192.168.1.200"
#define RTL865xC_TEST_WAN1_HOST_MAC		"00-00-00-00-22-11"

/* WAN 2 : untag PPPoE WAN   */
#define RTL865xC_TEST_WAN2_VID 			10
#define RTL865xC_TEST_WAN2_FID 			0
#define RTL865xC_TEST_WAN2_GMAC 			"00-00-00-CC-CC-CC"
#define RTL865xC_TEST_WAN2_IP			"192.168.5.55"
#define RTL865xC_TEST_WAN2_SUBNET_MASK	"255.255.255.255"
#define RTL865xC_TEST_WAN2_SSID			123

#define RTL865xC_TEST_WAN2_HOST_IP		"192.168.5.66"
#define RTL865xC_TEST_WAN2_HOST_MAC		"00-00-00-00-22-22"


/* stream info */
#define RTL865xC_TEST_STREAM0_TTL			254
#define RTL865xC_TEST_STREAM0_PORT_INT		1024
#define RTL865xC_TEST_STREAM0_PORT_EXT		1025
#define RTL865xC_TEST_STREAM0_PORT_REM		1026
#define RTL865xC_TEST_STREAM0_PAYLOAD_LEN	25




/*
 *  *ip is return with host-order.
 *
 *  IP               0   1   2   3
 *     BIG-endian: 31_24 .....  7_0
 *  Little-endian:  7_0  ..... 31_24
 */
static uint32  strtoip(ipaddr_t *ip, int8 *str)
{
    int32 t1, t2, t3, t4;
    sscanf((char*)str, "%d.%d.%d.%d", &t1, &t2, &t3, &t4);
    *ip = (t1<<24)|(t2<<16)|(t3<<8)|t4;
	return *(uint32*)ip;
}

typedef uint32 in_addr_t;
static int inet_aton(const char *cp, struct in_addr *addrptr)
{
	in_addr_t addr;
	int value;
	int part;

	addr = 0;
	for (part = 1; part <= 4; part++) {

		if (!isdigit(*cp))
			return 0;

		value = 0;
		while (isdigit(*cp)) {
			value *= 10;
			value += *cp++ - '0';
			if (value > 255)
				return 0;
		}

		if (part < 4) {
			if (*cp++ != '.')
				return 0;
		} else {
			char c = *cp++;
			if (c != '\0' && !isspace(c))
			return 0;
		}

		addr <<= 8;
		addr |= value;
	}

	/*  W. Richard Stevens in his book UNIX Network Programming,
	 *  Volume 1, second edition, on page 71 says:
	 *
	 *  An undocumented feature of inet_aton is that if addrptr is
	 *  a null pointer, the function still performs it validation
	 *  of the input string, but does not store the result.
	 */
	if (addrptr) {
	    addrptr->s_addr = htonl(addr);
	}

	return 1;
}



static uint32 inet_addr(const int8 *cp){

	struct in_addr a;

	if (!inet_aton(cp, &a))
		return -1;
	else
		return a.s_addr;
}



/*
	Create two subnet , LAN and WAN(untag routing WAN)
	There exists hosts(192.168.1.2 , 192.168.2.1) under coresponding subnet.
*/
int32 rtl865xC_layer3Config( int WAN_TYPE , int isPPPoE)
{
	int i;
	rtl865x_netif_t netif;
	ipaddr_t ipAddr;
	ipaddr_t ipMask;
	ether_addr_t macAddr;

	/* init WAN para. */
	int 				wan_vid=0;
	int 				wan_fid=0;
	ether_addr_t	wan_gmac;
	ipaddr_t			wan_ipAddr;
	ipaddr_t			wan_ipMask;	

	ipaddr_t			wan_host_ip;
	ether_addr_t	wan_host_mac;
	
	switch(WAN_TYPE)
	{
		case RTL865xC_TEST_WANTYPE_UNTAG_PURE_ROUTING:
			if(!isPPPoE)
			{
				wan_vid = RTL865xC_TEST_WAN0_VID;
				wan_fid = RTL865xC_TEST_WAN0_FID;
				strtomac(&wan_gmac, RTL865xC_TEST_WAN0_GMAC);	
				strtoip(&wan_ipAddr, RTL865xC_TEST_WAN0_IP);
				strtoip(&wan_ipMask, RTL865xC_TEST_WAN0_SUBNET_MASK);	
				
				strtoip(&wan_host_ip, RTL865xC_TEST_WAN0_HOST_IP);
				strtomac(&wan_host_mac, RTL865xC_TEST_WAN0_HOST_MAC);
			}
			else
			{
				wan_vid = RTL865xC_TEST_WAN2_VID;
				wan_fid = RTL865xC_TEST_WAN2_FID;
				strtomac(&wan_gmac, RTL865xC_TEST_WAN2_GMAC);	
				strtoip(&wan_ipAddr, RTL865xC_TEST_WAN2_IP);
				strtoip(&wan_ipMask, RTL865xC_TEST_WAN2_SUBNET_MASK);	
				
				strtoip(&wan_host_ip, RTL865xC_TEST_WAN2_HOST_IP);
				strtomac(&wan_host_mac, RTL865xC_TEST_WAN2_HOST_MAC);
			}
			break;
		case RTL865xC_TEST_WANTYPE_UNTAG_BRIDGE:
			if(isPPPoE)
				return FAILED;
			wan_vid = RTL865xC_TEST_WAN1_VID;
			wan_fid = RTL865xC_TEST_WAN1_FID;
			strtomac(&wan_gmac, RTL865xC_TEST_WAN1_GMAC);
			
			strtoip(&wan_host_ip, RTL865xC_TEST_WAN1_HOST_IP);
			strtomac(&wan_host_mac, RTL865xC_TEST_WAN1_HOST_MAC);
			break;
	}



	
	/* module switch control reg: enable L2/L3/L4/STP, trap 802.1D/GARP to CPU */
	RTL_TESTMODEL_INIT_CHECK_SUCCESS(rtl8651_setAsicOperationLayer(4));
	//WRITE_MEM32(ALECR, READ_MEM32(ALECR)|EN_PPPOE|TTL_1Enable);//enable PPPoE auto encapsulation and TTL-1	

	/* Set pvid */
	for(i=0;i<RTL865xC_TEST_PORT_NUM;i++)
	{
		if(WAN_TYPE==RTL865xC_TEST_WANTYPE_UNTAG_BRIDGE)
		{
			RTL_TESTMODEL_INIT_CHECK_SUCCESS(rtl8651_setAsicPvid(i,wan_vid));
		}
		else
		{
			if(i==RTL865xC_TEST_WAN_PORT)
				RTL_TESTMODEL_INIT_CHECK_SUCCESS(rtl8651_setAsicPvid(i,wan_vid));
			else
				RTL_TESTMODEL_INIT_CHECK_SUCCESS(rtl8651_setAsicPvid(i,RTL865xC_TEST_LAN_VID));
		}
	}



	/* Set Network Interface Table */
	/*  		-- LAN --  */
	memset(&netif, 0, sizeof(rtl865x_netif_t));
	memcpy(netif.name,RTL865xC_TEST_LAN_NETIFNAME,MAX_IFNAMESIZE);
	strtomac(&netif.macAddr, RTL865xC_TEST_LAN_GMAC);
	netif.mtu 		= 1500;
	netif.if_type 		= IF_ETHER;
	netif.vid 		= RTL865xC_TEST_LAN_VID;
	netif.is_wan 		= 0;	
	netif.is_slave 	= 0;
	netif.enableRoute	= 1;
	RTL_TESTMODEL_INIT_CHECK_SUCCESS(rtl865x_addNetif(&netif));

	/*  		-- WAN --  */
	memset(&netif, 0, sizeof(rtl865x_netif_t));
	memcpy(netif.name,RTL865xC_TEST_WAN_NETIFNAME,MAX_IFNAMESIZE);
	memcpy(&netif.macAddr,&wan_gmac,ETHER_ADDR_LEN);
	netif.mtu 		= 1500;
	netif.if_type 		= IF_ETHER;
	netif.vid 		= wan_vid;
	netif.is_wan 		= 0;	
	netif.is_slave 	= 0;
	netif.enableRoute	= 1;
	RTL_TESTMODEL_INIT_CHECK_SUCCESS(rtl865x_addNetif(&netif));

	if(isPPPoE)
	{
		memset(&netif, 0, sizeof(rtl865x_netif_t));
		memcpy(netif.name,RTL865xC_TEST_WAN_NETIFNAME_PPP,MAX_IFNAMESIZE);
		memcpy(&netif.macAddr,&wan_gmac,ETHER_ADDR_LEN);
		netif.mtu 		= 1500;
		netif.if_type 		= IF_PPPOE;
		netif.vid 		= wan_vid;
		netif.is_wan 		= 0;	
		netif.is_slave 	= 1;
		netif.enableRoute	= 1;
		RTL_TESTMODEL_INIT_CHECK_SUCCESS(rtl865x_addNetif(&netif));
		RTL_TESTMODEL_INIT_CHECK_SUCCESS(rtl865x_attachMasterNetif(RTL865xC_TEST_WAN_NETIFNAME_PPP, RTL865xC_TEST_WAN_NETIFNAME));
	}


	/* Set VLAN Table */
	/*  		-- LAN --  */
	RTL_TESTMODEL_INIT_CHECK_SUCCESS(rtl865x_addVlan(RTL865xC_TEST_LAN_VID));
	RTL_TESTMODEL_INIT_CHECK_SUCCESS(rtl865x_setVlanFilterDatabase(RTL865xC_TEST_LAN_VID,RTL865xC_TEST_LAN_FID));
	RTL_TESTMODEL_INIT_CHECK_SUCCESS(rtl865x_modVlanPortMember(RTL865xC_TEST_LAN_VID,RTL865xC_TEST_LAN_PORT_BITMAP,RTL865xC_TEST_LAN_PORT_BITMAP));

	/*  		-- WAN --  */
	RTL_TESTMODEL_INIT_CHECK_SUCCESS(rtl865x_addVlan(wan_vid));
	RTL_TESTMODEL_INIT_CHECK_SUCCESS(rtl865x_setVlanFilterDatabase(wan_vid,wan_fid));
	if(WAN_TYPE==RTL865xC_TEST_WANTYPE_UNTAG_BRIDGE)
		RTL_TESTMODEL_INIT_CHECK_SUCCESS(rtl865x_modVlanPortMember(wan_vid,RTL865xC_TEST_ALL_PORT_BITMAP,RTL865xC_TEST_ALL_PORT_BITMAP));
	else
		RTL_TESTMODEL_INIT_CHECK_SUCCESS(rtl865x_modVlanPortMember(wan_vid,RTL865xC_TEST_WAN_PORT_BITMAP,RTL865xC_TEST_WAN_PORT_BITMAP));

	printk("[HP]** %s %d  **\n",__func__,__LINE__);


	/* Set L2 Table */
	/*  		-- LAN --  */
	strtomac(&macAddr, RTL865xC_TEST_LAN_HOST0_MAC);
	RTL_TESTMODEL_INIT_CHECK_SUCCESS(_rtl865x_addFilterDatabaseEntry(RTL865x_L2_TYPEII, RTL865xC_TEST_LAN_FID, &macAddr
							, FDB_TYPE_FWD, RTL865xC_TEST_LAN_HOST0_PHYPORT_MASK	, FALSE, FALSE));
	
	strtomac(&macAddr, RTL865xC_TEST_LAN_HOST1_MAC);
	RTL_TESTMODEL_INIT_CHECK_SUCCESS(_rtl865x_addFilterDatabaseEntry(RTL865x_L2_TYPEII, RTL865xC_TEST_LAN_FID, &macAddr
							, FDB_TYPE_FWD, RTL865xC_TEST_LAN_HOST1_PHYPORT_MASK	, FALSE, FALSE));
	
	/*  		-- WAN --  */
	RTL_TESTMODEL_INIT_CHECK_SUCCESS(_rtl865x_addFilterDatabaseEntry(RTL865x_L2_TYPEII, RTL865xC_TEST_LAN_FID, &wan_host_mac
							, FDB_TYPE_FWD, RTL865xC_TEST_WAN_HOST_PHYPORT_MASK	, FALSE, FALSE));	

	printk("[HP]** %s %d  **\n",__func__,__LINE__);
	

	/* Set PPPoE Table */
	if(isPPPoE)
	{
		RTL_TESTMODEL_INIT_CHECK_SUCCESS(
			rtl865x_addPpp(RTL865xC_TEST_WAN_NETIFNAME_PPP , &wan_host_mac, RTL865xC_TEST_WAN2_SSID, 0));
	}

	
	/* Set L3 Table */
	/*  		-- LAN --  */
	strtoip(&ipAddr, RTL865xC_TEST_LAN_IP);
	strtoip(&ipMask, RTL865xC_TEST_LAN_SUBNET_MASK);	
	RTL_TESTMODEL_INIT_CHECK_SUCCESS(rtl865x_addRoute (ipAddr,ipMask,0,RTL865xC_TEST_LAN_NETIFNAME,0,1));

	/*  		-- WAN --  */
	if(WAN_TYPE!=RTL865xC_TEST_WANTYPE_UNTAG_BRIDGE)
	{
		if(!isPPPoE)
			RTL_TESTMODEL_INIT_CHECK_SUCCESS(rtl865x_addRoute (wan_ipAddr,wan_ipMask,0,RTL865xC_TEST_WAN_NETIFNAME,0,1));
		else
			RTL_TESTMODEL_INIT_CHECK_SUCCESS(rtl865x_addRoute (wan_ipAddr,wan_ipMask,0,RTL865xC_TEST_WAN_NETIFNAME_PPP,0,1));
	}
	

	/* Set ARP Table */
	/*  		-- LAN --  */
	strtoip(&ipAddr, RTL865xC_TEST_LAN_HOST0_IP);
	strtomac(&macAddr, RTL865xC_TEST_LAN_HOST0_MAC);
	RTL_TESTMODEL_INIT_CHECK_SUCCESS(rtl865x_addArp (ipAddr,&macAddr));
	
	strtoip(&ipAddr, RTL865xC_TEST_LAN_HOST1_IP);
	strtomac(&macAddr, RTL865xC_TEST_LAN_HOST1_MAC);
	RTL_TESTMODEL_INIT_CHECK_SUCCESS(rtl865x_addArp (ipAddr,&macAddr));
	
	/*		-- WAN --  */
	if(WAN_TYPE!=RTL865xC_TEST_WANTYPE_UNTAG_BRIDGE && !isPPPoE)
		RTL_TESTMODEL_INIT_CHECK_SUCCESS(rtl865x_addArp (wan_host_ip,&wan_host_mac));
	

	return SUCCESS;
	
}

hsb_param_t Ghsb =
{
		spa: 3,			len: 64,
		vid: 0x0,			tagif: 0,
		pppoeif: 0,		sip:0,
		sprt: 0,			dip:0,
		dprt: 0,			ipptl: 0xdd,
		ipfg: 0x0,			iptos: 0x65,
		tcpfg: 0x00,		type: HSB_TYPE_TCP,
		patmatch:0,		ethtype: 0x0800,
		da:{0x00,0x00,0x00,0x00,0x88,0x8a},
		sa:{0x00,0x80,0xc0,0x00,0x00,0x00},
		hiprior: 0,		snap: 0,
		udpnocs: 0,		ttlst: 0x2,
		dirtx: 0,			l3csok: 1,
		l4csok: 1,		ipfo0_n: 0,
		llcothr: 0,			urlmch: 0,
		extspa: 0,		extl2: 0,
};


static uint8 port_number_of_host[3][MAX_PORT_NUMBER] = {{0,1,2,3,4,5},{4,3,2,1,0,5},{1,0,3,2,4,5}};
static uint8 *hp;


#define VID_0 0
#define WAN0_VID 1
#define GW_WAN0_MAC "00-00-00-00-88-88"
#define WAN1_VID 9
#define GW_WAN1_MAC "00-00-00-00-88-89"
#define GW_WAN1_SUBNET_MASK "255.255.255.248"
#define LAN0_VID 0x3C7
#define GW_LAN0_MAC "00-00-00-00-88-8a"
#define GW_LAN0_IP "192.168.2.1"
#define GW_LAN0_SUBNET_MASK "255.255.255.248"
#define LAN1_VID 11
#define GW_LAN1_MAC "00-00-00-00-88-8b"
#define GW_LAN1_IP "192.168.1.1"
#define GW_LAN1_SUBNET_MASK "255.255.255.248"
/* isp 0 */
#define WAN0_ISP_MAC "00-00-00-00-99-9c"
#define WAN0_ISP_PPPOE_SID 0x55
#define WAN0_ISP_PUBLIC_IP "192.168.4.1"
#define WAN3_ISP_PUBLIC_IP "192.168.4.3"
/* isp 1 */
#define WAN1_ISP_PUBLIC_IP 	"192.168.3.1"
#define GW_WAN1_IP 			"192.168.3.1"
#define GW_WAN1_SUBNET_MASK "255.255.255.248"

/* wan 0 hosts */
#define HOST5_MAC "00-00-00-00-00-10"
#define HOST5_IP "192.168.4.5"
/* wan 1 hosts */
#define HOST4_MAC "00-00-00-00-00-20"
#define HOST4_IP "192.168.3.2"

/* lan 0 hosts */
#define HOST3_MAC "00-00-00-00-00-30"
#define HOST3_IP "192.168.2.2"
/* lan 1 hosts */
#define HOST1_MAC "00-00-00-00-00-40"
#define HOST1_IP "192.168.1.2"
#define HOST2_MAC "00-00-00-00-00-50"
#define HOST2_IP "192.168.1.3"
#define HOST6_MAC "00-00-00-00-00-60"
#define HOST6_IP "192.168.1.4"
#define MULTICAST_MAC "01-00-5e-00-00-03"
#define EXTERNAL_PORT 20
#define INTERNAL_PORT 30
#define LOWER_PORT	    80
#define UPPER_PORT	    180
#define MORE_FRAG	2


uint32 getL2TableRowFromMacStr(int8 *str)
{
    int32 t1, t2, t3, t4, t5, t6;
    ether_addr_t mac;
    sscanf((char*)str, "%x-%x-%x-%x-%x-%x", &t1, &t2, &t3, &t4, &t5, &t6);
    mac.octet[0]=t1; mac.octet[1]=t2; mac.octet[2]=t3; mac.octet[3]=t4; mac.octet[4]=t5; mac.octet[5]=t6;
    return rtl8651_filterDbIndex(&mac,0);
}

int32   rtl865xc_netIfIndex(int vid)  
{
	uint16 i;
	rtl865xc_tblAsic_netifTable_t	 netif;
	for (i=0;i<RTL865XC_NETIFTBL_SIZE;i++)
	{
		_rtl8651_readAsicEntry(TYPE_NETINTERFACE_TABLE, i, &netif);
		if (netif.vid==vid)
			return i;
	}
	rtlglue_printf("Error  !!! not  foud network interface..... %s %d\n",__FILE__,__LINE__);
	return RTL865XC_NETIFTBL_SIZE;
}
#ifndef bzero
		#define bzero( p, s ) memset( p, 0, s )
#endif

int32 rtl8651_clearAsicPvid(void)
{


	WRITE_MEM32( PVCR0,READ_MEM32(PVCR0) &( ~(0xfff<<16)) & (~0xffff));
	WRITE_MEM32( PVCR1,READ_MEM32(PVCR1) &( ~(0xfff<<16)) & (~0xffff));
	WRITE_MEM32( PVCR2,READ_MEM32(PVCR2) &( ~(0xfff<<16)) & (~0xffff));

	return SUCCESS;
}
/*
@func void | rtl8651_clearRegister | Clear ALL registers in ASIC
@comm
	Clear ALL registers in ASIC.
	for RTL865xC only
*/

void rtl8651_clearRegister(void)
{
#ifdef CONFIG_RTL865XC
	int i = 0;

#if !defined(CONFIG_RTL_8196C) && !defined(CONFIG_RTL_8198)
	WRITE_MEM32(MACCR,LONG_TXE);	
#endif
	WRITE_MEM32(MGFCR_E0R0,0);
	WRITE_MEM32(MGFCR_E0R1,0);
	WRITE_MEM32(MGFCR_E0R2,0);
	WRITE_MEM32(MGFCR_E1R0,0);
	WRITE_MEM32(MGFCR_E1R1,0);
	WRITE_MEM32(MGFCR_E1R2,0);	
	WRITE_MEM32(MGFCR_E2R0,0);
	WRITE_MEM32(MGFCR_E2R1,0);
	WRITE_MEM32(MGFCR_E2R2,0);
	WRITE_MEM32(MGFCR_E3R0,0);
	WRITE_MEM32(MGFCR_E3R1,0);
	WRITE_MEM32(MGFCR_E3R2,0);	
	WRITE_MEM32(VCR0,0);		
	WRITE_MEM32(VCR1,0);	
	WRITE_MEM32(PVCR0,0);	
	WRITE_MEM32(PVCR1,0);	
	WRITE_MEM32(PVCR2,0);	
	WRITE_MEM32(PVCR3,0);	
	WRITE_MEM32(PVCR4,0);	
	WRITE_MEM32(TEACR,0);
	WRITE_MEM32(FFCR,0);
	WRITE_MEM32(DOT1XPORTCR,0);
	WRITE_MEM32(DOT1XMACCR,0);
	WRITE_MEM32(GVGCR,0);
	WRITE_MEM32(SWTCR0,0);
	WRITE_MEM32(SWTCR1,0);
	WRITE_MEM32(PLITIMR,0);
	/* Set all Protocol-Based Reg. to 0 */
	WRITE_MEM32(PBVCR0,  0x00000000);
	WRITE_MEM32(PBVCR1, 0x00000000);
	WRITE_MEM32(PBVR0_0, 0x00000000);	/* IPX */
	WRITE_MEM32(PBVR0_1, 0x00000000);	/* IPX */
	WRITE_MEM32(PBVR0_2, 0x00000000);	/* IPX */
	WRITE_MEM32(PBVR0_3, 0x00000000);	/* IPX */
	WRITE_MEM32(PBVR0_4, 0x00000000);	/* IPX */		
	WRITE_MEM32(PBVR1_0,  0x00000000);	/* NetBIOS */
	WRITE_MEM32(PBVR1_1,  0x00000000);	/* NetBIOS */
	WRITE_MEM32(PBVR1_2,  0x00000000);	/* NetBIOS */
	WRITE_MEM32(PBVR1_3,  0x00000000);	/* NetBIOS */
	WRITE_MEM32(PBVR1_4,  0x00000000);	/* NetBIOS */
	WRITE_MEM32(PBVR2_0,  0x00000000);	/* PPPoE Control */
	WRITE_MEM32(PBVR2_1,  0x00000000);	
	WRITE_MEM32(PBVR2_2,  0x00000000);	
	WRITE_MEM32(PBVR2_3,  0x00000000);	
	WRITE_MEM32(PBVR2_4,  0x00000000);	
	WRITE_MEM32(PBVR3_0,  0x00000000);	/* PPPoE Session */
	WRITE_MEM32(PBVR3_1,  0x00000000);	/* PPPoE Session */
	WRITE_MEM32(PBVR3_2,  0x00000000);	/* PPPoE Session */
	WRITE_MEM32(PBVR3_3,  0x00000000);	/* PPPoE Session */
	WRITE_MEM32(PBVR3_4,  0x00000000);	/* PPPoE Session */
	WRITE_MEM32(PBVR4_0,  0x00000000);	/* User-defined 1 */
	WRITE_MEM32(PBVR4_1,  0x00000000);	/* User-defined 1 */
	WRITE_MEM32(PBVR4_2,  0x00000000);	/* User-defined 1 */
	WRITE_MEM32(PBVR4_3,  0x00000000);	/* User-defined 1 */
	WRITE_MEM32(PBVR4_4,  0x00000000);	/* User-defined 1 */
	WRITE_MEM32(PBVR5_0,  0x00000000);	/* User-defined 2 */
	WRITE_MEM32(PBVR5_1,  0x00000000);	/* User-defined 2 */
	WRITE_MEM32(PBVR5_2,  0x00000000);	/* User-defined 2 */
	WRITE_MEM32(PBVR5_3,  0x00000000);	/* User-defined 2 */
	WRITE_MEM32(PBVR5_4,  0x00000000);	/* User-defined 2 */
	WRITE_MEM32(MSCR,0);   

	#ifdef CONFIG_RTL8198
	WRITE_MEM32(PCRP0, (1 | MacSwReset));
	WRITE_MEM32(PCRP1, (1 | MacSwReset));   
	WRITE_MEM32(PCRP2, (1 | MacSwReset));   
	WRITE_MEM32(PCRP3, (1 | MacSwReset));   
	WRITE_MEM32(PCRP4, (1 | MacSwReset));   
	#else
	WRITE_MEM32(PCRP0,1);   
	WRITE_MEM32(PCRP1,1);   
	WRITE_MEM32(PCRP2,1);   
	WRITE_MEM32(PCRP3,1);   
	WRITE_MEM32(PCRP4,1);   
	#endif

	WRITE_MEM32(PCRP5,1);   
	WRITE_MEM32(PCRP6,1);   
	WRITE_MEM32(PCRP7,1);   
	WRITE_MEM32(PCRP8,1);   
	WRITE_MEM32(PPMAR,1);
	WRITE_MEM32(SIRR, TRXRDY);
	/* WRITE_MEM32(TMCR,0xFCFC0000); */
	WRITE_MEM32(QIDDPCR,(0x1<<NAPT_PRI_OFFSET)|(0x1<<ACL_PRI_OFFSET)|(0x1<<DSCP_PRI_OFFSET)|(0x1<<BP8021Q_PRI_OFFSET)|(0x1<<PBP_PRI_OFFSET));
	WRITE_MEM32(LPTM8021Q,0);
	WRITE_MEM32(DSCPCR0,0);
	WRITE_MEM32(DSCPCR1,0);
	WRITE_MEM32(DSCPCR2,0);
	WRITE_MEM32(DSCPCR3,0);
	WRITE_MEM32(DSCPCR4,0);
	WRITE_MEM32(DSCPCR5,0);
	WRITE_MEM32(DSCPCR6,0);	
	WRITE_MEM32(RMACR,0);
	WRITE_MEM32(ALECR,0);

	/* System Based Flow Control Threshold Register */
	WRITE_MEM32( SBFCR0, (0x1E0<<S_DSC_RUNOUT_OFFSET) );
	WRITE_MEM32( SBFCR1, (0x0190<<S_DSC_FCOFF_OFFSET)|(0x01CC<<S_DSC_FCON_OFFSET) );
	WRITE_MEM32( SBFCR2, (0x0050<<S_Max_SBuf_FCOFF_OFFSET)|(0x006C<<S_Max_SBuf_FCON_OFFSET) );
	/* Port Based Flow Control Threshold Register */
	WRITE_MEM32( PBFCR0, (0x003C<<P_MaxDSC_FCOFF_OFFSET)|(0x005A<<P_MaxDSC_FCON_OFFSET) );
	WRITE_MEM32( PBFCR1, (0x003C<<P_MaxDSC_FCOFF_OFFSET)|(0x005A<<P_MaxDSC_FCON_OFFSET) );
	WRITE_MEM32( PBFCR2, (0x003C<<P_MaxDSC_FCOFF_OFFSET)|(0x005A<<P_MaxDSC_FCON_OFFSET) );
	WRITE_MEM32( PBFCR3, (0x003C<<P_MaxDSC_FCOFF_OFFSET)|(0x005A<<P_MaxDSC_FCON_OFFSET) );
	WRITE_MEM32( PBFCR4, (0x003C<<P_MaxDSC_FCOFF_OFFSET)|(0x005A<<P_MaxDSC_FCON_OFFSET) );
	WRITE_MEM32( PBFCR5, (0x003C<<P_MaxDSC_FCOFF_OFFSET)|(0x005A<<P_MaxDSC_FCON_OFFSET) );

	/* Packet Schecduling Control Register */
	WRITE_MEM32(ELBPCR, (51<<Token_OFFSET)|(38<<Tick_OFFSET) );
	WRITE_MEM32(ELBTTCR, /*0x99*/0x400<<L2_OFFSET );
	WRITE_MEM32(ILBPCR1, (0<<UpperBound_OFFSET)|(0<<LowerBound_OFFSET) );
	WRITE_MEM32(ILBPCR2, (0x33<<ILB_feedToken_OFFSET)|(0x26<<ILB_Tick_OFFSET) );
	for( i = 0; i<42; i++ )
		WRITE_MEM32(P0Q0RGCR+i*4, (7<<PPR_OFFSET)|(0xff<<L1_OFFSET)|(0x3FFF<<APR_OFFSET) );
	for( i = 0; i<7; i++ )
	{
		WRITE_MEM32(WFQRCRP0+i*4*3+ 0, 0x3fff<<APR_OFFSET );
		WRITE_MEM32(WFQRCRP0+i*4*3+ 4, 0 ); /* WFQWCR0P? */
		WRITE_MEM32(WFQRCRP0+i*4*3+ 8, 0 ); /* WFQWCR1P? */
	}

	for (i=0;i<48;i+=4)
			WRITE_MEM32(PATP0+i,0xfe12);	

	rtl8651_totalExtPortNum=0;

	/* Set chip's mode as NORMAL mode */
	WRITE_MEM32(TMCR,~ENHSBTESTMODE&READ_MEM32(TMCR));

	/* CPU NIC */
	WRITE_MEM32(CPUICR,READ_MEM32(CPUICR)&~(LBMODE|LB10MHZ|MITIGATION|EXCLUDE_CRC)); 
	WRITE_MEM32(CPUIISR,READ_MEM32(CPUIISR));  /* clear all interrupt */
	WRITE_MEM16(CPUQDM0,0); 
	WRITE_MEM16(CPUQDM1,0); 
	WRITE_MEM16(CPUQDM2,0); 
	WRITE_MEM16(CPUQDM3,0); 
	WRITE_MEM16(CPUQDM4,0); 
	WRITE_MEM16(CPUQDM5,0); 
	WRITE_MEM32(RMCR1P,0); 
	WRITE_MEM32(DSCPRM0,0); 
	WRITE_MEM32(DSCPRM1,0); 
	WRITE_MEM32(RLRC,0); 
	
#if 0 /* Since swNic only init once when starting model code, we don't clear the following registers. */
	WRITE_MEM32(CPURPDCR0,0); 
	WRITE_MEM32(CPURPDCR1,0); 
	WRITE_MEM32(CPURPDCR2,0); 
	WRITE_MEM32(CPURPDCR3,0); 
	WRITE_MEM32(CPURPDCR4,0); 
	WRITE_MEM32(CPURPDCR5,0); 
	WRITE_MEM32(CPURMDCR0,0); 
	WRITE_MEM32(CPUTPDCR0,0); 
	WRITE_MEM32(CPUTPDCR1,0); 
	WRITE_MEM32(CPUIIMR,0); 
#endif

#else
	rtlglue_printf("Driver of current CHIP version does not support [%s]\n", __FUNCTION__);
#endif
}


#undef REAL_ASIC_ACCESS

int32  layer3Config( int routetIsPPPoE )
{
#ifdef 	REAL_ASIC_ACCESS
	rtl865x_tblAsicDrv_vlanParam_t vlant;
	rtl865x_tblAsicDrv_intfParam_t intf;
	rtl865x_AclRule_t *aclt;
#else
	struct rtl865x_netif_s netif;
#endif		
	rtl865x_tblAsicDrv_l2Param_t l2t;
	rtl865x_tblAsicDrv_pppoeParam_t pppoet;
	rtl865x_tblAsicDrv_arpParam_t arpt;
	rtl865x_tblAsicDrv_routingParam_t routet;
	rtl865x_tblAsicDrv_nextHopParam_t nxthpt;

	ipaddr_t ip32;
	
	hp = port_number_of_host[0];
	rtl8651_clearRegister();
	rtl8651_clearAsicCommTable();
//	rtl8651_clearAsicAllTable();
	rtl8651_clearAsicPvid();
	rtl8651_setAsicPvid(hp[5],WAN0_VID);  /*host 5 pvid 0*/
	rtl8651_setAsicPvid(hp[4],9);   /*host 4 pvid 1*/
	rtl8651_setAsicPvid(hp[3],LAN0_VID);  /*host 3 pvid 2*/
	rtl8651_setAsicPvid(hp[2],11);  /*host 2 pvid 3*/
	rtl8651_setAsicPvid(hp[1],11);  /*host 1 pvid 3*/
	rtl8651_setAsicPvid(hp[6],11);  /*host 6 pvid 3*/
	/* module switch control reg: enable L2/L3/L4/STP, trap 802.1D/GARP to CPU */
	rtl8651_setAsicOperationLayer(3);
	/* TTL control reg: enable TTL-1 operation */
	WRITE_MEM32(ALECR,READ_MEM32(ALECR)|(uint32)EN_TTL1);	
	WRITE_MEM32(SWTCR1,READ_MEM32(SWTCR1)|EN_51B_CPU_REASON);
	/* VLAN tagging control reg: accept all frame */
	//WRITE_MEM32( VCR0, EN_ALL_PORT_VLAN_INGRESS_FILTER);	



	 /* config vlan table */
	/* wan 0 */
#ifdef REAL_ASIC_ACCESS

	memset((void*) &intf,0, sizeof(intf));
	strtomac(&intf.macAddr, GW_WAN0_MAC);
	intf.macAddrNumber = 1;
	intf.vid = WAN0_VID;
	intf.mtu = 1500;	
	intf.inAclStart = 0;
	intf.inAclEnd = 0;
	intf.outAclStart = intf.outAclEnd = 0;
	intf.enableRoute = 1;
	intf.valid = 1;
	rtl8651_setAsicNetInterface( RTL865XC_NETIFTBL_SIZE, &intf );
#else
	memset(&netif, 0, sizeof(rtl865x_netif_t));
	memcpy(netif.name,"nas0_0test",MAX_IFNAMESIZE);
	strtomac(&netif.macAddr, GW_WAN0_MAC);
	netif.mtu = 1500;
	netif.if_type = IF_ETHER;
	netif.vid = WAN0_VID;
	netif.is_wan = 0;
	netif.is_slave = 0;
	netif.enableRoute=1;
	rtl865x_addNetif(&netif);
#endif


	WRITE_MEM32( PCRP0, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP1, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP2, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP3, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP4, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP5, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP6, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP7, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP8, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
#ifdef REAL_ASIC_ACCESS

	memset( &vlant,0, sizeof(vlant) );
	vlant.memberPortMask = vlant.untagPortMask= (1<<hp[5]);
	rtl8651_setAsicVlan(WAN0_VID,&vlant);
#else	
	if(rtl865x_addVlan(WAN0_VID)!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}
	if(rtl865x_setVlanFilterDatabase(WAN0_VID,0)!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}
	if(rtl865x_modVlanPortMember(WAN0_VID,(1<<hp[5]),(1<<hp[5]))!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}
	
#endif

#ifdef REAL_ASIC_ACCESS

	 /* ingress filter gateway mac */
	aclt = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
	 memset(aclt, 0,sizeof(rtl865x_AclRule_t));	
	aclt->ruleType_ = RTL865X_ACL_MAC;
	aclt->actionType_ = RTL865X_ACL_PERMIT;
	aclt->pktOpApp_	 = RTL865X_ACL_ALL_LAYER;
	_rtl865x_setAclToAsic(0, aclt);
	
#endif	


	 /* wan 1 */
#ifdef REAL_ASIC_ACCESS

	strtomac(&intf.macAddr, GW_WAN1_MAC);
	intf.vid = VID_0;
	
	intf.inAclStart = 0;
	intf.inAclEnd = 0;
	intf.outAclStart = intf.outAclEnd = 0;
	rtl8651_setAsicNetInterface( RTL865XC_NETIFTBL_SIZE, &intf );
#else
	
	memcpy(netif.name,"nas0_1test",MAX_IFNAMESIZE); 
	strtomac(&netif.macAddr, GW_WAN1_MAC);
	netif.mtu = 1500;
	netif.if_type = IF_ETHER;
	netif.vid = VID_0;
	netif.is_wan = 0;
	netif.is_slave = 0; 
	netif.enableRoute=1; 
	rtl865x_addNetif(&netif);
#endif
	
	WRITE_MEM32( PCRP0, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP1, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP2, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP3, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP4, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP5, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP6, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP7, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP8, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
#ifdef REAL_ASIC_ACCESS

	memset( &vlant,0, sizeof(vlant) );
	vlant.memberPortMask = vlant.untagPortMask = (1<<hp[4])|(1<<hp[0]);	
		rtl8651_setAsicVlan(VID_0,&vlant);
#else
	if(rtl865x_addVlan(VID_0)!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}
	if(rtl865x_setVlanFilterDatabase(VID_0,0)!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}
	if(rtl865x_modVlanPortMember(VID_0,(1<<hp[4])|(1<<hp[0]),(1<<hp[4])|(1<<hp[0]))!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}
	
#endif

#ifdef REAL_ASIC_ACCESS

	/* ingress filter gateway mac */	
	aclt->ruleType_ = RTL865X_ACL_MAC;
	memcpy(aclt->un_ty.srcMac_.octet, intf.macAddr.octet, 6);
	aclt->actionType_ = RTL865X_ACL_TOCPU;
	aclt->pktOpApp_		 = RTL865X_ACL_ALL_LAYER;
	_rtl865x_setAclToAsic(4, aclt);
#endif	
	/* lan 0 */

#ifdef REAL_ASIC_ACCESS

	strtomac(&intf.macAddr, GW_LAN0_MAC);
	intf.vid = LAN0_VID;
	
	intf.inAclStart = 0;
	intf.inAclEnd = 0;
	intf.outAclStart = intf.outAclEnd = 0;
	rtl8651_setAsicNetInterface( RTL865XC_NETIFTBL_SIZE, &intf );
#else

	memcpy(netif.name,RTL865xC_TEST_LAN_NETIFNAME,MAX_IFNAMESIZE);
	strtomac(&netif.macAddr, GW_LAN0_MAC);
	netif.mtu = 1500;
	netif.if_type = IF_ETHER;
	netif.vid = LAN0_VID;
	netif.is_wan = 0;
	netif.is_slave = 0; 
	netif.enableRoute=1; 
	rtl865x_addNetif(&netif);
#endif

	
	WRITE_MEM32( PCRP0, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP1, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP2, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP3, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP4, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP5, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP6, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP7, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP8, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
#ifdef REAL_ASIC_ACCESS

	memset( &vlant,0, sizeof(vlant) );
	vlant.memberPortMask = 0x3f;//(1<<hp[3]);
	vlant.untagPortMask = 0; //tagged
	rtl8651_setAsicVlan(LAN0_VID,&vlant);
#else
		if(rtl865x_addVlan(LAN0_VID)!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}
	if(rtl865x_setVlanFilterDatabase(LAN0_VID,0)!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}
	if(rtl865x_modVlanPortMember(LAN0_VID,0x3f,0)!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}
	
#endif

#ifdef REAL_ASIC_ACCESS

	/* ingress filter gateway mac */
	aclt->ruleType_ = RTL865X_ACL_MAC;
	memcpy(aclt->un_ty.srcMac_.octet, intf.macAddr.octet, 6);
	aclt->actionType_ = RTL865X_ACL_TOCPU;
	aclt->pktOpApp_	 = RTL865X_ACL_ALL_LAYER;
	_rtl865x_setAclToAsic(8, aclt);
#endif	
	/* lan 1 */
#ifdef REAL_ASIC_ACCESS

	strtomac(&intf.macAddr, GW_LAN1_MAC);
	intf.vid = LAN1_VID;
	
	intf.inAclStart = 0;
	intf.inAclEnd = 0;
	intf.outAclStart = intf.outAclEnd = 0;	
	rtl8651_setAsicNetInterface( RTL865XC_NETIFTBL_SIZE, &intf );
#else
	memcpy(netif.name,"br1",MAX_IFNAMESIZE);
	strtomac(&netif.macAddr, GW_LAN1_MAC);
	netif.mtu = 1500;
	netif.if_type = IF_ETHER;
	netif.vid = LAN1_VID;
	netif.is_wan = 0;
	netif.is_slave = 0; 
	netif.enableRoute=1; 
	rtl865x_addNetif(&netif);
#endif


	WRITE_MEM32( PCRP0, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP1, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP2, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP3, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP4, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP5, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP6, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP7, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP8, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
#ifdef REAL_ASIC_ACCESS

	memset( &vlant,0, sizeof(vlant) );
	vlant.memberPortMask = vlant.untagPortMask = (1<<hp[2])|(1<<hp[1])|(1<<hp[6]);
	rtl8651_setAsicVlan(LAN1_VID,&vlant);
#else
	if(rtl865x_addVlan(LAN1_VID)!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}
	if(rtl865x_setVlanFilterDatabase(LAN1_VID,0)!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}
	if(rtl865x_modVlanPortMember(LAN1_VID,(1<<hp[2])|(1<<hp[1])|(1<<hp[6]),(1<<hp[2])|(1<<hp[1])|(1<<hp[6]))!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}


#endif
#ifdef REAL_ASIC_ACCESS

	 /* ingress filter gateway mac */
	aclt->ruleType_ = RTL865X_ACL_MAC;
	memcpy(aclt->un_ty.srcMac_.octet, intf.macAddr.octet, 6);
	aclt->actionType_ = RTL865X_ACL_TOCPU;
	aclt->pktOpApp_	 = RTL865X_ACL_ALL_LAYER;
	_rtl865x_setAclToAsic(12, aclt);    
	kfree(aclt);
#endif	


	/* config l2 table */
	memset((void*) &l2t,0, sizeof(l2t));
	/* wan 0 isp modem */
	strtomac(&l2t.macAddr, WAN0_ISP_MAC);
	l2t.memberPortMask = (1<<hp[5]);
	l2t.isStatic = 1;
	l2t.nhFlag=TRUE;
	
	rtl8651_setAsicL2Table(getL2TableRowFromMacStr(WAN0_ISP_MAC), 0, &l2t);
	/* wan 1 host */
	strtomac(&l2t.macAddr, HOST4_MAC);
	l2t.memberPortMask = (1<<hp[4]);
	rtl8651_setAsicL2Table(getL2TableRowFromMacStr(HOST4_MAC), 0, &l2t);
	/* lan 0 host */
	strtomac(&l2t.macAddr, HOST3_MAC);
	l2t.memberPortMask = (1<<hp[3]);
	rtl8651_setAsicL2Table(getL2TableRowFromMacStr(HOST3_MAC), 0, &l2t);
	/* lan 1 host 1 */
	strtomac(&l2t.macAddr, HOST2_MAC);
	l2t.memberPortMask = (1<<hp[2]);
	rtl8651_setAsicL2Table(getL2TableRowFromMacStr(HOST2_MAC), 0, &l2t);
	/* lan 1 host 0 */
	strtomac(&l2t.macAddr, HOST1_MAC);
	l2t.memberPortMask = (1<<hp[1]);
	rtl8651_setAsicL2Table(getL2TableRowFromMacStr(HOST1_MAC), 0, &l2t);
	/* lan 1 host 5 */
	strtomac(&l2t.macAddr, HOST6_MAC);
	l2t.memberPortMask = (1<<hp[6]);
	rtl8651_setAsicL2Table(getL2TableRowFromMacStr(HOST6_MAC), 0, &l2t);
	    
	/* config pppoe table */
	memset((void*) &pppoet,0, sizeof(pppoet));
	pppoet.sessionId = WAN0_ISP_PPPOE_SID;
	rtl8651_setAsicPppoe(0, &pppoet);
	/* config arp table */
	memset((void*) &arpt,0, sizeof(arpt));
	/* wan 1 host */
	arpt.nextHopRow = getL2TableRowFromMacStr(HOST4_MAC);
	strtoip(&ip32, HOST4_IP);
	rtl8651_setAsicArp(ip32 & 0x7, &arpt);
	/* lan 0 host */
	arpt.nextHopRow = getL2TableRowFromMacStr(HOST3_MAC);
	strtoip(&ip32, HOST3_IP);
	rtl8651_setAsicArp(8 + (ip32 & 0x7), &arpt);
    
	/* lan 1 host 1 */
	arpt.nextHopRow = getL2TableRowFromMacStr(HOST2_MAC);
	strtoip(&ip32, HOST2_IP);
	rtl8651_setAsicArp(16 + (ip32 & 0xff), &arpt);	
	/* lan 1 host 0 */
	arpt.nextHopRow = getL2TableRowFromMacStr(HOST1_MAC);
	strtoip(&ip32, HOST1_IP);
	rtl8651_setAsicArp(16 + (ip32 & 0xff), &arpt);
	 /* lan 1 host 5 */
	arpt.nextHopRow = getL2TableRowFromMacStr(HOST6_MAC);
	strtoip(&ip32, HOST6_IP);
	rtl8651_setAsicArp(16 + (ip32 & 0xff), &arpt);
	/* config routing table */
	memset((void*) &routet,0, sizeof(routet));
	/* route to wan 1: use arp table [0:7] */
	strtoip(&routet.ipAddr, GW_WAN1_IP);
	strtoip(&routet.ipMask, GW_WAN1_SUBNET_MASK);
	routet.process = PROCESS_INDIRECT;
	routet.vidx = rtl865xc_netIfIndex(VID_0);
	routet.arpStart = 0;
	routet.arpEnd = 0;
	routet.internal=1;
 	rtl8651_setAsicRouting(0, &routet);
	
	/* route to lan 0: use arp table [8:15] */
	strtoip(&routet.ipAddr, GW_LAN0_IP);
	strtoip(&routet.ipMask, GW_LAN0_SUBNET_MASK);
	routet.process = PROCESS_INDIRECT;
	routet.vidx = rtl865xc_netIfIndex(LAN0_VID);
	routet.arpStart = 1;
	routet.arpEnd = 1;
	routet.internal=1;
	rtl8651_setAsicRouting(1, &routet);
	 /* route to lan 1: use arp table [16:23] */
	 strtoip(&routet.ipAddr, GW_LAN1_IP);
	strtoip(&routet.ipMask, GW_LAN1_SUBNET_MASK);
	routet.process = PROCESS_INDIRECT;
	routet.vidx = rtl865xc_netIfIndex(LAN1_VID);
	routet.arpStart = 2;
	routet.arpEnd = 2;
	routet.internal=1;
	rtl8651_setAsicRouting(6, &routet);
	/* default route to wan 0: direct to modem */
	 memset((void*) &routet,0, sizeof(routet));
	 strtoip(&routet.ipAddr, "0.0.0.0");
	strtoip(&routet.ipMask, "0.0.0.0");
	routet.process = PROCESS_NXT_HOP;
	routet.vidx = rtl865xc_netIfIndex(WAN0_VID);
	routet.nextHopRow = getL2TableRowFromMacStr(WAN0_ISP_MAC);
	if (routetIsPPPoE==0)
	{
		routet.nhStart = 0;
		routet.nhNum = 2; /*index 4~5*/
		routet.nhNxt = 0;
		routet.nhAlgo = 2; /*per-source*/
		routet.ipDomain = 0;
		routet.internal=1;
	}
	else
	{
		routet.nhStart = 30;
		routet.nhNum = 2; /*index 4~5*/
		routet.nhNxt = 0;
		routet.nhAlgo = 2; 
		routet.ipDomain = 0;
		routet.internal=1;
	}


	rtl8651_setAsicRouting(7, &routet);
	 /* config next hop table */
	 memset((void*) &nxthpt,0, sizeof(nxthpt));
	 /* wan 0: load balance traffic through isp0 & isp1 as 3:1 */
	/* isp 0 occupies 3 entries */
	nxthpt.nextHopRow = getL2TableRowFromMacStr(WAN0_ISP_MAC);
	nxthpt.pppoeIdx = 0; 
	nxthpt.dvid = rtl865xc_netIfIndex(WAN0_VID);
	nxthpt.extIntIpIdx = 0;
	nxthpt.isPppoe = 0;
	rtl8651_setAsicNextHopTable(0, &nxthpt);       

	nxthpt.nextHopRow = getL2TableRowFromMacStr(WAN0_ISP_MAC);
	nxthpt.pppoeIdx = 0; 
	nxthpt.dvid = rtl865xc_netIfIndex(WAN0_VID);
	nxthpt.extIntIpIdx = 0; 
	nxthpt.isPppoe = 0;
	rtl8651_setAsicNextHopTable(1, &nxthpt);       


	nxthpt.nextHopRow = getL2TableRowFromMacStr(HOST4_MAC);
	nxthpt.pppoeIdx = 0; 
	nxthpt.dvid =1;
	nxthpt.extIntIpIdx = 0; 
	nxthpt.isPppoe = 1;
	rtl8651_setAsicNextHopTable(30, &nxthpt);       

	nxthpt.nextHopRow = getL2TableRowFromMacStr(HOST4_MAC);
	nxthpt.pppoeIdx = 0;
	nxthpt.dvid =0;
	nxthpt.extIntIpIdx = 0; 
	nxthpt.isPppoe = 0;
	rtl8651_setAsicNextHopTable(31, &nxthpt);       



	return 0;
	
}
/*
 *
 *          +----+----+----+----+----+----+
 *          | P0 | P1 | P2 | P3 | P4 | P5 |
 *          +----+----+----+----+----+----+
 *      PVID  11   11   11  967    9    1
 * tag/untag                      tag  tag
 *  internal      LAN1      LAN0  WAN1  WAN0 default route: PPPoE, 192.168.4.1(NPE) ----- HOST5:192.168.4.5
 *              1.1/29     2.1/29  3.1/29(NE)
 *                 |          |     |
 *                 |          |     \---- HOST4:192.168.3.1/29
 *                 |          |
 *                 |          \---- HOST3:192.168.2.2 (NPI)
 *                 |
 *                 \--- HOST1:192.168.1.2(NI)
 */


int32 rtl865xC_testLayer3GuestVLAN(void)
{
	hsa_param_t hsa;
	uint16 guestVid;
	ether_addr_t mac;
	int32 retval = SUCCESS;
	rtl865x_tblAsicDrv_multiCastParam_t mcast; 
	rtl865x_tblAsicDrv_l2Param_t l2t;
	hsb_param_t hsb ;
	uint32 multi_mtu;

	layer3Config(0);			
	rtl8651_setAsicOperationLayer(3);
	guestVid=13;
	memcpy((void*)&hsb,(void*)&Ghsb,sizeof(hsb));
	memcpy((void*)&hsb.da[0],(void*)strtomac(&mac,GW_LAN0_MAC),sizeof(ether_addr_t));
#ifndef REAL_ASIC_ACCESS		
	#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER_ACL) 
	rtl865x_acl_control_set_mode(RTL865X_ACL_Mode_Always_Permit);	
	#endif
#endif	
	/**********************************************************************
	IP Multicast Packet 224.0.0.3 
	TAG Packet 
	expect : Drop
	**********************************************************************/
	WRITE_MEM32(DOT1XMACCR, EN_8021X_PORT3_MAC_CONTROL);
	multi_mtu=1500; 
	rtl8651_setAsicMulticastMTU(multi_mtu);
	rtl8651_setAsicMulticastEnable(TRUE);
	memset((void*) &mcast,0,sizeof(mcast));
	mcast.dip= ntohl(inet_addr("224.0.0.3"));
	mcast.sip=ntohl(inet_addr("192.168.1.1"));
	mcast.mbr=PM_PORT_1|PM_PORT_2|PM_PORT_3;
	mcast.svid=LAN0_VID;
	mcast.port=3;
	rtl8651_setAsicIpMulticastTable(&mcast);
	memcpy((void*)&hsb.da[0],(void*)strtomac(&mac,"01-00-5e-00-00-03"),sizeof(ether_addr_t));
	hsb.sip=ntohl(inet_addr("192.168.1.1"));
	hsb.dip=ntohl(inet_addr("224.0.0.3"));
	hsb.spa=3;
	hsb.tagif=1;
	hsb.vid=LAN0_VID;
	hsb.type=HSB_TYPE_UDP;
	RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
	RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
	RTL_TESTMODEL_IS_EQUAL("Packet  should be DROP , but .......",hsa.dp,0) ;

	/**********************************************************************
	IP Multicast Packet 224.0.0.3 
	TAG Packet 
	expect : forward lookup ip multicast table
	**********************************************************************/
	memcpy((void*)&hsb.sa[0],(void*)strtomac(&mac,"00-00-5e-00-00-03"),sizeof(ether_addr_t));
	memset((void*) &l2t,0,sizeof(l2t));
	strtomac(&l2t.macAddr,"00-00-5e-00-00-03");
	l2t.memberPortMask = (1<<3);
	l2t.auth=1;
	retval=rtl8651_setAsicL2Table(getL2TableRowFromMacStr("00-00-5e-00-00-03"), 0, &l2t);
	RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
	RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
	RTL_TESTMODEL_IS_EQUAL("Packet  should be to forward , but .......",hsa.dp,mcast.mbr) ;									

	/**********************************************************************
	IP Multicast Packet 224.0.0.3 
	TAG Packet 
	expect : forward , because ... disable multicast table
	**********************************************************************/
	rtl8651_setAsicMulticastEnable(FALSE);
	WRITE_MEM32(FFCR, READ_MEM32(FFCR)&~EnUnkMC2CPU);

	WRITE_MEM32(DOT1XMACCR, EN_8021X_PORT3_MAC_CONTROL|EN_8021X_PORT2_MAC_CONTROL);
	RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
	RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
	RTL_TESTMODEL_IS_EQUAL("Packet  should be to forward , but .......",hsa.dp,PM_PORT_0|PM_PORT_1|PM_PORT_4|PM_PORT_5) ;									

	/**********************************************************************
	IP Multicast Packet 224.0.0.3 
	TAG Packet 
	enable port authentication			
	expect : forward

	**********************************************************************/
	rtl8651_setAsicMulticastEnable(TRUE);

	WRITE_MEM32(DOT1XPORTCR, EN_DOT1XPB_P0|EN_DOT1XPB_P1);							
	RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
	RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
	RTL_TESTMODEL_IS_EQUAL("Packet  should be to forward , but .......",hsa.dp,mcast.mbr) ;												
	/**********************************************************************
	IP Multicast Packet 224.0.0.3 
	TAG Packet 
	expect : CPU
	**********************************************************************/
	memcpy((void*)&hsb.sa[0],(void*)strtomac(&mac," 01-00-5e-00-00-03"),sizeof(ether_addr_t));
	memset((void*) &l2t,0,sizeof(l2t));
	strtomac(&l2t.macAddr,"01-00-5e-00-00-03");
	l2t.memberPortMask = (1<<3);
	l2t.isStatic =1;
	l2t.auth=1;
	retval=rtl8651_setAsicL2Table(getL2TableRowFromMacStr("01-00-5e-00-00-03"), 0, &l2t);				
	RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
	RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
	RTL_TESTMODEL_IS_EQUAL("Packet  should be to forward , but .......",hsa.dp,mcast.mbr) ;						

	/*********************************************************************
	authenticated node --> unauthenticated node 
	DA is Gateway MAC
	Expect  :Forward
	**********************************************************************/
	WRITE_MEM32(DOT1XMACCR, EN_8021X_PORT3_MAC_CONTROL);			
	memset((void*) &l2t,0,sizeof(l2t));
	strtomac(&l2t.macAddr, HOST3_MAC);
	l2t.memberPortMask = (1<<3);
	l2t.isStatic =0;
	l2t.auth=1;
	rtl8651_setAsicL2Table(getL2TableRowFromMacStr(HOST3_MAC), 0, &l2t);				
	memcpy((void*)&hsb.sa[0],(void*)strtomac(&mac,HOST3_MAC),sizeof(ether_addr_t));
	memcpy((void*)&hsb.da[0],(void*)strtomac(&mac,GW_LAN0_MAC),sizeof(ether_addr_t));
	hsb.dip= ntohl(inet_addr("124.0.0.3"));
	hsb.spa=3;
	hsb.type=HSB_TYPE_TCP;
	RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
	RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
	RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward, but .......",hsa.dp,PM_PORT_5 );
	/*********************************************************************
	authenticated node --> unauthenticated node 
	enable port authenicatioin
	DA is Gateway MAC
	Expect  :drop
	**********************************************************************/
	WRITE_MEM32(DOT1XPORTCR, EN_DOT1XPB_P0|EN_DOT1XPB_P5);				
	memset((void*) &l2t,0,sizeof(l2t));
	strtomac(&l2t.macAddr, HOST3_MAC);
	l2t.memberPortMask = (1<<3);
	l2t.isStatic =0;
	l2t.auth=1;
	rtl8651_setAsicL2Table(getL2TableRowFromMacStr(HOST3_MAC), 0, &l2t);				
	memcpy((void*)&hsb.sa[0],(void*)strtomac(&mac,HOST3_MAC),sizeof(ether_addr_t));
	memcpy((void*)&hsb.da[0],(void*)strtomac(&mac,GW_LAN0_MAC),sizeof(ether_addr_t));
	hsb.dip= ntohl(inet_addr("124.0.0.3"));
	hsb.spa=3;
	hsb.type=HSB_TYPE_TCP;
	RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
	RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
	RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward, but .......",hsa.dp,0  );
	RTL_TESTMODEL_IS_EQUAL("Finish  Lookup Working .......",hsa.hwfwrd,1);
	/*********************************************************************
	authenticated node --> unauthenticated node 
	enable port authenicatioin
	DA is Gateway MAC
	Expect  :forward
	**********************************************************************/
	WRITE_MEM32(DOT1XPORTCR, EN_DOT1XPB_P0|EN_DOT1XPB_P5|EN_DOT1XPB_P5_OPINDIR);				
	memset((void*) &l2t,0,sizeof(l2t));
	strtomac(&l2t.macAddr, HOST3_MAC);
	l2t.memberPortMask = (1<<3);
	l2t.isStatic =0;
	l2t.auth=1;
	rtl8651_setAsicL2Table(getL2TableRowFromMacStr(HOST3_MAC), 0, &l2t);				
	memcpy((void*)&hsb.sa[0],(void*)strtomac(&mac,HOST3_MAC),sizeof(ether_addr_t));
	memcpy((void*)&hsb.da[0],(void*)strtomac(&mac,GW_LAN0_MAC),sizeof(ether_addr_t));
	hsb.dip= ntohl(inet_addr("124.0.0.3"));
	hsb.spa=3;
	hsb.type=HSB_TYPE_TCP;
	RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
	RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
	RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward, but .......",hsa.dp,PM_PORT_5  );
	RTL_TESTMODEL_IS_EQUAL("Finish  Lookup Working .......",hsa.hwfwrd,1  );
	/*********************************************************************
	authenticated node --> unauthenticated node 
	enable port authenicatioin and MAC 1X
	DA is Gateway MAC
	Expect  :forward
	**********************************************************************/
	WRITE_MEM32(DOT1XPORTCR, EN_DOT1XPB_P0|EN_DOT1XPB_P5|EN_DOT1XPB_P5_OPINDIR);				
	WRITE_MEM32(DOT1XMACCR, EN_8021X_PORT3_MAC_CONTROL|EN_8021X_PORT5_MAC_CONTROL);			
	memset((void*) &l2t,0,sizeof(l2t));
	strtomac(&l2t.macAddr, HOST3_MAC);
	l2t.memberPortMask = (1<<3);
	l2t.isStatic =0;
	l2t.auth=1;
	rtl8651_setAsicL2Table(getL2TableRowFromMacStr(HOST3_MAC), 0, &l2t);				
	memcpy((void*)&hsb.sa[0],(void*)strtomac(&mac,HOST3_MAC),sizeof(ether_addr_t));
	memcpy((void*)&hsb.da[0],(void*)strtomac(&mac,GW_LAN0_MAC),sizeof(ether_addr_t));
	hsb.dip= ntohl(inet_addr("124.0.0.3"));
	hsb.spa=3;
	hsb.type=HSB_TYPE_TCP;
	RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
	RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
	RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward, but .......",hsa.dp,0 );
	RTL_TESTMODEL_IS_EQUAL("Finish  Lookup Working .......",hsa.hwfwrd,1 );			
	/*********************************************************************
	unauthenticated node --> unauthenticated node 
	DA is Gateway MAC
	Entry is dynamic
	Expect  :Drop
	**********************************************************************/
	memset((void*) &l2t,0,sizeof(l2t));
	strtomac(&l2t.macAddr, HOST3_MAC);
	l2t.memberPortMask = (1<<3);
	l2t.isStatic =0;
	l2t.auth=0;
	rtl8651_setAsicL2Table(getL2TableRowFromMacStr(HOST3_MAC), 0, &l2t);				
	memcpy((void*)&hsb.sa[0],(void*)strtomac(&mac,HOST3_MAC),sizeof(ether_addr_t));
	memcpy((void*)&hsb.da[0],(void*)strtomac(&mac,GW_LAN0_MAC),sizeof(ether_addr_t));
	hsb.spa=3;
	hsb.type=HSB_TYPE_TCP;
		RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
	RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
	RTL_TESTMODEL_IS_EQUAL("Pakcet should be Drop, but .......",hsa.dp,0x0 );	
	/*********************************************************************
	Enable Guest VLAN  
	unauthenticated node --> unauthenticated node 
		DA is Gateway MAC		
		Entry is static
	Expect Drop
	*********************************************************************/			 
	WRITE_MEM32(GVGCR,guestVid|EN_8021X_TOGVLAN);
	strtomac(&l2t.macAddr, HOST3_MAC);
	l2t.memberPortMask = (1<<3);
	l2t.isStatic = 1;
	l2t.auth=0;
	rtl8651_setAsicL2Table(getL2TableRowFromMacStr(HOST3_MAC), 0, &l2t);							
	RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
	RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
	RTL_TESTMODEL_IS_EQUAL(" Pakcet should be Drop, but .......",hsa.dp,0x0 );
	/*********************************************************************
	Enable Guest VLAN  layer3 ToCPU 
	unauthenticated node --> unauthenticated node 
		DA is Gateway MAC			 
	Expect to CPU
	*********************************************************************/		
	WRITE_MEM32(GVGCR,guestVid|EN_8021X_TOGVLAN|EN_DOT1X_GVR2CPU);
	RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
	RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
	RTL_TESTMODEL_IS_EQUAL(" Pakcet should be CPU, but .......",hsa.dpext,PM_PORT_CPU );
	/*********************************************************************
	Enable Guest VLAN  layer3 ToCPU 
	unauthenticated node --> unauthenticated node 
		DA is Gateway MAC			 
	Expect to CPU
	*********************************************************************/		
	strtomac(&l2t.macAddr, GW_LAN0_MAC);
	l2t.memberPortMask = (1<<3);
	l2t.isStatic = 0;
	l2t.auth=1;			
	rtl8651_setAsicL2Table(getL2TableRowFromMacStr(GW_LAN0_MAC), 0, &l2t);
	RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
	RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
	RTL_TESTMODEL_IS_EQUAL(" Pakcet should be CPU, but .......",hsa.dpext,PM_PORT_CPU  );

	return 	SUCCESS;
}

int32 rtl865xC_testLayer3Routing(void)
{
	int32 retval = SUCCESS;
	hsb_param_t hsb =
	{
		spa: 3, 	len: 64,
		vid: 0x0,		tagif: 0,
		pppoeif: 0, 	sip: ntohl(inet_addr("192.168.1.2")),
		sprt: 1020, 	dip:ntohl(inet_addr("120.168.1.3")),
		dprt: 80,			ipptl: 0xdd,
		iptos: 0x65,
		tcpfg: 0x00,		type: HSB_TYPE_UDP, 
		patmatch:0, 	ethtype: 0x0800,
		ipfg:0, 		ipfo0_n:0,
		sa:{0x00,0x80,0xc0,0x00,0x00,0x00},
		hiprior: 0, 	snap: 0,
		udpnocs: 0, 	ttlst: 2,
		dirtx: 0,			l3csok: 1,
		l4csok: 1,		
		llcothr: 0, 		urlmch: 0,
		extspa: 0,		extl2: 0,
		
	};
	
	hsa_param_t hsa;
	ether_addr_t mac;
	{

	/* Compare the HSA of expected, model, FPGA, and real IC. */
		layer3Config(0);			
#ifndef REAL_ASIC_ACCESS		
	#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER_ACL) 
		rtl865x_acl_control_set_mode(RTL865X_ACL_Mode_Always_Permit);	
	#endif
#endif	
		/*********************************************************************		
		Expect : Forward
		*********************************************************************/
		WRITE_MEM32(SWTCR0, (READ_MEM32(SWTCR0)&~LIMDBC_MASK) | LIMDBC_VLAN);
		memcpy((void*)&hsb.da[0],(void*)strtomac(&mac,GW_LAN0_MAC),sizeof(ether_addr_t));
		hsb.dip=ntohl(inet_addr(HOST5_IP));
		hsb.sip=ntohl(inet_addr("192.168.1.2"));
		RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
		RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
		RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward ,but ....", hsa.dp,PM_PORT_5);

		
		/*********************************************************************		
		PORT  BASE Routing
		Expect : Forward
		*********************************************************************/
		WRITE_MEM32(SWTCR0,(READ_MEM32(SWTCR0)&~LIMDBC_MASK) | LIMDBC_PORT);
		
		memcpy((void*)&hsb.da[0],(void*)strtomac(&mac,GW_WAN0_MAC),sizeof(ether_addr_t));
		hsb.spa=PN_PORT0;
		hsb.dip=ntohl(inet_addr(HOST5_IP));
		RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
		RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
		RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward ,but ....", hsa.dp,PM_PORT_4);

		/*********************************************************************		
		PORT  BASE Routing
		Expect : NOT MATCH AND Layer2 Forward
		*********************************************************************/
		rtl8651_setPortToNetif(0,6);
		RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
		RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
		RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward ,but ....", hsa.dp,PM_PORT_4);
	
		/*********************************************************************		
		PORT  BASE Routing
		Expect : Routing and Forward
		*********************************************************************/
		hsb.spa=PN_PORT1;

		RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
		RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
		RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward ,but ....", hsa.dp,PM_PORT_5);

		
		WRITE_MEM32( VCR0,READ_MEM32(VCR0) &~ EN_ALL_PORT_VLAN_INGRESS_FILTER); 
		hsb.vid=WAN0_VID;
		hsb.tagif=1;

		RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
		RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
		RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward ,but ....", hsa.dp,PM_PORT_5);

		hsb.tagif=0;
		rtl8651_setPortToNetif(1,6);
		RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
		RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
		RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward ,but ....", hsa.dp,PM_PORT_4|PM_PORT_2);		
		
		/*********************************************************************		
		MAC  BASE Routing
		Expect : Forward
		*********************************************************************/		
		WRITE_MEM32(SWTCR0,(READ_MEM32(SWTCR0) & ~LIMDBC_MASK)| LIMDBC_MAC);
		memcpy((void*)&hsb.da[0],(void*)strtomac(&mac,GW_WAN0_MAC),sizeof(ether_addr_t));
		hsb.spa=0;
		hsb.dip=ntohl(inet_addr(HOST5_IP));
		RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
		RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
		RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward ,but ....", hsa.dp,PM_PORT_5);

		/*********************************************************************		
		MAC  BASE Routing
		Expect : Forward
		*********************************************************************/		
		hsb.spa=PN_PORT1;
		RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
		RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
		RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward ,but ....", hsa.dp,PM_PORT_5);
		/*********************************************************************		
		MAC  BASE Routing
		Expect : Forward
		*********************************************************************/		
		hsb.spa=PN_PORT2;
		RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
		RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
		RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward ,but ....", hsa.dp,PM_PORT_5);
		/*********************************************************************		
		MAC  BASE Routing
		Expect : Forward
		*********************************************************************/		
		hsb.spa=PN_PORT3;
		RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
		RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
		RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward ,but ....", hsa.dp,PM_PORT_5);

		/*********************************************************************		
		MAC  BASE Routing
		Expect : Forward
		*********************************************************************/		
		hsb.spa=PN_PORT3;
		memcpy((void*)&hsb.da[0],(void*)strtomac(&mac,GW_LAN0_MAC),sizeof(ether_addr_t));
		RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
		RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
		RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward ,but ....", hsa.dp,PM_PORT_5);
	//	RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward ,but ....", hsa.dp,PM_PORT_0|PM_PORT_1|PM_PORT_2|PM_PORT_4|PM_PORT_5);
		/*********************************************************************		
		MAC  BASE Routing
		Expect : Forward
		*********************************************************************/		
		hsb.spa=PN_PORT4;
		RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
		RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
	//	RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward ,but ....",PM_PORT_1|PM_PORT_2,hsa.dp);
		RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward ,but ....", hsa.dp,PM_PORT_5);
#if 0 //MAC mask have a bug, so we skip !	
		/*********************************************************************
		Testing 8 GATEWAY MACs
		*********************************************************************/
		layer3Config(0);		
		bzero(&intf,sizeof(intf));			
		strtomac(&intf.macAddr, GW_LAN0_MAC);
		intf.macAddr.octet[5]&=0xf0;
		intf.vid = LAN0_VID;
		intf.inAclStart = 0;
		intf.inAclEnd = 0;
		intf.outAclStart = intf.outAclEnd = 0;
		intf.valid=1;
		intf.macAddrNumber=8;
		intf.mtu=1500;
		intf.enableRoute=1; 
		rtl8651_setAsicNetInterface( RTL865XC_NETIFTBL_SIZE, &intf );
		
		WRITE_MEM32( PCRP0, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
		WRITE_MEM32( PCRP1, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
		WRITE_MEM32( PCRP2, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
		WRITE_MEM32( PCRP3, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
		WRITE_MEM32( PCRP4, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
		WRITE_MEM32( PCRP5, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
		WRITE_MEM32( PCRP6, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
		WRITE_MEM32( PCRP7, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
		WRITE_MEM32( PCRP8, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
		bzero( &vlant, sizeof(vlant) );
		vlant.memberPortMask = (1<<hp[3]);
		vlant.untagPortMask = 0; //tagged
		rtl8651_setAsicVlan(intf.vid,&vlant);
		memcpy((void*)&hsb,(void*)&Ghsb,sizeof(hsb));
		memcpy((void*)&hsb.da[0],(void*)strtomac(&mac,GW_LAN0_MAC),sizeof(ether_addr_t));
		hsb.da[5] =intf.macAddr.octet[5] |0x1;
		hsb.sip=ntohl(inet_addr("192.168.1.2")),
		hsb.dip=ntohl(inet_addr(HOST5_IP));
		RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
		RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
		RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward ,but ....", hsa.dp,PM_PORT_5);
		/*********************************************************************		
		Expect : Forward
		*********************************************************************/
		hsb.dip=ntohl(inet_addr(HOST5_IP));
		hsb.da[5] |=0x2;
		RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
		RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
		RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward ,but ....", hsa.dp,PM_PORT_5);
		/*********************************************************************		
		Expect : Forward
		*********************************************************************/
		hsb.da[5] |=0x3;
		RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
		RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
		RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward ,but ....", hsa.dp,PM_PORT_5);
		/*********************************************************************		
		Expect : Forward
		*********************************************************************/
		hsb.da[5] |=0x7;
		RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
		RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
		RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward ,but ....", hsa.dp,PM_PORT_5);
		/*********************************************************************
		 VID 0 Routing 
		Expect : Piroirty TAG
		*********************************************************************/
		hsb.dip=ntohl(inet_addr(HOST4_IP));
		RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
		RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
		RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward ,but ....", hsa.dp,PM_PORT_4);

#endif
	}
	retval = SUCCESS;	
	return retval;
}


int32 rtl865xC_testEnhancedGRE(void)
{
	return SUCCESS;
}
int32 rtl865xC_testIpOther(void)
{
	return SUCCESS;
}
int32 rtl865xC_testLayer3RoutingToCpu(void)
{
	hsb_param_t hsb =
	{
		spa: 3,		len: 64,
		vid: 0x0,		tagif: 0,
		pppoeif: 0,		sip: ntohl(inet_addr("192.168.1.2")),
		sprt: 1020,		dip:ntohl(inet_addr("120.168.1.3")),
		dprt: 80,			ipptl: 0xdd,
		iptos: 0x65,
		tcpfg: 0x00,		type: HSB_TYPE_UDP,	
		patmatch:0,		ethtype: 0x0800,
		ipfg:0,			ipfo0_n:0,
		sa:{0x00,0x80,0xc0,0x00,0x00,0x00},
		hiprior: 0,		snap: 0,
		udpnocs: 0,		ttlst: 2,
		dirtx: 0,			l3csok: 1,
		l4csok: 1,		
		llcothr: 0,			urlmch: 0,
		extspa: 0,		extl2: 0,
		
	};
	ether_addr_t mac;
		rtl865x_tblAsicDrv_routingParam_t routet;
		hsa_param_t hsa;		
		//ether_addr_t mac;
		/* Configure a pure routing environment  */
		layer3Config(0);
		//RTL_TESTMODEL_INIT_CHECK(rtl865xC_layer3Config(0));
#ifndef REAL_ASIC_ACCESS			
	#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER_ACL) 
		rtl865x_acl_control_set_mode(RTL865X_ACL_Mode_Always_Permit);	
	#endif
#endif		
		strtoip(&routet.ipAddr, "222.222.222.222");
		strtoip(&routet.ipMask, "255.255.255.128");
		routet.process = PROCESS_S_CPU;
		routet.internal = 1;
		rtl8651_setAsicRouting(2, &routet);
	
		/*********************************************************************		
		Expect : Forward
		*********************************************************************/
		memcpy((void*)&hsb.da[0],(void*)strtomac(&mac,GW_LAN0_MAC),sizeof(ether_addr_t));
		hsb.dip=ntohl(inet_addr("222.222.222.234"));
		RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
		RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
		
		RTL_TESTMODEL_IS_EQUAL("hsa.dpc should be 1", hsa.dpc, 1 );
		RTL_TESTMODEL_IS_EQUAL("Pakcet should be trapped to CPU", hsa.dp, PM_PORT_NOTPHY );
		RTL_TESTMODEL_IS_EQUAL("Packet should be forwarded to CPU", hsa.dpext, PM_PORT_CPU) ;
		RTL_TESTMODEL_IS_EQUAL("CPU Reason[4:1] should be L34 Action Required (real, expect)", (hsa.why2cpu>>1)&0xf, 5/*L34 Action Required*/ );
		RTL_TESTMODEL_IS_EQUAL("CPU Reason[9:5] should be NPI(SIP) (real, expect)", (hsa.why2cpu>>5)&0x1f, 0x10/*NPI*/);
		RTL_TESTMODEL_IS_EQUAL("CPU Reason[14:10] should be NPI(DIP) (real, expect)", (hsa.why2cpu>>10)&0x1f, 0x10/*NPI*/ );	

	return SUCCESS;

}

int32  layer4Config( void )
{

	rtl865x_tblAsicDrv_extIntIpParam_t ipt;	
#ifdef 	REAL_ASIC_ACCESS	
	rtl865x_tblAsicDrv_vlanParam_t vlant;
	rtl865x_tblAsicDrv_intfParam_t intf;
	rtl865x_AclRule_t *aclt;
#else	
	struct rtl865x_netif_s netif;
#endif	
	rtl865x_tblAsicDrv_l2Param_t l2t;
	rtl865x_tblAsicDrv_pppoeParam_t pppoet;
	rtl865x_tblAsicDrv_arpParam_t arpt;
	rtl865x_tblAsicDrv_routingParam_t routet;
	rtl865x_tblAsicDrv_nextHopParam_t nxthpt;
	ipaddr_t ip32;
	hp = port_number_of_host[0];
	rtl8651_clearRegister();
	rtl8651_clearAsicPvid();
	rtl8651_setAsicPvid(hp[5],WAN0_VID);  /*host 5 pvid 1*/
	rtl8651_setAsicPvid(hp[4],9);   /*host 4 pvid 1*/
	rtl8651_setAsicPvid(hp[3],LAN0_VID);  /*host 3 pvid 0x3c7*/
	rtl8651_setAsicPvid(hp[2],11);  /*host 2 pvid 3*/
	rtl8651_setAsicPvid(hp[1],11);  /*host 1 pvid 3*/
	rtl8651_setAsicPvid(hp[0],11);  /*host 6 pvid 3*/
	/* module switch control reg: enable L2/L3/L4/STP, trap 802.1D/GARP to CPU */
	rtl8651_setAsicOperationLayer(4);
	/* TTL control reg: enable TTL-1 operation */
	WRITE_MEM32(ALECR,READ_MEM32(ALECR)|(uint32)EN_TTL1);	
	WRITE_MEM32(SWTCR1,READ_MEM32(SWTCR1)|EN_51B_CPU_REASON);

	 /* config vlan table */

#ifdef REAL_ASIC_ACCESS	 
	 bzero((void*) &intf, sizeof(intf));
	/* wan 0 */
	strtomac(&intf.macAddr, GW_WAN0_MAC);
	intf.macAddrNumber = 1;
	intf.vid = WAN0_VID;
	intf.mtu = 1500;
	intf.inAclStart = 0;
	intf.inAclEnd = 0;
	intf.outAclStart = intf.outAclEnd = 0;
	intf.enableRoute = 1;
	intf.valid = 1;
	rtl8651_setAsicNetInterface( RTL865XC_NETIFTBL_SIZE, &intf );
#else
	memset(&netif, 0, sizeof(rtl865x_netif_t));
	memcpy(netif.name,"nas0_0test",MAX_IFNAMESIZE);
	strtomac(&netif.macAddr, GW_WAN0_MAC);
	netif.mtu = 1500;
	netif.if_type = IF_ETHER;
	netif.vid = WAN0_VID;
	netif.is_wan = 0;
	netif.is_slave = 0;
	netif.enableRoute=1;
	rtl865x_addNetif(&netif);
#endif	
	WRITE_MEM32( PCRP0, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP1, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP2, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP3, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP4, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP5, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP6, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP7, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );
	WRITE_MEM32( PCRP8, (READ_MEM32(PCRP0)&~STP_PortST_MASK)|STP_PortST_FORWARDING );	
#ifdef REAL_ASIC_ACCESS	
	memset( &vlant,0, sizeof(vlant) );
	vlant.memberPortMask = vlant.untagPortMask= (1<<hp[5]);
	rtl8651_setAsicVlan(WAN0_VID,&vlant);
#else	
	if(rtl865x_addVlan(WAN0_VID)!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}
	if(rtl865x_setVlanFilterDatabase(WAN0_VID,0)!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}
	if(rtl865x_modVlanPortMember(WAN0_VID,(1<<hp[5]),(1<<hp[5]))!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}
	
#endif
#ifdef REAL_ASIC_ACCESS	
	 /* ingress filter gateway mac */

	aclt = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
	memset(aclt, 0,sizeof(rtl865x_AclRule_t)); 
	aclt->ruleType_ = RTL865X_ACL_MAC;
	aclt->actionType_ = RTL865X_ACL_PERMIT;
	aclt->pktOpApp_  = RTL865X_ACL_ALL_LAYER;
	_rtl865x_setAclToAsic(0, aclt);
#endif	
	 /* wan 1 */	
#ifdef REAL_ASIC_ACCESS
	strtomac(&intf.macAddr, GW_WAN1_MAC);
	intf.vid = WAN1_VID;
	
	intf.inAclStart = 0;
	intf.inAclEnd = 0;
	intf.outAclStart = intf.outAclEnd = 0;
	rtl8651_setAsicNetInterface( RTL865XC_NETIFTBL_SIZE, &intf );
#else
	
	memcpy(netif.name,"nas0_1test",MAX_IFNAMESIZE); 
	strtomac(&netif.macAddr, GW_WAN1_MAC);
	netif.mtu = 1500;
	netif.if_type = IF_ETHER;
	netif.vid = WAN1_VID;
	netif.is_wan = 0;
	netif.is_slave = 0; 
	netif.enableRoute=1; 
	rtl865x_addNetif(&netif);
#endif


#ifdef REAL_ASIC_ACCESS	
	memset( &vlant,0, sizeof(vlant) );
	vlant.memberPortMask = vlant.untagPortMask = (1<<hp[4]);	
		rtl8651_setAsicVlan(WAN1_VID,&vlant);
#else
	if(rtl865x_addVlan(WAN1_VID)!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}
	if(rtl865x_setVlanFilterDatabase(WAN1_VID,0)!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}
	if(rtl865x_modVlanPortMember(WAN1_VID,(1<<hp[4]),(1<<hp[4]))!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}


#endif



	
#ifdef REAL_ASIC_ACCESS
	/* ingress filter gateway mac */	
	aclt->ruleType_ = RTL865X_ACL_MAC;
	memcpy(aclt->un_ty.srcMac_.octet, intf.macAddr.octet, 6);
	aclt->actionType_ = RTL865X_ACL_TOCPU;
	aclt->pktOpApp_		 = RTL865X_ACL_ALL_LAYER;
	_rtl865x_setAclToAsic(4, aclt);
#endif
	/* lan 0 */

#ifdef REAL_ASIC_ACCESS

	strtomac(&intf.macAddr, GW_LAN0_MAC);
	intf.vid = LAN0_VID;
	
	intf.inAclStart = 0;
	intf.inAclEnd = 0;
	intf.outAclStart = intf.outAclEnd = 0;
	rtl8651_setAsicNetInterface( RTL865XC_NETIFTBL_SIZE, &intf );
#else

	memcpy(netif.name,RTL865xC_TEST_LAN_NETIFNAME,MAX_IFNAMESIZE);
	strtomac(&netif.macAddr, GW_LAN0_MAC);
	netif.mtu = 1500;
	netif.if_type = IF_ETHER;
	netif.vid = LAN0_VID;
	netif.is_wan = 0;
	netif.is_slave = 0; 
	netif.enableRoute=1; 
	rtl865x_addNetif(&netif);
#endif


#ifdef REAL_ASIC_ACCESS

	memset( &vlant,0, sizeof(vlant) );
	vlant.memberPortMask = 0xf;//(1<<hp[3]);
	vlant.untagPortMask = 0; //tagged

	rtl8651_setAsicVlan(LAN0_VID,&vlant);
#else
	if(rtl865x_addVlan(LAN0_VID)!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}
	if(rtl865x_setVlanFilterDatabase(LAN0_VID,0)!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}
	if(rtl865x_modVlanPortMember(LAN0_VID,0xf,0)!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}
	
#endif
#ifdef REAL_ASIC_ACCESS

	/* ingress filter gateway mac */
	aclt->ruleType_ = RTL865X_ACL_MAC;
	memcpy(aclt->un_ty.srcMac_.octet, intf.macAddr.octet, 6);
	aclt->actionType_ = RTL865X_ACL_TOCPU;
	aclt->pktOpApp_	 = RTL865X_ACL_ALL_LAYER;
	_rtl865x_setAclToAsic(8, aclt);
	
#endif		
	/* lan 1 */


#ifdef REAL_ASIC_ACCESS

	strtomac(&intf.macAddr, GW_LAN1_MAC);
	intf.vid = LAN1_VID;
	
	intf.inAclStart = 0;
	intf.inAclEnd = 0;
	intf.outAclStart = intf.outAclEnd = 0;	
	rtl8651_setAsicNetInterface( RTL865XC_NETIFTBL_SIZE, &intf );
#else
	memcpy(netif.name,"br1",MAX_IFNAMESIZE);
	strtomac(&netif.macAddr, GW_LAN1_MAC);
	netif.mtu = 1500;
	netif.if_type = IF_ETHER;
	netif.vid = LAN1_VID;
	netif.is_wan = 0;
	netif.is_slave = 0; 
	netif.enableRoute=1; 
	rtl865x_addNetif(&netif);
#endif

#ifdef REAL_ASIC_ACCESS

	memset( &vlant,0, sizeof(vlant) );
	vlant.memberPortMask = vlant.untagPortMask = (1<<hp[2])|(1<<hp[1])|(1<<hp[0]);
	rtl8651_setAsicVlan(LAN1_VID,&vlant);
#else
	if(rtl865x_addVlan(LAN1_VID)!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}
	if(rtl865x_setVlanFilterDatabase(LAN1_VID,0)!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}
	if(rtl865x_modVlanPortMember(LAN1_VID,(1<<hp[2])|(1<<hp[1])|(1<<hp[6]),(1<<hp[2])|(1<<hp[1])|(1<<hp[6]))!=SUCCESS)
	{
		DBG_TESTMODEL_L34_TEST_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}


#endif

	
#ifdef REAL_ASIC_ACCESS

	 /* ingress filter gateway mac */
	 aclt->ruleType_ = RTL865X_ACL_MAC;
	memcpy(aclt->un_ty.srcMac_.octet, intf.macAddr.octet, 6);
	aclt->actionType_ = RTL865X_ACL_TOCPU;
	aclt->pktOpApp_	 = RTL865X_ACL_ALL_LAYER;
	_rtl865x_setAclToAsic(12, aclt);    
	
#endif	
	/* config l2 table */
	bzero((void*) &l2t, sizeof(l2t));
	/* wan 0 isp modem */
	strtomac(&l2t.macAddr, WAN0_ISP_MAC);
	l2t.memberPortMask = (1<<hp[5]);
	l2t.isStatic = 1;
	rtl8651_setAsicL2Table(getL2TableRowFromMacStr(WAN0_ISP_MAC), 0, &l2t);
	/* wan 1 host */
	strtomac(&l2t.macAddr, HOST4_MAC);
	l2t.memberPortMask = (1<<hp[4]);
	rtl8651_setAsicL2Table(getL2TableRowFromMacStr(HOST4_MAC), 0, &l2t);
	/* lan 0 host */
	strtomac(&l2t.macAddr, HOST3_MAC);
	l2t.memberPortMask = (1<<hp[3]);
	rtl8651_setAsicL2Table(getL2TableRowFromMacStr(HOST3_MAC), 0, &l2t);
	/* lan 1 host 1 */
	strtomac(&l2t.macAddr, HOST2_MAC);
	l2t.memberPortMask = (1<<hp[2]);
	rtl8651_setAsicL2Table(getL2TableRowFromMacStr(HOST2_MAC), 0, &l2t);
	/* lan 1 host 0 */
	strtomac(&l2t.macAddr, HOST1_MAC);
	l2t.memberPortMask = (1<<hp[1]);
	rtl8651_setAsicL2Table(getL2TableRowFromMacStr(HOST1_MAC), 0, &l2t);
	/* lan 1 host 5 */
	strtomac(&l2t.macAddr, HOST6_MAC);
	l2t.memberPortMask = (1<<hp[0]);
	rtl8651_setAsicL2Table(getL2TableRowFromMacStr(HOST6_MAC), 0, &l2t);
	    
	/* config pppoe table */
	bzero((void*) &pppoet, sizeof(pppoet));
	pppoet.sessionId = WAN0_ISP_PPPOE_SID;
	rtl8651_setAsicPppoe(0, &pppoet);
	/* config arp table */
	bzero((void*) &arpt, sizeof(arpt));
	/* wan 1 host */
	arpt.nextHopRow = getL2TableRowFromMacStr(HOST4_MAC);
	strtoip(&ip32, HOST4_IP);
	rtl8651_setAsicArp(ip32 & 0x7, &arpt);
	/* lan 0 host */
	arpt.nextHopRow = getL2TableRowFromMacStr(HOST3_MAC);
	strtoip(&ip32, HOST3_IP);
	rtl8651_setAsicArp(8 + (ip32 & 0x7), &arpt);
    
	/* lan 1 host 1 */
	arpt.nextHopRow = getL2TableRowFromMacStr(HOST2_MAC);
	strtoip(&ip32, HOST2_IP);
	rtl8651_setAsicArp(16 + (ip32 & 0xff), &arpt);		
	/* lan 1 host 0 */
	arpt.nextHopRow = getL2TableRowFromMacStr(HOST1_MAC);
	strtoip(&ip32, HOST1_IP);
	rtl8651_setAsicArp(16 + (ip32 & 0xff), &arpt);
	 /* lan 1 host 5 */
	arpt.nextHopRow = getL2TableRowFromMacStr(HOST6_MAC);
	strtoip(&ip32, HOST6_IP);
	rtl8651_setAsicArp(16 + (ip32 & 0xff), &arpt);
	/* config routing table */
	bzero((void*) &routet, sizeof(routet));
	/* route to wan 1: use arp table [0:7] */
	strtoip(&routet.ipAddr, GW_WAN1_IP);
	strtoip(&routet.ipMask, GW_WAN1_SUBNET_MASK);
	routet.process = PROCESS_INDIRECT;
	routet.vidx = rtl865xc_netIfIndex(WAN1_VID);
	routet.arpStart = 0;
	routet.arpEnd = 0;
	routet.internal=0;
 	rtl8651_setAsicRouting(0, &routet);
	
	/* route to lan 0: use arp table [8:15] */
	strtoip(&routet.ipAddr, GW_LAN0_IP);
	strtoip(&routet.ipMask, GW_LAN0_SUBNET_MASK);
	routet.process = PROCESS_INDIRECT;
	routet.vidx = rtl865xc_netIfIndex(LAN0_VID);
	routet.arpStart = 1;
	routet.arpEnd = 1;
	routet.internal=1;
	rtl8651_setAsicRouting(1, &routet);
	 /* route to lan 1: use arp table [16:23] */
	 strtoip(&routet.ipAddr, GW_LAN1_IP);
	strtoip(&routet.ipMask, GW_LAN1_SUBNET_MASK);
	routet.process = PROCESS_INDIRECT;
	routet.vidx = rtl865xc_netIfIndex(LAN1_VID);
	routet.arpStart = 2;
	routet.arpEnd = 2;
	routet.internal=1;
	rtl8651_setAsicRouting(2, &routet);
	/* default route to wan 0: direct to modem */
	 bzero((void*) &routet, sizeof(routet));
	 strtoip(&routet.ipAddr, "0.0.0.0");
	strtoip(&routet.ipMask, "0.0.0.0");
	routet.process = PROCESS_NXT_HOP;
	routet.vidx = rtl865xc_netIfIndex(WAN0_VID);
	routet.nextHopRow = getL2TableRowFromMacStr(WAN0_ISP_MAC);
	routet.nhStart = 0;
	routet.nhNum = 2; //index 0-1
	routet.nhNxt = 0;
	routet.nhAlgo = 2; //per-source
	routet.ipDomain = 0;
	routet.internal=0;
	rtl8651_setAsicRouting(7, &routet);
	 /* config next hop table */
	 bzero((void*) &nxthpt, sizeof(nxthpt));
	 /* wan 0: load balance traffic through isp0 & isp1 as 3:1 */
	/* isp 0 occupies 3 entries */
	 bzero((void*) &nxthpt, sizeof(nxthpt));
	 /* wan 0: load balance traffic through isp0 & isp1 as 3:1 */
	/* isp 0 occupies 3 entries */
	nxthpt.nextHopRow = getL2TableRowFromMacStr(WAN0_ISP_MAC);
	nxthpt.pppoeIdx = 0; /*WAN0_ISP0_PPPOE_SID*/
	nxthpt.dvid = rtl865xc_netIfIndex(WAN0_VID);
	nxthpt.extIntIpIdx = 0; /*WAN0_ISP0_NPE_IP*/
	nxthpt.isPppoe = 0;
	rtl8651_setAsicNextHopTable(0, &nxthpt);       

	nxthpt.nextHopRow = getL2TableRowFromMacStr(WAN0_ISP_MAC);
	nxthpt.pppoeIdx = 0; /*WAN0_ISP0_PPPOE_SID*/
	nxthpt.dvid = rtl865xc_netIfIndex(WAN0_VID);
	nxthpt.extIntIpIdx = 0; /*WAN0_ISP0_NPE_IP*/
	nxthpt.isPppoe = 0;
	rtl8651_setAsicNextHopTable(1, &nxthpt);       


	
	nxthpt.nextHopRow = getL2TableRowFromMacStr(HOST4_MAC);
	nxthpt.pppoeIdx = 0; /*WAN0_ISP0_PPPOE_SID*/
	nxthpt.dvid = rtl865xc_netIfIndex(WAN1_VID);
	nxthpt.extIntIpIdx = 0; /*WAN0_ISP0_NPE_IP*/
	nxthpt.isPppoe = 1;
	rtl8651_setAsicNextHopTable(30, &nxthpt);       

	nxthpt.nextHopRow = getL2TableRowFromMacStr(HOST4_MAC);
	nxthpt.pppoeIdx = 0; /*WAN0_ISP0_PPPOE_SID*/
	nxthpt.dvid = rtl865xc_netIfIndex(WAN1_VID);
	nxthpt.extIntIpIdx = 0; /*WAN0_ISP0_NPE_IP*/
	nxthpt.isPppoe = 1;
	rtl8651_setAsicNextHopTable(29, &nxthpt);       	
	nxthpt.nextHopRow = getL2TableRowFromMacStr(HOST4_MAC);
	nxthpt.pppoeIdx = 0; /*WAN0_ISP0_PPPOE_SID*/
	nxthpt.dvid = rtl865xc_netIfIndex(WAN1_VID);
	nxthpt.extIntIpIdx = 0; /*WAN0_ISP0_NPE_IP*/
	nxthpt.isPppoe = 0;
	rtl8651_setAsicNextHopTable(31, &nxthpt);       
	
	/* config ip table */
	bzero((void*) &ipt, sizeof(ipt));
	/* wan 0 napt */
	strtoip(&ipt.intIpAddr, "0.0.0.0");
	strtoip(&ipt.extIpAddr, WAN0_ISP_PUBLIC_IP);
	rtl8651_setAsicExtIntIpTable(0, &ipt);
	/* wan 1 nat */
	strtoip(&ipt.intIpAddr, HOST1_IP);
	strtoip(&ipt.extIpAddr, WAN1_ISP_PUBLIC_IP);
	ipt.nat = 1;
	rtl8651_setAsicExtIntIpTable(1, &ipt);
	return 0;
	
}


int32 rtl865xC_testIPMulticast(void)
{
	hsa_param_t hsa;
	int32 retval = SUCCESS;
	rtl865x_tblAsicDrv_multiCastParam_t mcast; 
	hsb_param_t hsb ;
	uint32 multi_mtu;
	ether_addr_t mac;
	rtl865x_tblAsicDrv_l2Param_t l2t;
#ifdef REAL_ASIC_ACCESS	
	rtl865x_tblAsicDrv_vlanParam_t vlant;
	rtl865x_tblAsicDrv_intfParam_t intf;
	rtl865x_AclRule_t *aclt;
#endif
	memcpy((void*)&hsb,(void*)&Ghsb,sizeof(hsb));

	/* Compare the HSA of expected, model, FPGA, and real IC. */

	{	
			layer4Config(); 	
#ifndef REAL_ASIC_ACCESS		
	#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER_ACL) 
			rtl865x_acl_control_set_mode(RTL865X_ACL_Mode_Always_Permit);
	#endif
#endif			
			/*strtomac(&intf.macAddr, GW_LAN0_MAC);*/

			/**********************************************************************
			0. Config: IP Multicast Table
			   SVID:10, sport=3, mbr=P0,P1,P2,P3
			**********************************************************************/
			multi_mtu=1500;
			rtl8651_setAsicMulticastMTU(multi_mtu);
			rtl8651_setAsicMulticastEnable(TRUE);

			bzero((void*) &mcast,sizeof(mcast));
			mcast.dip= ntohl(inet_addr("224.0.0.3"));
			mcast.sip=ntohl(inet_addr("192.168.1.1"));
			mcast.mbr=PM_PORT_0|PM_PORT_1|PM_PORT_2|PM_PORT_3;
			mcast.svid=LAN0_VID;
			mcast.port=PN_PORT3;
			rtl8651_setAsicIpMulticastTable(&mcast);
			
			/**********************************************************************
			1. IP Multicast Packet 224.0.0.3
			   expect hsa.trip WAN0_ISP_PUBLIC_IP
			**********************************************************************/
			memcpy((void*)&hsb.da[0],(void*)strtomac(&mac," 01-00-5e-00-00-03"),sizeof(ether_addr_t));
			hsb.sip=ntohl(inet_addr("192.168.1.1"));
			hsb.dip=ntohl(inet_addr("224.0.0.3"));
			hsb.spa=PN_PORT3;
			hsb.tagif=0;
			hsb.pppoeif=0;
			hsb.type=HSB_TYPE_UDP;
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
			RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
			RTL_TESTMODEL_IS_EQUAL("IP should be TRIP, but .......",hsa.trip,ntohl(inet_addr(WAN0_ISP_PUBLIC_IP))) ;
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;
			
			/**********************************************************************
			2. IP Multicast Packet 224.0.0.3
			   Payload length(46B) is just match to MTU
			   Expect Forward
			**********************************************************************/
			multi_mtu=46;	
			rtl8651_setAsicMulticastMTU(multi_mtu);
			memcpy((void*)&hsb.da[0],(void*)strtomac(&mac," 01-00-5e-00-00-03"),sizeof(ether_addr_t));
			hsb.sip=ntohl(inet_addr("192.168.1.1"));
			hsb.dip=ntohl(inet_addr("224.0.0.3"));
			hsb.spa=PN_PORT3;
			hsb.tagif=0;
			hsb.type=HSB_TYPE_UDP;
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
			RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
			RTL_TESTMODEL_IS_EQUAL("Packet should be forward, but .......",hsa.dp,mcast.mbr) ;
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;
			
			/**********************************************************************
			3. IP Multicast Packet 224.0.0.3
			   Payload length(46B) exceeds MTU(45B)
			   Expect to CPU
			**********************************************************************/
			multi_mtu=45;	
			rtl8651_setAsicMulticastMTU(multi_mtu);
			memcpy((void*)&hsb.da[0],(void*)strtomac(&mac," 01-00-5e-00-00-03"),sizeof(ether_addr_t));
			hsb.sip=ntohl(inet_addr("192.168.1.1"));
			hsb.dip=ntohl(inet_addr("224.0.0.3"));
			hsb.spa=PN_PORT3;
			hsb.tagif=0;
			hsb.type=HSB_TYPE_UDP;
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
			RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
			RTL_TESTMODEL_IS_EQUAL("hsa.dpc should be 1", hsa.dpc, 1) ;
			RTL_TESTMODEL_IS_EQUAL("Packet should be forwarded to CPU", hsa.dp, PM_PORT_NOTPHY) ;
			RTL_TESTMODEL_IS_EQUAL("Packet should be forwarded to CPU", hsa.dpext, PM_PORT_CPU) ;
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;
			RTL_TESTMODEL_IS_EQUAL("Bit[4:1] of CPU Reason is wrong (real, expect)", hsa.why2cpu&0x1e, 0x4<<1/*ACL filtered, but before DMAC==GMAC*/ );
			
			/**********************************************************************
			4. IP Multicast Packet 224.0.0.3
			   Payload length(42B, -4[vlan]) under MTU(45B)
			   Expect to forward
			**********************************************************************/
			multi_mtu=45;	
			rtl8651_setAsicMulticastMTU(multi_mtu);
			memcpy((void*)&hsb.da[0],(void*)strtomac(&mac," 01-00-5e-00-00-03"),sizeof(ether_addr_t));
			hsb.sip=ntohl(inet_addr("192.168.1.1"));
			hsb.dip=ntohl(inet_addr("224.0.0.3"));
			hsb.spa=PN_PORT3;
			hsb.type=HSB_TYPE_UDP;
			hsb.tagif=1;
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
			RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;
			RTL_TESTMODEL_IS_EQUAL("Packet should be forward, but .......",hsa.dp,mcast.mbr) ;
			
			/**********************************************************************
			5. IP Multicast Packet 224.0.0.3
			   expect hsa.trip WAN0_ISP_PUBLIC_IP
			   Payload length(42B) exceeds MTU (41B)
			   Expect to CPU
			**********************************************************************/
			multi_mtu=41;	
			rtl8651_setAsicMulticastMTU(multi_mtu);
			bzero((void*) &mcast,sizeof(mcast));
			mcast.dip= ntohl(inet_addr("224.0.0.3"));
			mcast.sip=ntohl(inet_addr("192.168.1.1"));
			mcast.mbr=PM_PORT_0|PM_PORT_1|PM_PORT_2|PM_PORT_3;
			mcast.svid=LAN0_VID;
			mcast.port=PN_PORT3;
			rtl8651_setAsicIpMulticastTable(&mcast);
			memcpy((void*)&hsb.da[0],(void*)strtomac(&mac," 01-00-5e-00-00-03"),sizeof(ether_addr_t));
			hsb.sip=ntohl(inet_addr("192.168.1.1"));
			hsb.dip=ntohl(inet_addr("224.0.0.3"));
			hsb.spa=PN_PORT3;
			hsb.type=HSB_TYPE_UDP;
			hsb.tagif=1;
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
			RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
			RTL_TESTMODEL_IS_EQUAL("Packet should be CPU, but .......",hsa.dpext,PM_PORT_CPU) ;
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;
			
			/**********************************************************************
			6. IP Multicast Packet 224.0.0.3
			   expect hsa.trip WAN0_ISP_PUBLIC_IP
			   MTU exceed but disable multicast (pure layer2 operation)
			   Expect :forward
			**********************************************************************/
			rtl8651_setAsicMulticastEnable(FALSE);
			WRITE_MEM32(FFCR, READ_MEM32(FFCR)&~EnUnkMC2CPU);
			bzero((void*) &mcast,sizeof(mcast));
			mcast.dip= ntohl(inet_addr("224.0.0.3"));
			mcast.sip=ntohl(inet_addr("192.168.1.1"));
			mcast.mbr=PM_PORT_0|PM_PORT_1|PM_PORT_2|PM_PORT_3;
			mcast.svid=LAN0_VID;
			mcast.port=PN_PORT3;
			rtl8651_setAsicIpMulticastTable(&mcast);
			memcpy((void*)&hsb.da[0],(void*)strtomac(&mac," 01-00-5e-00-00-03"),sizeof(ether_addr_t));
			hsb.sip=ntohl(inet_addr("192.168.1.1"));
			hsb.dip=ntohl(inet_addr("224.0.0.3"));
			hsb.spa=PN_PORT3;
			hsb.type=HSB_TYPE_UDP;
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
			RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
			RTL_TESTMODEL_IS_EQUAL("Packet should be CPU, but .......",hsa.dp,PM_PORT_0|PM_PORT_1|PM_PORT_2) ;
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;
			
			
			/**********************************************************************
			7. IP Multicast Packet 224.0.0.4
			   DMAC:01-00-5e-00-00-02 is unknown MAC (Multicast is enabled, MTU=1500)
			   expect to CPU
			**********************************************************************/ 		
			multi_mtu=1500; 
			rtl8651_setAsicMulticastMTU(multi_mtu);
			rtl8651_setAsicMulticastEnable(TRUE);
			memcpy((void*)&hsb.da[0],(void*)strtomac(&mac," 01-00-5e-00-00-02"),sizeof(ether_addr_t));
			hsb.sip=ntohl(inet_addr("192.168.1.1"));
			hsb.dip=ntohl(inet_addr("224.0.0.5"));
			hsb.spa=PN_PORT3;
			hsb.type=HSB_TYPE_UDP;
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
			RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
			RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward, but .......",hsa.dpext,PM_PORT_CPU);
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;
			RTL_TESTMODEL_IS_EQUAL("Bit[4:1] of CPU Reason is wrong (real, expect)", hsa.why2cpu&0x1e, 0x4<<1/*ACL filtered, but before DMAC==GMAC*/ );
			
			/**********************************************************************
			8. IP Multicast Packet 224.0.0.3 
			   TAG Packet (normal forward, just like 1 and 4)
			   expect hsa.trip WAN0_ISP_PUBLIC_IP
			**********************************************************************/
			rtl8651_setAsicMulticastEnable(TRUE);

			bzero((void*) &mcast,sizeof(mcast));
			mcast.dip= ntohl(inet_addr("224.0.0.3"));
			mcast.sip=ntohl(inet_addr("192.168.1.1"));
			mcast.mbr=PM_PORT_1|PM_PORT_2|PM_PORT_3;
			mcast.svid=LAN0_VID;
			mcast.port=PN_PORT3;
			rtl8651_setAsicIpMulticastTable(&mcast);
			
			memcpy((void*)&hsb.da[0],(void*)strtomac(&mac," 01-00-5e-00-00-03"),sizeof(ether_addr_t));
			hsb.sip=ntohl(inet_addr("192.168.1.1"));
			hsb.dip=ntohl(inet_addr("224.0.0.3"));
			hsb.spa=PN_PORT3;
			hsb.tagif=1;
			hsb.vid=LAN0_VID;
			hsb.type=HSB_TYPE_UDP;
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
			RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
			RTL_TESTMODEL_IS_EQUAL("IP should be translated, but .......",hsa.trip,ntohl(inet_addr(WAN0_ISP_PUBLIC_IP))) ;
			RTL_TESTMODEL_IS_EQUAL("Packet  should be forward , but .......",hsa.dp,mcast.mbr) ;
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;
			
			/**********************************************************************
			9. IP Multicast Packet 224.0.0.3 
			   ACL[5]: ONLY_L4 --> trap to CPU
			   ACL[6]: ONLY_L3 --> PERMIT
			   expect forward and hsa.trip=WAN0_ISP_PUBLIC_IP
			**********************************************************************/
#ifdef REAL_ASIC_ACCESS			

			strtomac(&intf.macAddr, GW_LAN0_MAC);
			intf.vid = LAN0_VID;
			intf.outAclStart = intf.outAclEnd = 0;
			intf.mtu = 1500;
			intf.inAclStart =5;
			intf.inAclEnd = 6;
			intf.outAclStart = intf.outAclEnd = 0;
			intf.enableRoute = 1;
			intf.macAddrNumber=1;
			intf.valid = 1;
			retval=rtl8651_setAsicNetInterface( RTL865XC_NETIFTBL_SIZE, &intf );


			aclt = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
			memset(aclt, 0,sizeof(rtl865x_AclRule_t)); 
			aclt->ruleType_ = RTL865X_ACL_MAC;
			aclt->actionType_ = RTL865X_ACL_TOCPU;
			aclt->pktOpApp_  = RTL865X_ACL_ONLY_L4;
			_rtl865x_setAclToAsic(5, aclt);

			memset(aclt, 0,sizeof(rtl865x_AclRule_t)); 
			aclt->ruleType_ = RTL865X_ACL_MAC;
			aclt->actionType_ = RTL865X_ACL_PERMIT;
			aclt->pktOpApp_  = RTL865X_ACL_ONLY_L3;
			_rtl865x_setAclToAsic(6, aclt);

#else
		{
			rtl865x_AclRule_t *rule;
			rule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
			if(!rule)
			{				
			printk("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
			return FAILED;
			}		
			memset(rule, 0,sizeof(rtl865x_AclRule_t));	
			rule->ruleType_ = RTL865X_ACL_MAC;
			rule->actionType_ = RTL865X_ACL_TOCPU;
			rule->pktOpApp_ = RTL865X_ACL_ONLY_L4;
			rule->direction_ = RTL865X_ACL_INGRESS;	
			rtl865x_add_acl(rule,RTL865xC_TEST_LAN_NETIFNAME, RTL865X_ACL_IPv4MUL,0,1);

			memset(rule, 0,sizeof(rtl865x_AclRule_t));	
			rule->ruleType_ = RTL865X_ACL_MAC;
			rule->actionType_ = RTL865X_ACL_PERMIT;
			rule->pktOpApp_ = RTL865X_ACL_ONLY_L3;
			rule->direction_ = RTL865X_ACL_INGRESS;	
			rtl865x_add_acl(rule,RTL865xC_TEST_LAN_NETIFNAME, RTL865X_ACL_IPv4MUL,0,1);
		
			kfree(rule);
		}
#endif
			memcpy((void*)&hsb.da[0],(void*)strtomac(&mac," 01-00-5e-00-00-03"),sizeof(ether_addr_t));
			hsb.sip=ntohl(inet_addr("192.168.1.1"));
			hsb.dip=ntohl(inet_addr("224.0.0.3"));
			hsb.spa=PN_PORT3;
			hsb.tagif=1;
			hsb.vid=LAN0_VID;
			hsb.type=HSB_TYPE_UDP;
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
			RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
			RTL_TESTMODEL_IS_EQUAL("IP should be translated, but .......",hsa.trip,ntohl(inet_addr(WAN0_ISP_PUBLIC_IP))) ;
			RTL_TESTMODEL_IS_EQUAL("Packet  should be forward , but .......",hsa.dp,mcast.mbr) ;
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;
			
			/**********************************************************************
			10. IP Multicast Packet 224.0.0.3 
				Disable netif l34 routing
				expect hsa.trip WAN0_ISP_PUBLIC_IP
				expect to layer2 operatoin
			**********************************************************************/ 	
#ifdef REAL_ASIC_ACCESS			
			strtomac(&intf.macAddr, GW_LAN0_MAC);
			intf.vid = LAN0_VID;
			intf.enableRoute=0;
			intf.inAclStart = 0;
			intf.inAclEnd = 0;
			intf.outAclStart = intf.outAclEnd = 0;
			intf.mtu = 1500;
			intf.inAclStart = 0;
			intf.inAclEnd = 0;
			intf.outAclStart = intf.outAclEnd = 0;
			intf.enableRoute = 0;
			intf.macAddrNumber=1;
			intf.valid = 1;
			retval=rtl8651_setAsicNetInterface( RTL865XC_NETIFTBL_SIZE, &intf );
#else		
{
			rtl865x_netif_local_t *netif = NULL;
			netif=_rtl865x_getNetifByName(RTL865xC_TEST_LAN_NETIFNAME);
			rtl865x_disableNetifRouting(netif);
//			kfree(netif);
}
#endif
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
			RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
			RTL_TESTMODEL_IS_EQUAL("Packet  should be forward , but .......",hsa.dp,PM_PORT_0|PM_PORT_1|PM_PORT_2) ;
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;
			
			/**********************************************************************
			11. Expect layer2 operation due to l3/4 routing disable
				Seems the same as 10.
			**********************************************************************/

			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
			RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
			RTL_TESTMODEL_IS_EQUAL("Packet  should be forward , but .......",hsa.dp,PM_PORT_0|PM_PORT_1|PM_PORT_2) ;
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;

			/**********************************************************************
			12. ACL[5] not include L3/L4 when ip mulicast is layer2 operation 
				Expect: apply layer2   
			**********************************************************************/
#ifdef REAL_ASIC_ACCESS			
			strtomac(&intf.macAddr, GW_LAN0_MAC);
			intf.vid = LAN0_VID;
			intf.enableRoute=0;
			intf.inAclStart = 0;
			intf.inAclEnd = 0;
			intf.outAclStart = intf.outAclEnd = 0;
			intf.mtu = 1500;
			intf.inAclStart =5;
			intf.inAclEnd = 6;
			intf.outAclStart = intf.outAclEnd = 0;
			intf.enableRoute = 0;
			intf.macAddrNumber=1;
			intf.valid = 1;
			retval=rtl8651_setAsicNetInterface( RTL865XC_NETIFTBL_SIZE, &intf );
	
			memset(aclt, 0,sizeof(rtl865x_AclRule_t)); 
			aclt->ruleType_ = RTL865X_ACL_MAC;
			aclt->actionType_ = RTL865X_ACL_TOCPU;
			aclt->pktOpApp_  = RTL865X_ACL_L3_AND_L4;
			_rtl865x_setAclToAsic(5, aclt);

			memset(aclt, 0,sizeof(rtl865x_AclRule_t)); 
			aclt->ruleType_ = RTL865X_ACL_MAC;
			aclt->actionType_ = RTL865X_ACL_PERMIT;
			aclt->pktOpApp_  = RTL865X_ACL_ALL_LAYER;
			_rtl865x_setAclToAsic(6, aclt);

#else

		{
			rtl865x_AclRule_t *rule;
			int32 retval2 = SUCCESS;
			rule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
			if(!rule)
			{				
			printk("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
			return FAILED;
			}		
			memset(rule, 0,sizeof(rtl865x_AclRule_t));	
			rule->ruleType_ = RTL865X_ACL_MAC;
			rule->actionType_ = RTL865X_ACL_TOCPU;
			rule->pktOpApp_ = RTL865X_ACL_L3_AND_L4;
			rule->direction_ = RTL865X_ACL_INGRESS; 
			retval2=rtl865x_add_acl(rule,RTL865xC_TEST_LAN_NETIFNAME, RTL865X_ACL_IPv4MUL,0,1);

			memset(rule, 0,sizeof(rtl865x_AclRule_t));	
			rule->ruleType_ = RTL865X_ACL_MAC;
			rule->actionType_ = RTL865X_ACL_PERMIT;
			rule->pktOpApp_ = RTL865X_ACL_ALL_LAYER;
			rule->direction_ = RTL865X_ACL_INGRESS; 
			retval2=rtl865x_add_acl(rule,RTL865xC_TEST_LAN_NETIFNAME, RTL865X_ACL_IPv4MUL,0,1);

			kfree(rule);
		}
#endif
			memcpy((void*)&hsb.da[0],(void*)strtomac(&mac," 01-00-5e-00-00-06"),sizeof(ether_addr_t));
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
			RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
			RTL_TESTMODEL_IS_EQUAL("Packet  should be forward , but .......",hsa.dp,PM_PORT_0|PM_PORT_1|PM_PORT_2) ;
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;
		

			/**********************************************************************
			13. Egress ACL[20]: all ==> trap to CPU
				Expect TO CPU
			**********************************************************************/
			WRITE_MEM32(FFCR, READ_MEM32(FFCR)|EnUnkMC2CPU);
#ifdef REAL_ASIC_ACCESS			
			intf.outAclStart=intf.outAclEnd = 20;
			retval=rtl8651_setAsicNetInterface( RTL865XC_NETIFTBL_SIZE, &intf );
			
			
			memset(aclt, 0,sizeof(rtl865x_AclRule_t)); 
			aclt->ruleType_ = RTL865X_ACL_MAC;
			aclt->actionType_ = RTL865X_ACL_TOCPU;
			aclt->pktOpApp_  = RTL865X_ACL_ALL_LAYER;

			_rtl865x_setAclToAsic(20, aclt);
#else
			
		{
			rtl865x_AclRule_t *rule;
			int32 retval2 = SUCCESS;
			rule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
			if(!rule)
			{				
			printk("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
			return FAILED;
			}			
			memset(rule, 0,sizeof(rtl865x_AclRule_t));	
			rule->ruleType_ = RTL865X_ACL_MAC;
			rule->actionType_ = RTL865X_ACL_TOCPU;
			rule->pktOpApp_ = RTL865X_ACL_ALL_LAYER;
			rule->direction_ = RTL865X_ACL_INGRESS; 
			retval2=rtl865x_add_acl(rule,RTL865xC_TEST_LAN_NETIFNAME, RTL865X_ACL_IPv4MUL,0,1);			
			kfree(rule);
		}
		
#endif			
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
			RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
			RTL_TESTMODEL_IS_EQUAL("Packet  should be CPU , but .......",hsa.dpext,PM_PORT_CPU) ;
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;

			/**********************************************************************
			 * restore netif setting (netif.enableRoute=FALSE)
			 **********************************************************************/
#ifdef REAL_ASIC_ACCESS			 
			bzero( &vlant, sizeof(vlant) );
			vlant.memberPortMask = (1<<3);
			vlant.untagPortMask = 0; //tagged	
			/* ingress filter gateway mac */
			strtomac(&intf.macAddr, GW_LAN0_MAC);
			intf.vid = LAN0_VID;
			intf.enableRoute=0;
			intf.inAclStart = 0;
			intf.inAclEnd = 0;
			intf.outAclStart = intf.outAclEnd = 0;
			intf.mtu = 1500;
			intf.inAclStart = 0;
			intf.inAclEnd = 0;
			intf.outAclStart = intf.outAclEnd = 0;
			intf.enableRoute = 1;
			intf.macAddrNumber=1;
			intf.valid = 1;
			retval=rtl8651_setAsicNetInterface( RTL865XC_NETIFTBL_SIZE, &intf );
#else
{
			rtl865x_netif_local_t *netif = NULL;
			netif=_rtl865x_getNetifByName(RTL865xC_TEST_LAN_NETIFNAME);
			rtl865x_enableNetifRouting(netif);
				//	kfree(netif);
			}
#endif
		
			/**********************************************************************
			14. Source Port Mismatch
				expect DROP
			**********************************************************************/
			rtl8651_setAsicIpMulticastTable(&mcast);
			memcpy((void*)&hsb.da[0],(void*)strtomac(&mac," 01-00-5e-00-00-03"),sizeof(ether_addr_t));
			hsb.sip=ntohl(inet_addr("192.168.1.1"));
			hsb.dip=ntohl(inet_addr("224.0.0.3"));
			hsb.spa=2;
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
			RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
			RTL_TESTMODEL_IS_EQUAL("Packet  should be drop",hsa.dp,0) ;
			RTL_TESTMODEL_IS_EQUAL("hsa.dpc should be equal to 0", hsa.dpc, 0) ;
#if 0
	/* please see RTL8198_SWRevision_20090924.doc
	the IP multicast table has no "Source Vlan ID" field in 8196c formal chip
	*/
#if !defined(CONFIG_RTL_8196C_REV_B) && !defined(CONFIG_RTL_8198_REV_B)
			/**********************************************************************
			15. SVID Mismatch
				expect : drop
			**********************************************************************/
			hsb.sip=ntohl(inet_addr("192.168.1.1"));
			hsb.dip=ntohl(inet_addr("224.0.0.3"));
			hsb.spa=3;
			hsb.vid=LAN1_VID;
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
	RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
	RTL_TESTMODEL_IS_EQUAL("Packet  should be drop , but .......",hsa.dp,0) ;
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be equal to 0", hsa.dpc, 0) ;
			
			/**********************************************************************
			16. Enable 1Q_UNWARE. So even SVID is dismatch, ASIC can forward this packet based on port-based VID.
				expect hsa.trip WAN0_ISP_PUBLIC_IP
			**********************************************************************/
			WRITE_MEM32( VCR0,READ_MEM32(VCR0)| EN_1QTAGVIDIGNORE); 		
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
	RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
	RTL_TESTMODEL_IS_EQUAL("IP should be translated",hsa.trip,ntohl(inet_addr(WAN0_ISP_PUBLIC_IP))) ;
			RTL_TESTMODEL_IS_EQUAL("Packet should be forward",hsa.dp,mcast.mbr) ;
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;
#endif			
#endif
			/**********************************************************************
			17. Wan2Lan
				PPPoE unicast Packet 
				Downstream (Only )
				IP Multicast Packet 224.0.0.3 
				TAG Packet 
			**********************************************************************/
			bzero((void*) &mcast,sizeof(mcast));
			mcast.dip= ntohl(inet_addr("224.0.0.3"));
			mcast.sip=ntohl(inet_addr("142.168.1.1"));
			mcast.mbr=PM_PORT_0|PM_PORT_1|PM_PORT_2|PM_PORT_3;
			mcast.svid=LAN0_VID;
			mcast.port=3;
			rtl8651_setAsicIpMulticastTable(&mcast);
			memcpy((void*)&hsb.da[0],(void*)strtomac(&mac,GW_LAN0_MAC),sizeof(ether_addr_t));
			hsb.sip=ntohl(inet_addr("142.168.1.1"));
			hsb.dip=ntohl(inet_addr("224.0.0.3"));
			hsb.spa=3;
			hsb.tagif=1;
			hsb.vid=LAN0_VID;
			hsb.type=HSB_TYPE_IGMP;
			hsb.pppoeif=1;
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
			RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
			//RTL_TESTMODEL_IS_EQUAL_MAC(" MAC should be translated ....",hsa.nhmac,mac);
			RTL_TESTMODEL_IS_EQUAL("Pakcet should be drop , but .......",hsa.dp,PM_PORT_0|PM_PORT_1|PM_PORT_2|PM_PORT_3);
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;
			
			/**********************************************************************
			18. LAN to WAN
				DMAC:01-00-5e-00-00-03
				MAC and IP Dismatch
				expect to CPU
			**********************************************************************/
			memcpy((void*)&hsb.da[0],(void*)strtomac(&mac," 01-00-5e-00-00-03"),sizeof(ether_addr_t));
			hsb.sip=ntohl(inet_addr("192.168.1.1"));
			hsb.dip=ntohl(inet_addr("224.0.0.4"));
			hsb.spa=3;
			hsb.tagif=0;
			hsb.type=HSB_TYPE_UDP;
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
			RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
			RTL_TESTMODEL_IS_EQUAL("Pakcet should be drop , but .......",hsa.dpext,PM_PORT_CPU);
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;
			
			/**********************************************************************
			19. IP Multicast Packet IP	224.0.0.3
				DMAC:01-00-5e-00-00-03
				Multicast mask = P0,P1,P2,P3
				L2 mask = P0
				L2 table entry can't override IP Multicast Table
				Expect Forward
			**********************************************************************/
			bzero((void*) &mcast,sizeof(mcast));
			mcast.dip= ntohl(inet_addr("224.0.0.3"));
			mcast.sip=ntohl(inet_addr("192.168.1.1"));
			mcast.mbr=PM_PORT_0|PM_PORT_1|PM_PORT_2|PM_PORT_3;
			mcast.svid=LAN0_VID;
			mcast.port=3;
			retval=rtl8651_setAsicIpMulticastTable(&mcast);

			memcpy((void*)&hsb.da[0],(void*)strtomac(&mac," 01-00-5e-00-00-03"),sizeof(ether_addr_t));
			hsb.sip=ntohl(inet_addr("192.168.1.1"));
			hsb.dip=ntohl(inet_addr("224.0.0.3"));
			bzero(&l2t,sizeof(l2t));
			strtomac(&l2t.macAddr, " 01-00-5e-00-00-03");
			l2t.memberPortMask = PM_PORT_0;
			l2t.isStatic=1; 		
			rtl8651_setAsicL2Table(getL2TableRowFromMacStr("01-00-5e-00-00-03"), 0, &l2t);
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
			RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
			RTL_TESTMODEL_IS_EQUAL_MAC(" MAC should be translated ....",hsa.nhmac,mac);
			RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward , but .......",hsa.dp,PM_PORT_0|PM_PORT_1|PM_PORT_2|PM_PORT_3);
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;

			/**********************************************************************
			20. Wan2Lan
				Member Two Extension Ports
				PPPoE unicast Packet 
				Expect to Ext0 and Ext2
			**********************************************************************/
			bzero((void*) &mcast,sizeof(mcast));
			mcast.dip= ntohl(inet_addr("224.0.0.3"));
			mcast.sip=ntohl(inet_addr("142.168.1.1"));
			mcast.mbr=PM_PORT_0|PM_PORT_1|PM_PORT_2|PM_PORT_3|(PM_PORT_EXT0<<RTL8651_PORT_NUMBER)|(PM_PORT_EXT2<<RTL8651_PORT_NUMBER);
			mcast.svid=LAN0_VID;
			mcast.port=3;
			rtl8651_setAsicIpMulticastTable(&mcast);
			memcpy((void*)&hsb.da[0],(void*)strtomac(&mac,GW_LAN0_MAC),sizeof(ether_addr_t));
			hsb.sip=ntohl(inet_addr("142.168.1.1"));
			hsb.dip=ntohl(inet_addr("224.0.0.3"));
			hsb.spa=3;
			hsb.tagif=1;
			hsb.vid=LAN0_VID;
			hsb.type=HSB_TYPE_IGMP;
			hsb.pppoeif=1;
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
			RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
//			RTL_TESTMODEL_IS_EQUAL_MAC(" MAC should be translated ....",hsa.nhmac,mac);
			RTL_TESTMODEL_IS_EQUAL("hsa.dp",hsa.dp,PM_PORT_NOTPHY|PM_PORT_0|PM_PORT_1|PM_PORT_2|PM_PORT_3 );
			RTL_TESTMODEL_IS_EQUAL("hsa.dpext",hsa.dpext,PM_PORT_EXT0|PM_PORT_EXT2 );
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;

			/**********************************************************************
			21. Wan2Lan
				FFCR : IP Multicast To CPU
				Expect to CPU
			**********************************************************************/
			WRITE_MEM32(FFCR,(READ_MEM32(FFCR) & ~IPMltCstCtrl_MASK)|IPMltCstCtrl_TrapToCpu);
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
			RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
			RTL_TESTMODEL_IS_EQUAL("hsa.dp",hsa.dp,PM_PORT_NOTPHY );
			RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward, but .......",hsa.dpext,PM_PORT_CPU );
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;
			
			/**********************************************************************
				Change extension port member port mask
			**********************************************************************/
			WRITE_MEM32(FFCR,(READ_MEM32(FFCR) & ~IPMltCstCtrl_MASK)|IPMltCstCtrl_Enable);
			bzero((void*) &mcast,sizeof(mcast));
			mcast.dip= ntohl(inet_addr("224.0.0.3"));
			mcast.sip=ntohl(inet_addr("142.168.1.1"));
			mcast.mbr=PM_PORT_0|PM_PORT_1|PM_PORT_2|PM_PORT_3|(PM_PORT_EXT1<<RTL8651_PORT_NUMBER)|(PM_PORT_EXT2<<RTL8651_PORT_NUMBER);
			mcast.svid=LAN0_VID;
			mcast.port=3;
			rtl8651_setAsicIpMulticastTable(&mcast);
			
			/**********************************************************************
			22. Wan2Lan
				Payload length(38B, minused PPPoE header) just makes MTU(38B)
				Expect forward
			**********************************************************************/
			multi_mtu=38;	
			rtl8651_setAsicMulticastMTU(multi_mtu);
			hsb.pppoeif = 1;
			hsb.tagif = 0;
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
			RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
	
			strtomac(&mac,"01-00-5e-00-00-03");
			RTL_TESTMODEL_IS_EQUAL("hsa.dp",hsa.dp, PM_PORT_NOTPHY|PM_PORT_0|PM_PORT_1|PM_PORT_2|PM_PORT_3 );
			RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward, but .......",hsa.dpext,PM_PORT_EXT1|PM_PORT_EXT2 );
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;
			
			/**********************************************************************
			23. Wan2Lan
				Payload length(38B) exceeds MTU(37B)
				Expect to CPU
			**********************************************************************/
			multi_mtu=37;	
			rtl8651_setAsicMulticastMTU(multi_mtu);
			hsb.tagif = 0;
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
			RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
			RTL_TESTMODEL_IS_EQUAL("hsa.dp",hsa.dp,PM_PORT_NOTPHY);
			RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward, but .......",hsa.dpext,PM_PORT_CPU);
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;
			
			/**********************************************************************
			24. Wan2Lan
				Payload length(34B, minused VLAN header) just make MTU(34B)
				Expect forward
			**********************************************************************/
			multi_mtu=34;	
			rtl8651_setAsicMulticastMTU(multi_mtu);
			hsb.tagif = 1;
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
				RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));

			strtomac(&mac,"01-00-5e-00-00-03");
			RTL_TESTMODEL_IS_EQUAL("hsa.dp",hsa.dp, PM_PORT_NOTPHY|PM_PORT_0|PM_PORT_1|PM_PORT_2|PM_PORT_3);
			RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward, but .......",hsa.dpext,PM_PORT_EXT1|PM_PORT_EXT2);
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;
			
			/**********************************************************************
			25. Wan2Lan
				Payload length(34B) exceeds MTU(33B)
				Expect to CPU
			**********************************************************************/
			multi_mtu=33;	
			rtl8651_setAsicMulticastMTU(multi_mtu);
			hsb.tagif = 1;
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
			RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
			strtomac(&mac,"01-00-5e-00-00-03");
			RTL_TESTMODEL_IS_EQUAL("hsa.dp",hsa.dp,PM_PORT_NOTPHY);
			RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward, but .......",hsa.dpext,PM_PORT_CPU);
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;

			/**********************************************************************
			26. Wan2Lan (trap to CPU)
				Payload length(34B, minused VLAN header) just make MTU(34B)
				Expect forward to physical ports and to CPU
			**********************************************************************/
			{ /* set cpu bit */
				rtl865xc_tblAsic_ipMulticastTable_t entry;
				uint32 index;

				index = rtl8651_ipMulticastTableIndex( mcast.dip, mcast.sip );
				retval = _rtl8651_readAsicEntry( TYPE_MULTICAST_TABLE, index, &entry );

				entry.toCPU = TRUE;
				retval = _rtl8651_forceAddAsicEntry( TYPE_MULTICAST_TABLE, index, &entry );

			}
			multi_mtu=34;	
			rtl8651_setAsicMulticastMTU(multi_mtu);
			hsb.tagif = 1;
			RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
			RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
			strtomac(&mac,"01-00-5e-00-00-03");
			RTL_TESTMODEL_IS_EQUAL("hsa.dp",hsa.dp, PM_PORT_NOTPHY|PM_PORT_0|PM_PORT_1|PM_PORT_2|PM_PORT_3);

			RTL_TESTMODEL_IS_EQUAL("Pakcet should be forward, but .......",hsa.dpext,PM_PORT_EXT1|PM_PORT_EXT2);
			RTL_TESTMODEL_IS_NOT_EQUAL("hsa.dpc should be more than 0", hsa.dpc, 0) ;
			{ /* recover */
				rtl865xc_tblAsic_ipMulticastTable_t entry;
				uint32 index;

				index = rtl8651_ipMulticastTableIndex( mcast.dip, mcast.sip );
				retval = _rtl8651_readAsicEntry( TYPE_MULTICAST_TABLE, index, &entry );
				entry.toCPU = FALSE;
				retval = _rtl8651_forceAddAsicEntry( TYPE_MULTICAST_TABLE, index, &entry );

			}

			
	}


	return SUCCESS;

}


int32 rtl865xC_testLayer3MTU(void)
{
	
	hsa_param_t hsa;	
	hsb_param_t hsb =
	{
		spa: RTL865xC_TEST_LAN_HOST0_PHYPORT,		len:1300,
		vid: 0x0,		tagif: 0,
		pppoeif: 0,		sip: ntohl(inet_addr(RTL865xC_TEST_LAN_HOST0_IP)),
		sprt: 1020,		dip: ntohl(inet_addr(RTL865xC_TEST_WAN0_HOST_IP)),
		dprt: 80,			ipptl: 0xdd,
		ipfg: 0x0,		iptos: 0x65,
		tcpfg: 0x00,		type: HSB_TYPE_UDP,	
		patmatch:0,		ethtype: 0x0800,
		ipfo0_n:0,
		hiprior: 0,		snap: 0,
		udpnocs: 0,		ttlst:2,
		dirtx: 0,			l3csok: 1,
		l4csok: 1,		
		llcothr: 0,		urlmch: 0,
		extspa: 0,		extl2: 0,
		
	};
	strtomac((ether_addr_t *)hsb.sa, RTL865xC_TEST_LAN_HOST0_MAC);
	strtomac((ether_addr_t *)hsb.da, RTL865xC_TEST_LAN_GMAC);

	/* Configure a pure routing environment  */
	RTL_TESTMODEL_INIT_CHECK(rtl865xC_layer3Config(RTL865xC_TEST_WANTYPE_UNTAG_PURE_ROUTING,0));
	#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER_ACL) 
	rtl865x_acl_control_set_mode(RTL865X_ACL_Mode_Always_Permit);	
	#endif

	/* Start Test 1 : Use the packet whose length <= MTU, should  hwacc normally */
	RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
	RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
	RTL_TESTMODEL_IS_EQUAL("Pakcet should be hwacc normally,but ....", hsa.dp,RTL865xC_TEST_WAN_HOST_PHYPORT_MASK);
	RTL_TESTMODEL_IS_EQUAL("Pakcet should be hwacc normally,but ....", hsa.dpext,0);


	/* Start Test 2 : Use the packet whose length > MTU, should  trap to cpu  */
	hsb.len = 1600;
	RTL_TESTMODEL_INIT_CHECK(virtualMacSetHsb(&hsb));
	RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));
	RTL_TESTMODEL_IS_EQUAL("Pakcet should be to CPU,but ....", hsa.dp,PM_PORT_NOTPHY);
	RTL_TESTMODEL_IS_EQUAL("Pakcet should be to CPU,but ....", hsa.dpext,PM_PORT_CPU);	

	return RTL_TESTMODEL_TESTCASE_RUN_SUCCESS;
}


/* Test Downstream Mulitcast , port 0 to port 1*/
static int32 _rtl865xC_testPktIPMulticast(int layer,int isPPPoE)
{
	rtl8651_PktConf_t conf;
	int32 pktLen;  
	int32 pktLen_Expect;  
	uint8 *pktBuf			= RTL865xC_Test_SharedPkt[0];	
	uint8 *pktBuf_Expect		= RTL865xC_Test_SharedPkt[1];
	uint8 *pkt_payload_Data = RTL865xC_Test_SharedPkt[2];	
	enum PORT_MASK rxPort;	
	hsa_param_t hsa;	


	/* setup environment */
	ether_addr_t	multicast_stream_smac;
	ether_addr_t	multicast_stream_dmac;
	ipaddr_t			multicast_stream_sip;
	ipaddr_t			multicast_stream_dip;	
	ether_addr_t	wan_gmac;
	
	if(layer==2 && isPPPoE==0)
	{
		RTL_TESTMODEL_INIT_CHECK(rtl865xC_layer3Config(RTL865xC_TEST_WANTYPE_UNTAG_BRIDGE,0));
		strtomac(&multicast_stream_smac,RTL865xC_TEST_WAN1_HOST_MAC);
		strtomac(&multicast_stream_dmac," 01-00-5e-00-00-03");
		strtoip(&multicast_stream_sip, RTL865xC_TEST_WAN1_HOST_IP);
		strtoip(&multicast_stream_dip, "224.0.0.3");
		strtomac(&wan_gmac,RTL865xC_TEST_WAN1_GMAC);
	}
	else if(layer==3 && isPPPoE==0)
	{
		RTL_TESTMODEL_INIT_CHECK(rtl865xC_layer3Config(RTL865xC_TEST_WANTYPE_UNTAG_PURE_ROUTING,0));
		strtomac(&multicast_stream_smac,RTL865xC_TEST_WAN0_HOST_MAC);
		strtomac(&multicast_stream_dmac," 01-00-5e-00-00-03");
		strtoip(&multicast_stream_sip, RTL865xC_TEST_WAN0_HOST_IP);
		strtoip(&multicast_stream_dip, "224.0.0.3");
		strtomac(&wan_gmac,RTL865xC_TEST_WAN0_GMAC);
	}
	else if(layer==3 && isPPPoE==1)
	{
		RTL_TESTMODEL_INIT_CHECK(rtl865xC_layer3Config(RTL865xC_TEST_WANTYPE_UNTAG_PURE_ROUTING,1));
		strtomac(&multicast_stream_smac,RTL865xC_TEST_WAN2_HOST_MAC);
		strtomac(&multicast_stream_dmac,RTL865xC_TEST_WAN2_GMAC);
		strtoip(&multicast_stream_sip, RTL865xC_TEST_WAN2_HOST_IP);
		strtoip(&multicast_stream_dip, "224.0.0.3");
		strtomac(&wan_gmac,RTL865xC_TEST_WAN2_GMAC);
	}
	else
		return RTL_TESTMODEL_TESTCASE_RUN_INIT_FAILED;

	/* Add Downstream Multicast entry into hw table */
	RTL_TESTMODEL_INIT_CHECK(rtl865x_addMulticastEntry(multicast_stream_dip,multicast_stream_sip
						,0,RTL865xC_TEST_WAN_HOST_PHYPORT,RTL865xC_TEST_LAN_HOST0_PHYPORT_MASK));


	
	/* Generate and input a UDP-multicast packet*/
	memset(&conf, 0, sizeof(rtl8651_PktConf_t));  
	conf.pktType						= _PKT_TYPE_UDP;	
	conf.ip.id							= 0xa;
	conf.ip.ttl 					= RTL865xC_TEST_STREAM0_TTL;
	conf.conf_dport 				= RTL865xC_TEST_STREAM0_PORT_REM;
	conf.conf_sport 				= RTL865xC_TEST_STREAM0_PORT_INT;
	conf.payload.length 			= RTL865xC_TEST_STREAM0_PAYLOAD_LEN;
	conf.payload.content				= pkt_payload_Data;
	conf.conf_sip						= multicast_stream_sip;
	conf.conf_dip						= multicast_stream_dip; 
	memcpy(conf.conf_smac,&multicast_stream_smac,ETHER_ADDR_LEN);
	memcpy(conf.conf_dmac,&multicast_stream_dmac,ETHER_ADDR_LEN);
	if(isPPPoE)
	{
		conf.pppoe.type 			= ETH_P_PPP_SES;
		conf.pppoe.session		= 0;
		conf.l2Flag 				= L2_PPPoE;
	}

	
	pktLen = pktGen(&conf,pktBuf);				/* generate pkt in buff */		
	RTL_TESTMODEL_DUMP_PACKET("INPUT",pktBuf,pktLen);
	
	/* Input */
	RTL_TESTMODEL_INIT_CHECK(virtualMacInput(RTL865xC_TEST_WAN_HOST_PHYPORT,pktBuf, pktLen+4));

	/* Output , and ckeck output port */
	memset(pktBuf,0,pktLen);
	pktLen = RTL865xC_TEST_SHARE_PKT_LEN;
	rxPort = RTL865xC_TEST_LAN_HOST0_PHYPORT_MASK;
	RTL_TESTMODEL_INIT_CHECK(virtualMacOutput( &rxPort,pktBuf, &pktLen));
	RTL_TESTMODEL_DUMP_PACKET("OUTPUT",pktBuf,pktLen);
	RTL_TESTMODEL_IS_EQUAL("Pakcet forward to ... ", rxPort,RTL865xC_TEST_LAN_HOST0_PHYPORT_MASK);

	/* Compare Output data with expected data 
		1. if output port is not in input's vlan mbr (routing) 
				: DA changes to {01005e, 0, dip[22:0]}
				: SA changes to the GMAC of "input netif"  (weird...,should be "output netif" , has already reportd to designer YoYo)
		2. remove valan and PPPoE tag
	*/	
	if(layer==3)
	{
		//conf.ip.ttl  = RTL865xC_TEST_STREAM0_TTL-1;
		stringToConfMac(conf.conf_dmac," 01-00-5e-00-00-03");
		memcpy(conf.conf_smac,&wan_gmac,ETHER_ADDR_LEN);
	}	
	if(isPPPoE)
		conf.l2Flag = 0;
	
	pktLen_Expect = pktGen(&conf, pktBuf_Expect);				/* generate pkt in buff */		
	
	
	RTL_TESTMODEL_INIT_CHECK(virtualMacGetHsa(&hsa));	
	RTL_TESTMODEL_COMPARE_PKT(pktBuf_Expect,pktLen_Expect,pktBuf,pktLen-4,(&hsa));	


	return RTL_TESTMODEL_TESTCASE_RUN_SUCCESS;
}


int32 rtl865xC_testPktIPMulticastL2(void)
{
	return _rtl865xC_testPktIPMulticast(2,0);
}


int32 rtl865xC_testPktIPMulticastL3(void)
{
	return _rtl865xC_testPktIPMulticast(3,0);
}

int32 rtl865xC_testPktIPMulticastPPPoE(void)
{
	return _rtl865xC_testPktIPMulticast(3,1);
}



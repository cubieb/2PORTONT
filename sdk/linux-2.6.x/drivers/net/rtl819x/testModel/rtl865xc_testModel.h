/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Header File for IC-specific Function
* Abstract : 
* Author : Louis Yung-Chieh Lo (yjlou@realtek.com.tw)               
* $Id: rtl865xc_testModel.h,v 1.3 2012/10/24 04:31:55 ikevin362 Exp $
*/

#ifndef _RTL865XC_IC_MODEL_
#define _RTL865XC_IC_MODEL_

#include <net/rtl/rtl_types.h>

#define RTL865xC_TEST_SHARE_PKT_LEN (16*1024+64) /* packet length for sharedPkt */
#define RTL865xC_TEST_SHARE_PKT_NUM	3
extern uint8 RTL865xC_Test_SharedPkt[RTL865xC_TEST_SHARE_PKT_NUM][RTL865xC_TEST_SHARE_PKT_LEN];

#include "testModel.h"
#include "virtualMac.h"
#include "../AsicDriver/rtl865x_asicL2.h"
#include "../AsicDriver/rtl865xC_hs.h"
#include "../AsicDriver/rtl865xc_asicregs.h"
#include "../AsicDriver/rtl865x_asicCom.h"


/*common*/
#include "../common/rtl865x_eventMgr.h"
#include "../common/rtl865x_vlan.h"
#include <net/rtl/rtl865x_netif.h>
#include "../common/rtl865x_netif_local.h"

#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER_ACL) 
#include "../acl_control/rtl865x_acl_control.h"
#endif

/*l2*/
#ifdef CONFIG_RTL_LAYERED_DRIVER_L2
#include "../l2Driver/rtl865x_fdb.h"
#endif

/*l3*/
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
#include "../l3Driver/rtl865x_ip.h"
#include "../l3Driver/rtl865x_nexthop.h"
#include <net/rtl/rtl865x_ppp.h>
#include "../l3Driver/rtl865x_ppp_local.h"
#include "../l3Driver/rtl865x_route.h"
#include "../l3Driver/rtl865x_arp.h"
#include <net/rtl/rtl865x_nat.h>
#endif





enum HSB_TYPE 
{
	HSB_TYPE_ETHERNET = 0,
	HSB_TYPE_PPTP = 1,
	HSB_TYPE_IP = 2,
	HSB_TYPE_ICMP = 3,
	HSB_TYPE_IGMP = 4,
	HSB_TYPE_TCP = 5,
	HSB_TYPE_UDP = 6,
	HSB_TYPE_IPV6 = 7,
};

enum HSA_TYPE 
{
	HSA_TYPE_ETHERNET = 0,
	HSA_TYPE_PPTP = 1,
	HSA_TYPE_IP = 2,
	HSA_TYPE_ICMP = 3,
	HSA_TYPE_IGMP = 4,
	HSA_TYPE_TCP = 5,
	HSA_TYPE_UDP = 6,
	HSA_TYPE_IPV6 = 7,
};

/* Port number for 'spa' */
enum PORT_NUM
{
	PN_PORT0 = 0,
	PN_PORT1 = 1,
	PN_PORT2 = 2,
	PN_PORT3 = 3,
	PN_PORT4 = 4,
	PN_PORT5 = 5,
	PN_PORT_NOTPHY = 6,
};

enum EXTPORT_NUM
{
	PN_PORT_EXT0 = 0,
	PN_PORT_EXT1 = 1,
	PN_PORT_EXT2 = 2,
	PN_PORT_CPU = 3,
};

/* Port number for 'dp' & 'dpext'*/
enum PORT_MASK
{
	PM_PORT_0 = (1<<PN_PORT0),
	PM_PORT_1 = (1<<PN_PORT1),
	PM_PORT_2 = (1<<PN_PORT2),
	PM_PORT_3 = (1<<PN_PORT3),
	PM_PORT_4 = (1<<PN_PORT4),
	PM_PORT_5 = (1<<PN_PORT5),
	PM_PORT_NOTPHY = (1<<PN_PORT_NOTPHY),
	PM_PORT_ALL = (1<<PN_PORT0)|(1<<PN_PORT1)|(1<<PN_PORT2)|(1<<PN_PORT3)|(1<<PN_PORT4)|(1<<PN_PORT5)|(1<<PN_PORT_NOTPHY),	
};

enum EXTPORT_MASK
{
	PM_PORT_EXT0 = (1<<PN_PORT_EXT0), /* PN_PORT_EXT0 is 0. For uniform port mask, use 'PM_PORT_EXT0<<RTL8651_PORT_NUMBER' */
	PM_PORT_EXT1 = (1<<PN_PORT_EXT1), /* PN_PORT_EXT1 is 1. For uniform port mask, use 'PM_PORT_EXT1<<RTL8651_PORT_NUMBER' */
	PM_PORT_EXT2 = (1<<PN_PORT_EXT2), /* PN_PORT_EXT2 is 2. For uniform port mask, use 'PM_PORT_EXT2<<RTL8651_PORT_NUMBER' */
	PM_PORT_CPU = (1<<PN_PORT_CPU),
	PM_EXTPORT = (1<<PN_PORT_CPU)|(1<<PN_PORT_EXT0)|(1<<PN_PORT_EXT1)|(1<<PN_PORT_EXT2),
};



/* RTL865xC virtualMac Functions */
int32 rtl865xC_virtualMacInit( void );
int32 rtl865xC_virtualMacInput( enum PORTID fromPort, uint8* packet, int32 len );
int32 rtl865xC_virtualMacOutput( uint32 *toPort, uint8* packet, int32 *len );

int32 rtl865xC_virtualMacSetHsb( hsb_t rawHsb );
int32 rtl865xC_virtualMacSetHsa( hsa_t rawHsa );

int32 rtl865xC_convertHsbToAsic( hsb_param_t* hsb, hsb_t* rawHsb );
int32 rtl865xC_convertHsbToSoftware( hsb_t* rawHsb, hsb_param_t* hsb );
int32 rtl865xC_convertHsaToAsic( hsa_param_t* hsa, hsa_t* rawHsa );


#endif


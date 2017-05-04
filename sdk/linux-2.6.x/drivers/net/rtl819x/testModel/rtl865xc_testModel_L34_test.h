/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Header File for IC-specific Function
* Abstract : 
* Author : Louis Yung-Chieh Lo (yjlou@realtek.com.tw)               
* $Id: rtl865xc_testModel_L34_test.h,v 1.3 2012/10/24 04:49:06 ikevin362 Exp $
*/

#ifndef _RTL865XC_IC_MODEL_L34_TEST_
#define _RTL865XC_IC_MODEL_L34_TEST_


#include <net/rtl/rtl_types.h>

int32 rtl865xC_testLayer3MTU(void);

int32 rtl865xC_testPktIPMulticastL2( void );
int32 rtl865xC_testPktIPMulticastL3( void );
int32 rtl865xC_testPktIPMulticastPPPoE( void );


int32 rtl865xC_testLayer3GuestVLAN(void);
int32 rtl865xC_testLayer3Routing(void);
int32 rtl865xC_testLayer3RoutingToCpu(void);
int32 rtl865xC_testIPMulticast(void);
int32 rtl865xC_testEnhancedGRE(void);
int32 rtl865xC_testIpOther(void);
int32 rtl865xC_testLayer3RoutingToCpu(void);



#endif


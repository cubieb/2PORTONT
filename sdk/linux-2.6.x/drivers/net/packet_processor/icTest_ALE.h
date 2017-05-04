/*
* Copyright (c) Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Header file of Test case for RTL8672 ALE
* Abstract : 
* Author : Michael Mao-Lin Huang (michaelhuang@realtek.com.tw)
* $Id: icTest_ALE.h,v 1.1.1.1 2010/05/05 09:00:48 jiunming Exp $
*/

#ifndef	__ICTEST_ALE_H__
#define	__ICTEST_ALE_H__

extern uint8 	*pVirtualPktBuffer;

int32 testALE_Example(uint32 caseNo);
int32 testALE_PNP(uint32 caseNo);

int32 testALE_L2Table(uint32 caseNo);
int32 testALE_L4Table(uint32 caseNo);
int32 testALE_ReasonPriority(uint32 caseNo);

int32 testALE_SMACCheck(uint32 caseNo);

/* L2 Bridge Case */
int32 testALE_L2BridgeS2N(uint32 caseNo);	/* Sar to Nic */
//int32 testALE_L2BridgeS2W(uint32 caseNo);	/* Sar to Wireless */
int32 testALE_Generic(uint32 caseNo);
/* ALE model behavior test */
int32 testALE_L2ModelFlow(uint32 caseNo);
int32 testALE_L34ModelFlow(uint32 caseNo);

/* ALE init function */
int32 testALE_Init(void);
/* ALE reset function */
int32 _ale_reset_hsb(hsb_param_t *hsb);
int32 testALE_Reset(void);



#endif	/* __ICTEST_ALE_H__ */


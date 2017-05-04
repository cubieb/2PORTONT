/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Header File of Model Code for RTL8672
* Abstract : 
* Author : Michael Mao-Lin Huang (michaelhuang@realtek.com.tw)
* $Id: icTest.h,v 1.1.1.1 2010/05/05 09:00:48 jiunming Exp $
*/

#ifndef __IC_TEST_H__
#define __IC_TEST_H__

#include "rtl_glue.h"
#include "hsModel.h"
#include "virtualMac.h"
#include "rtl8672_tblAsicDrv.h"


#define ASSERT(value)\
do {\
	if (!((int32)(value))) {\
		rtlglue_printf("\033[31;43m%s(%d): errno=%d, fail, so abort!\033[m\n", __FUNCTION__, __LINE__, 0);\
		return FAILED; \
	}\
}while (0)

#define ASSERT_EQ(value1,value2)\
do {\
	if ((value1) != (value2)) {\
		rtlglue_printf("\033[31;43m%s(%d): value1=0x%x value2=0x%x, fail, so abort!\033[m\n", __FUNCTION__, __LINE__,(value1),(value2));\
		return FAILED; \
	}\
}while (0)

#define IS_NOT_EQUAL_INT_DETAIL(str, v1, v2, _f_, _l_,i) \
	do{if ((v1) == (v2)) { \
		rtlglue_printf("\t%s(%u): %s\n\t%s: %d(0x%08x), %d(0x%08x)\n", _f_, _l_,i==IC_TYPE_REAL?"IC MODE":"MODEL MODE", str, v1, v1, v2, v2); \
		return FAILED; \
	}}while(0)

#define IS_EQUAL_INT_DETAIL(str, v1, v2, _f_, _l_,i) \
	do{if ((v1) != (v2)) { \
		rtlglue_printf("\t%s(%u): %s\n\t%s: %d(0x%08x), %d(0x%08x)", _f_, _l_, i==IC_TYPE_REAL?"IC MODE":"MODEL MODE",str, v1, v1,v2,v2); \
		return FAILED; \
	}}while(0)


#define IS_EQUAL_MAC_DETAIL(str, v1, v2, _f_, _l_,i) \
do { \
	if (memcmp((int8 *)&(v1), (int8 *)&(v2), 6)) { \
		rtlglue_printf("\t%s(%u):%s\n\t %s: %02x:%02x:%02x:%02x:%02x:%02x, %02x:%02x:%02x:%02x:%02x:%02x\n", _f_, _l_,i==IC_TYPE_REAL?"IC MODE":"MODEL MODE", str, \
		*( (uint8*)&v1+0),*( (uint8*)&v1+1),*( (uint8*)&v1+2),*( (uint8*)&v1+3),*( (uint8*)&v1+4),*( (uint8*)&v1+5),\
		*( (uint8*)&v2+0),*( (uint8*)&v2+1),*( (uint8*)&v2+2),*( (uint8*)&v2+3),*( (uint8*)&v2+4),*( (uint8*)&v2+5));\
		return FAILED; \
	} \
} while(0)

int32 runModelTest(uint32 userId,  int32 argc,int8 **saved);

/* Condition items mask */
#define CND_INIFTYPE		0x00000003
#define CND_OUTIFTYPE		0x0000000c
#define CND_OUTL2ENCAP		0x00000010
#define CND_OUTLANFCS		0x00000020
#define CND_OUTCOMPPPP		0x00000040
#define CND_DSCPREMR		0x00000080
#define CND_PIF				0x00000100
#define CND_TIF				0x00000200
#define CND_L3CHANGE		0x00000400
#define CND_L4CHANGE		0x00000800
#define CND_L3TYPE			0x00001000
#define CND_INL2ENCAP		0x00002000
#define CND_OUTSARHDR		0x0001c000
#define CND_FRAMECTRL		0x00060000
#define CND_INSARHDR		0x00380000
#define CND_MAX				0x003fffff

/* Bitmask for test code and model export */
#define AUTO_CASE_NO 0 /* auto-increased Case No */
#define GRP_NONE			       		0x80000000
#define GRP_ALL		         			0x00000001
#define GRP_PARSER				   	0x00000002
#define GRP_TRANSLATOR					0x00000004
#define GRP_RINGCONTROLLER				0x00000008
#define GRP_ALE						0x00000010


#define IS_EQUAL_INT(str, v1, v2, _f_, _l_) \
do { \
	if ((v1) != (v2)) { \
		rtlglue_printf("\t%s(%u): %s: %d(0x%08x), %d(0x%08x)\n", _f_, _l_, str, v1, v1, v2, v2); \
		return FAILED; \
	} \
} while(0)	

int32 runModelTest(uint32 userId,  int32 argc,int8 **saved);
void dumpHSB(hsb_param_t *hsb);
void dumpHSA(hsa_param_t *hsa);
void dumpHSBP(rtl8672_tblAsic_hsbpTable_t *hsbp);
int32 compHsb( hsb_param_t* hsb1, hsb_param_t* hsb2 );
int32 compHsa( hsa_param_t* hsa1, hsa_param_t* hsa2 );
int32 compHsbp( rtl8672_tblAsic_hsbpTable_t* hsbp1, rtl8672_tblAsic_hsbpTable_t* hsbp2 );
int32   rtl8672_dumpCmd(uint32 userId,  int32 argc,int8 **saved);
int32   rtl8672_addCmd(uint32 userId,  int32 argc,int8 **saved);
int32   rtl8672_delCmd(uint32 userId,  int32 argc,int8 **saved);
int32   runModelTarget(uint32 userId,  int32 argc,int8 **saved);
int rtl8672_dumpRing(int ring,int idx);

#endif	/* __IC_TEST_H__ */


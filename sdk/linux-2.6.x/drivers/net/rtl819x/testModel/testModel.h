/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Header File for Virtual MAC
* Abstract : 
* Author : Louis Yung-Chieh Lo (yjlou@realtek.com.tw)               
* $Id: testModel.h,v 1.4 2012/10/24 04:50:19 ikevin362 Exp $
*/

#ifndef _TEST_MODEL_
#define _TEST_MODEL_

#include <net/rtl/rtl_types.h>

/* Bitmask for test code and model export */
#define AUTO_CASE_NO 		0 /* auto-increased Case No */
#define GRP_NONE            	0x80000000
#define GRP_ALL             	0x00000001
#define GRP_PKT_PARSER      	0x00000002
#define GRP_PKT_TRANSLATOR  	0x00000004
#define GRP_PRE_CORE        	0x00000008
#define GRP_L2              	0x00000010
#define GRP_L34             	0x00000020
#define GRP_POST_CORE       	0x00000040
#define GRP_GDMA				0x00000080
#define GRP_NIC				0x00000100
#define GRP_NAPT				0x00000200
#define GRP_FT2				0x00000400

#define DBG_RTL_TESTMODEL_DUMP_PACKET

#define RTL_TESTMODEL_TESTCASE_RUN_SUCCESS		0
#define RTL_TESTMODEL_TESTCASE_RUN_INIT_FAILED	-1
#define RTL_TESTMODEL_TESTCASE_RUN_FAILED		-2

#define RTL_TESTMODEL_IS_NOT_EQUAL(str, v1, v2) \
do { \
	if ((v1) == (v2)) { \
		rtlglue_printf("\t%s(%u): %s: %d(0x%08x)  !=  %d(0x%08x)\n", __func__, __LINE__, str, v1, v1, v2, v2); \
		return RTL_TESTMODEL_TESTCASE_RUN_FAILED; \
	} \
} while(0)	
#define RTL_TESTMODEL_IS_EQUAL_MAC(str, v1, v2) \
	do { \
		if (memcmp((int8 *)&(v1), (int8 *)&(v2), 6)) { \
			rtlglue_printf("\t%s(%u): %s: %x:%x:%x:%x:%x:%x, %x:%x:%x:%x:%x:%x\n", __func__, __LINE__, str, \
			*( (int8*)&v1+0),*( (int8*)&v1+1),*( (int8*)&v1+2),*( (int8*)&v1+3),*( (int8*)&v1+4),*( (int8*)&v1+5),\
			*( (int8*)&v2+0),*( (int8*)&v2+1),*( (int8*)&v2+2),*( (int8*)&v2+3),*( (int8*)&v2+4),*( (int8*)&v2+5));\
			return FAILED; \
		} \
	} while(0)


#define RTL_TESTMODEL_IS_EQUAL(str, v1, v2) \
do { \
	if ((v1) != (v2)) { \
		rtlglue_printf("\t%s(%u): %s: %d(0x%08x)  !=  %d(0x%08x)\n", __func__, __LINE__, str, v1, v1, v2, v2); \
		return RTL_TESTMODEL_TESTCASE_RUN_FAILED; \
	} \
} while(0)	

#define RTL_TESTMODEL_INIT_CHECK(expr) \
do { \
	if(((int32)expr)!=SUCCESS){\
		rtlglue_printf("\t%s(%u):  init failed....\n", __func__, __LINE__); \
		return RTL_TESTMODEL_TESTCASE_RUN_INIT_FAILED; \
	} \
} while(0)

#define RTL_TESTMODEL_INIT_CHECK_SUCCESS(expr) \
do { \
	if(((int32)expr)!=SUCCESS){\
		rtlglue_printf("\t%s(%u):  init failed....\n", __func__, __LINE__); \
		return FAILED; \
	} \
} while(0)

void rtl_testModel_dumpPacket(char* data,int len);


#ifdef DBG_RTL_TESTMODEL_DUMP_PACKET
#define RTL_TESTMODEL_DUMP_PACKET(string,data,len) \
do { \
	printk("===(%s)=== data:0x%p len:%d==(%s  %d)======\n",string,data,len,__func__,__LINE__); \
	rtl_testModel_dumpPacket(data,len); \
} while(0)
#else
#define RTL_TESTMODEL_DUMP_PACKET(string,data,len) \
do { \
} while(0)
#endif


#define RTL_TESTMODEL_COMPARE_PKT(pktBuf_expect, pktLen_expect, pktBuf_check, pktLen_check, hsa_check) \
do { \
	int packet_check_ptr; \
	if(pktLen_expect!=pktLen_check) { \
		rtlglue_printf("\t%s(%u):  Packet Len  %d (expect)  !=  %d (check) \n", __func__, __LINE__, pktLen_expect, pktLen_check); \
		return RTL_TESTMODEL_TESTCASE_RUN_FAILED; \
	} \
	for(packet_check_ptr=0 ; packet_check_ptr<pktLen_expect ; packet_check_ptr++) { \
		if(hsa_check->l2tr==0) \
		{ \
			if(packet_check_ptr>=0 && packet_check_ptr<=5) \
			{ \
				if(hsa_check->nhmac[packet_check_ptr] != pktBuf_expect[packet_check_ptr]) \
				{ \
					rtlglue_printf("\t%s(%u):  Compare DA failed.... \n", __func__, __LINE__); \
					rtlglue_printf("=======   HSA's DA =========\n"); \
					rtlglue_printf("%02X-%02X-%02X-%02X-%02X-%02X\n" \
						,hsa_check->nhmac[0],hsa_check->nhmac[1],hsa_check->nhmac[2] \
						,hsa_check->nhmac[3],hsa_check->nhmac[4],hsa_check->nhmac[5]); \
					rtlglue_printf("=======   pktBuf_expect =========\n"); \
					rtl_testModel_dumpPacket(pktBuf_expect,pktLen_expect); \
					return RTL_TESTMODEL_TESTCASE_RUN_FAILED; \
				} \
			} \
			else if(packet_check_ptr>=6 && packet_check_ptr<=7) \
				continue; \
			else \
			{ \
				if(pktBuf_check[packet_check_ptr-8] != pktBuf_expect[packet_check_ptr]) \
				{ \
					rtlglue_printf("\t%s(%u):  Compare packet failed.... \n", __func__, __LINE__); \
					rtlglue_printf("=======   pktBuf_expect =========\n"); \
					rtl_testModel_dumpPacket(pktBuf_expect,pktLen_expect); \
					rtlglue_printf("=======   pktBuf_check =========\n"); \
					rtl_testModel_dumpPacket(pktBuf_check,pktLen_check); \
					return RTL_TESTMODEL_TESTCASE_RUN_FAILED; \
				} \
			} \
		} \
		else if(hsa_check->l2tr==1 && hsa_check->pppoeifo==0)\
		{ \
			if(packet_check_ptr<=11) \
			{ \
				if(pktBuf_check[packet_check_ptr] != pktBuf_expect[packet_check_ptr]) \
				{ \
					rtlglue_printf("\t%s(%u):  Compare packet failed.... \n", __func__, __LINE__); \
					rtlglue_printf("=======   pktBuf_expect =========\n"); \
					rtl_testModel_dumpPacket(pktBuf_expect,pktLen_expect); \
					rtlglue_printf("=======   pktBuf_check =========\n"); \
					rtl_testModel_dumpPacket(pktBuf_check,pktLen_check); \
					return RTL_TESTMODEL_TESTCASE_RUN_FAILED; \
				} \
			} \
			else if(packet_check_ptr>=12 && packet_check_ptr<=19) \
				continue; \
			else \
			{ \
				if(pktBuf_check[packet_check_ptr-8] != pktBuf_expect[packet_check_ptr]) \
				{ \
					rtlglue_printf("\t%s(%u):  Compare packet failed.... \n", __func__, __LINE__); \
					rtlglue_printf("=======   pktBuf_expect =========\n"); \
					rtl_testModel_dumpPacket(pktBuf_expect,pktLen_expect); \
					rtlglue_printf("=======   pktBuf_check =========\n"); \
					rtl_testModel_dumpPacket(pktBuf_check,pktLen_check); \
					return RTL_TESTMODEL_TESTCASE_RUN_FAILED; \
				} \
			} \
		} \
		else if(hsa_check->l2tr==1 && hsa_check->pppoeifo)\
		{ \
			if(packet_check_ptr<=13) \
			{ \
				if(pktBuf_check[packet_check_ptr] != pktBuf_expect[packet_check_ptr]) \
				{ \
					rtlglue_printf("\t%s(%u):  Compare packet failed.... \n", __func__, __LINE__); \
					rtlglue_printf("=======   pktBuf_expect =========\n"); \
					rtl_testModel_dumpPacket(pktBuf_expect,pktLen_expect); \
					rtlglue_printf("=======   pktBuf_check =========\n"); \
					rtl_testModel_dumpPacket(pktBuf_check,pktLen_check); \
					return RTL_TESTMODEL_TESTCASE_RUN_FAILED; \
				} \
			} \
			else if(packet_check_ptr>=14 && packet_check_ptr<=21) \
				continue; \
			else \
			{ \
				if(pktBuf_check[packet_check_ptr-8] != pktBuf_expect[packet_check_ptr]) \
				{ \
					rtlglue_printf("\t%s(%u):  Compare packet failed.... \n", __func__, __LINE__); \
					rtlglue_printf("=======   pktBuf_expect =========\n"); \
					rtl_testModel_dumpPacket(pktBuf_expect,pktLen_expect); \
					rtlglue_printf("=======   pktBuf_check =========\n"); \
					rtl_testModel_dumpPacket(pktBuf_check,pktLen_check); \
					return RTL_TESTMODEL_TESTCASE_RUN_FAILED; \
				} \
			} \
		} \
		else \
		{ \
			rtlglue_printf("not support type\n"); \
			return RTL_TESTMODEL_TESTCASE_RUN_FAILED; \
		} \
	} \
} while(0)



struct MODEL_TEST_CASE_S
{	
	int32 no;
	char* name;
	int32 (*fp)(void);
	uint32 group;
};
typedef struct MODEL_TEST_CASE_S MODEL_TEST_CASE_T;


int rtl_testModel_testLayer3MTU( void );

int rtl_testModel_testPktIPMulticastL2( void );
int rtl_testModel_testPktIPMulticastL3( void );
int rtl_testModel_testPktIPMulticastPPPoE( void );


int rtl_testModel_testLayer3Routing( void );
int rtl_testModel_testLayer3GuestVLAN(void);
int32 rtl_testModel_testEnhancedGRE(void);
int32 rtl_testModel_testIpOther(void);
int32 rtl_testModel_testLayer3RoutingToCpu(void);
int32 rtl_testModel_testIPMulticast(void);


void rtl_testModel_run(int testcase_id);


#endif

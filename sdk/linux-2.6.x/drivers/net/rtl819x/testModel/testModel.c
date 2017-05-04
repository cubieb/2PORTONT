/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Header File for Virtual MAC
* Abstract : 
* Author : Louis Yung-Chieh Lo (yjlou@realtek.com.tw)               
* $Id: testModel.c,v 1.3 2012/10/24 04:49:43 ikevin362 Exp $
*/

#include <linux/delay.h>
#include "testModel.h"
#include "virtualMac.h"
#include "../AsicDriver/rtl865xc_asicregs.h"
#include "rtl865xc_testModel_L34_test.h"

#define MODEL_TEST_CASE( case_no, func, group_mask ) \
	{ \
		no: case_no, \
		name: #func, \
		fp: func, \
		group: group_mask, \
	}

/****************************************************************************
 ****__*****__*************                                                ** 
 ***|  |***|  |****__******                                                ** 
 ***|  |***|  |***|  |*****                                                ** 
 ***|  |***|  |***|__|*****                                                ** 
 ***|  |***|  |************      Add Your Model Test Cases HERE !!!        ** 
 ***|   ___   |****__******                                                ** 
 ***|  |***|  |***|  |*****                                                ** 
 ***|  |***|  |***|  |*****                                                ** 
 ***|  |***|  |***|  |*****                                                ** 
 ***|  |***|  |***|  |*****                                                ** 
 ***|__|***|__|***|__|*****                                                **
 ****************************************************************************/
static MODEL_TEST_CASE_T modelTestCase[] =
{	
		
	/* L3 Routing */
	MODEL_TEST_CASE(        60000, rtl_testModel_testLayer3MTU, GRP_ALL | GRP_L34 ),
	MODEL_TEST_CASE( AUTO_CASE_NO, rtl_testModel_testLayer3Routing, GRP_ALL | GRP_L34 ),
	MODEL_TEST_CASE( AUTO_CASE_NO, rtl_testModel_testLayer3GuestVLAN, GRP_ALL | GRP_L34 ),
#ifndef CONFIG_RTL8196B	
	/* 
	 * Alpha's comment: GRE, ALG and ServerPort (include testEnhancedGRE, testServerPort, testPktGRE, 
	 *		testPktServerPort, testALG) do not need to test in 8196C or 8198.
	 * 		testPktPPPoE use server port table, does not test also.
	 */
	//MODEL_TEST_CASE( AUTO_CASE_NO, rtl_testModel_testEnhancedGRE, GRP_ALL | GRP_L34 ),	
	//MODEL_TEST_CASE( AUTO_CASE_NO, rtl_testModel_testIpOther, GRP_ALL | GRP_L34 ),	
#endif	
	MODEL_TEST_CASE( AUTO_CASE_NO, rtl_testModel_testLayer3RoutingToCpu, GRP_ALL | GRP_L34 ),
	/* MODEL_TEST_CASE( AUTO_CASE_NO, testL3RoutingInternal, GRP_ALL | GRP_L34 ), */
	
	/* Multicast */
	MODEL_TEST_CASE( AUTO_CASE_NO, rtl_testModel_testIPMulticast, GRP_ALL|GRP_L34),
	/* MODEL_TEST_CASE( AUTO_CASE_NO, testMulticastWithSVID, GRP_ALL | GRP_L34 ), */




	/*Layer 4*/
	MODEL_TEST_CASE( 		70000	, rtl_testModel_testPktIPMulticastL2, GRP_ALL | GRP_L34 ),
	MODEL_TEST_CASE( AUTO_CASE_NO	, rtl_testModel_testPktIPMulticastL3, GRP_ALL | GRP_L34 ),
	MODEL_TEST_CASE( AUTO_CASE_NO	, rtl_testModel_testPktIPMulticastPPPoE, GRP_ALL | GRP_L34 )
	/* Server Port */
#ifndef CONFIG_RTL8196B	
	//MODEL_TEST_CASE( AUTO_CASE_NO, testServerPort, GRP_ALL | GRP_L34 ),
	//MODEL_TEST_CASE( AUTO_CASE_NO, testPktRemarking, GRP_ALL | GRP_L34 ),	
#endif
	/* MODEL_TEST_CASE( AUTO_CASE_NO, testServerPortTcpOnly, GRP_ALL | GRP_L34 ), */
	/* MODEL_TEST_CASE( AUTO_CASE_NO, testServerPortUdpOnly, GRP_ALL | GRP_L34 ), */
	/* MODEL_TEST_CASE( AUTO_CASE_NO, testServerPortPriority, GRP_ALL | GRP_L34 ), */

#ifndef CONFIG_RTL8196B	
	//MODEL_TEST_CASE( AUTO_CASE_NO, testPktPPPoE, GRP_ALL | GRP_L34 ),	
	//MODEL_TEST_CASE( AUTO_CASE_NO, testPktGRE, GRP_ALL | GRP_L34 ),	
	//MODEL_TEST_CASE( AUTO_CASE_NO, testPktServerPort, GRP_ALL | GRP_L34 ),
	//MODEL_TEST_CASE( AUTO_CASE_NO, testPPTP,GRP_ALL | GRP_L34 ),
#endif
	//MODEL_TEST_CASE( AUTO_CASE_NO, testPktRemarkingRandom, GRP_ALL | GRP_L34 ),	
};


int rtl_testModel_testEnhancedGRE( void )
{
#ifdef CONFIG_RTL_8676HWNAT
	return rtl865xC_testEnhancedGRE();
#else
	#error please implement your platform here
#endif
}


int rtl_testModel_testIpOther( void )
{
#ifdef CONFIG_RTL_8676HWNAT
	return rtl865xC_testIpOther();
#else
	#error please implement your platform here
#endif
}




int rtl_testModel_testLayer3RoutingToCpu( void )
{
#ifdef CONFIG_RTL_8676HWNAT
	return rtl865xC_testLayer3RoutingToCpu();
#else
	#error please implement your platform here
#endif
}
int rtl_testModel_testIPMulticast( void )
{
#ifdef CONFIG_RTL_8676HWNAT
	return rtl865xC_testIPMulticast();
#else
	#error please implement your platform here
#endif
}

int rtl_testModel_testLayer3MTU( void )
{
#ifdef CONFIG_RTL_8676HWNAT
	return rtl865xC_testLayer3MTU();
#else
	#error please implement your platform here
#endif
}


int rtl_testModel_testPktIPMulticastL2( void )
{
#ifdef CONFIG_RTL_8676HWNAT
	return rtl865xC_testPktIPMulticastL2();
#else
	#error please implement your platform here
#endif
}

int rtl_testModel_testPktIPMulticastL3( void )
{
#ifdef CONFIG_RTL_8676HWNAT
	return rtl865xC_testPktIPMulticastL3();
#else
	#error please implement your platform here
#endif
}

int rtl_testModel_testPktIPMulticastPPPoE( void )
{
#ifdef CONFIG_RTL_8676HWNAT
	return rtl865xC_testPktIPMulticastPPPoE();
#else
	#error please implement your platform here
#endif
}

int rtl_testModel_testLayer3Routing( void )
{
#ifdef CONFIG_RTL_8676HWNAT
	return rtl865xC_testLayer3Routing();
#else
	#error please implement your platform here
#endif
}

int rtl_testModel_testLayer3GuestVLAN( void )
{
#ifdef CONFIG_RTL_8676HWNAT
	return rtl865xC_testLayer3GuestVLAN();
#else
	#error please implement your platform here
#endif
}


void rtl_testModel_run(int testcase_id)
{
	int i;
	int32 caseNoSeq = 1; /* to generate auto-increased case number. */
	uint32 groupmask = 0xffffffff;
	int totalCase=0;
	int retval;

	/* generate auto-increased pCase->no */
	for( i = 0; i < sizeof(modelTestCase)/sizeof(modelTestCase[0]); i++ )
	{
		MODEL_TEST_CASE_T *pCase = &modelTestCase[i];
		
		if  ( pCase->no == AUTO_CASE_NO )
			pCase->no = (caseNoSeq++);
		else
			caseNoSeq = pCase->no+1; /* auto learn the next no */
	}


	for( i = 0; i < sizeof(modelTestCase)/sizeof(modelTestCase[0]); i++ )
	{	
		MODEL_TEST_CASE_T *pCase = &modelTestCase[i];	
	
		if ( ( pCase->group & groupmask ) == 0 ) 
			continue;
		if ( pCase->fp == NULL ) 
			continue;
		if( pCase->no != testcase_id )
			continue;

		totalCase++;		
		printk( "Running Model Test Case %d: %s() ...\n", pCase->no, pCase->name );		

		
		/* Prepare virtualMac environment */
		//WRITE_MEM32( SSIR, READ_MEM32(SSIR)|FULL_RST);	
		//mdelay(50);	
		virtualMacInit();		

		retval = pCase->fp();
		
		if (retval == RTL_TESTMODEL_TESTCASE_RUN_SUCCESS )
		{
			printk("success\n");	
		}
		else if (retval == RTL_TESTMODEL_TESTCASE_RUN_INIT_FAILED )
		{
			printk("something wrong in initializing testing enviroment\n");
			break;
		}	
		else if (retval == RTL_TESTMODEL_TESTCASE_RUN_FAILED )
		{
			printk("FAILED!!\n");
			break;
		}
		else
		{
			printk("someting wrong !? \n");
			break;
		}
	}

	if(i==sizeof(modelTestCase)/sizeof(modelTestCase[0]))
		printk("%d test case all pass!\n",totalCase);
}

void rtl_testModel_dumpPacket(char* data,int len)
{
	int i;	
	printk("-- len:%d --\n",len);
	for (i=0; i<len; i++)
	{
		printk("%02X  ",data[i]&0xFF);
		if(i%16==15)
			printk("\n");
		else if(i%8==7)
			printk("  ");
	}
	printk("\n");
}




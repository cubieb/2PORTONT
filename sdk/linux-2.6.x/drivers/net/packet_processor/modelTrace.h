/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Header File for Model Trace Function
* Abstract : 
* Author : Louis Yung-Chieh Lo (yjlou@realtek.com.tw)               
* $Id: modelTrace.h,v 1.1.1.1 2010/05/05 09:00:48 jiunming Exp $
*/

#ifndef _MODEL_TRACE_
#define _MODEL_TRACE_

#include "rtl_types.h"


#define MT_PARSER_MASK        	0x0001
#define MT_TRANSLATOR_MASK	0x0002
#define MT_ALE_MASK			0x0004
#define MT_RINGCTRL_MASK		0x0008

#define MT_PARSER_DEBUG_MASK			0x0010
#define MT_TRANSLATOR_DEBUG_MASK		0x0020
#define MT_ALE_DEBUG_MASK				0x0040
#define MT_RINGCTRL_DEBUG_MASK		0x0080

#define MT_DROP_MASK		0x40000000
#define MT_TOCPU_MASK		0x80000000

//uint32 mt_trace_mask;

/*
 * The MT_DEBUG_* macro set is used to trace the packet processing in model code.
 * You can insert them into your model code.
 *
 *  MT_PARSER() -- Show Parser function name and line number.
 *  MT_TRANSLATOR() -- Show Translaotr function name and line number.
 *  MT_ALE() -- Show Parser function name and line number.
 *  MT_RINGCTRL() -- Show Translaotr function name and line number.

 *  MT_PARSER_DEBUG() -- Show Parser function name and line number.(just for debug)
 *  MT_TRANSLATOR_DEBUG() -- Show Translaotr function name and line number.(just for debug)
 *  MT_ALE_DEBUG() -- Show Parser function name and line number.(just for debug)
 *  MT_RINGCTRL_DEBUG() -- Show Translaotr function name and line number.(just for debug)
 
 *  MT_DROP() -- Show message when ASIC decides to drop the packet.
 *  MT_TOCPU() -- Show message when ASIC decides to trap the packet to CPU.
 */


#if 1
#define MT_PARSER( comment )
#else
#define MT_PARSER( comment ) \
		rtlglue_printf( "\033[32m[PARSER] %s @%s():%d\033[0m\n", comment, __FUNCTION__, __LINE__ );
#endif	


#if 1
#define MT_TRANSLATOR( comment ) 
#else
#define MT_TRANSLATOR( comment ) \
		rtlglue_printf( "\033[31m[TRANSLATOR] %s @%s():%d\033[0m\n", comment, __FUNCTION__, __LINE__ );
#endif	

#if 1
#define MT_ALE( comment ) 
#else
#define MT_ALE( comment ) \
		rtlglue_printf( "\033[33m[ALE] %s @%s():%d\033[0m\n", comment, __FUNCTION__, __LINE__ );	
#endif


#if 1
#define MT_RINGCTRL( comment ) 
#else
#define MT_RINGCTRL( comment ) \
		rtlglue_printf( "\033[34m[RINGCTRL] %s @%s():%d\033[0m\n", comment, __FUNCTION__, __LINE__ );	
#endif


#if 1
#define MT_PARSER_DEBUG( format, arg... ) 
#else
#define MT_PARSER_DEBUG( format, arg... ) \
		rtlglue_printf( "\033[1;32m[PARSER DBG]\033[0m " format, ##arg );
#endif

#if 1
#define MT_TRANSLATOR_DEBUG( format, arg... ) 
#else
#define MT_TRANSLATOR_DEBUG( format, arg... ) \
		rtlglue_printf( "\033[1;31m[TRANSLATOR DBG]\033[0m " format, ##arg );
#endif

#if 1
#define MT_ALE_DEBUG( format, arg... ) 
#else
#define MT_ALE_DEBUG( format, arg... ) \
		rtlglue_printf( "\033[1;33m[ALE DBG]\033[0m " format, ##arg );
#endif


#if 1
#define MT_RINGCTRL_DEBUG( format, arg... ) 
#else
#define MT_RINGCTRL_DEBUG( format, arg... ) \
		rtlglue_printf("\033[1;34m[RINGCTRL DBG]\033[0m" format, ##arg );
#endif


#if 1
#define MT_DROP( comment ) 
#else
#define MT_DROP( comment ) \
		rtlglue_printf( "[TRACE,DROP] %s @%s():%d\n", comment, __FUNCTION__, __LINE__ );
#endif


#if 1
#define MT_TOCPU( comment ) 
#else
#define MT_TOCPU( comment ) \
		rtlglue_printf( "[TRACE,TOCPU] %s @%s():%d\n", comment, __FUNCTION__, __LINE__ );
#endif


#if 1
#define IN_MT_PARSER(x) 
#else
#define IN_MT_PARSER(x) \
		do \
		{ \
		rtlglue_printf("\033[1;32m");  \
		x \
		rtlglue_printf("\033[0m"); \
		} \
		while(0); 
#endif

#if 1
#define IN_MT_TRANSLATOR(x) 
#else
#define IN_MT_TRANSLATOR(x) \
		do \
		{ \
		rtlglue_printf("\033[1;31m"); \
		x \
		rtlglue_printf("\033[0m"); \
		} \
		while(0); 
#endif

#if 1
#define IN_MT_ALE(x) 
#else
#define IN_MT_ALE(x) \
		do \
		{ \
		rtlglue_printf("\033[1;33m"); \
		x \
		rtlglue_printf("\033[0m"); \
		} \
		while(0);
#endif
		

#if 1
#define IN_MT_RINGCTRL(x) 
#else
#define IN_MT_RINGCTRL(x) \
		do \
		{ \
		rtlglue_printf("\033[1;34m"); \
		x \
		rtlglue_printf("\033[0m"); \
		} \
		while(0);		
#endif


#if 1
#define IN_MT_PARSER_DEBUG(x)
#else
#define IN_MT_PARSER_DEBUG(x) \
		do \
		{ \
		rtlglue_printf("\033[32m");  \
		x \
		rtlglue_printf("\033[0m"); \
		} \
		while(0); 
#endif


#if 1
#define IN_MT_TRANSLATOR_DEBUG(x)
#else
#define IN_MT_TRANSLATOR_DEBUG(x) \
		do \
		{ \
		rtlglue_printf("\033[31m");  \
		x \
		rtlglue_printf("\033[0m"); \
		} \
		while(0); 
#endif		


#if 1
#define IN_MT_ALE_DEBUG(x) 
#else
#define IN_MT_ALE_DEBUG(x) \
		do \
		{ \
		rtlglue_printf("\033[33m");  \
		x \
		rtlglue_printf("\033[0m"); \
		} \
		while(0); 
#endif		


#if 1
#define IN_MT_RINGCTRL_DEBUG(x) 
#else
#define IN_MT_RINGCTRL_DEBUG(x) \
		do \
		{ \
		rtlglue_printf("\033[34m");  \
		x \
		rtlglue_printf("\033[0m"); \
		} \
		while(0);	
#endif		


int32 runModelTrace(uint32 userId,  int32 argc,int8 **saved);


#endif

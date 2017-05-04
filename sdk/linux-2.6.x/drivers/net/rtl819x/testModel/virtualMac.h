/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Header File for Virtual MAC
* Abstract : 
* Author : Louis Yung-Chieh Lo (yjlou@realtek.com.tw)               
* $Id: virtualMac.h,v 1.1 2012/10/12 05:42:57 ikevin362 Exp $
*/

#ifndef _VIRTUAL_MAC_
#define _VIRTUAL_MAC_

#include <net/rtl/rtl_types.h>
#include "../AsicDriver/rtl865x_asicL2.h"
#include "../AsicDriver/rtl865xC_hs.h"







int32 virtualMacInit( void );
int32 virtualMacInput( enum PORTID fromPort, uint8* packet, int32 len );
int32 virtualMacOutput( uint32 *toPort, uint8* packet, int32 *len );




int32 virtualMacGetHsb( hsb_param_t* hsb );
int32 virtualMacSetHsb( hsb_param_t* hsb );
int32 virtualMacGetHsa( hsa_param_t* hsa );
int32 virtualMacSetHsa( hsa_param_t* hsa );


int32 convertHsaToAsic( hsa_param_t* hsa, hsa_t* rawHsa );
int32 convertHsaToSoftware( hsa_t* rawHsa, hsa_param_t* hsa );
int32 convertHsbToAsic( hsb_param_t* hsb, hsb_t* rawHsb );
int32 convertHsbToSoftware( hsb_t* rawHsb, hsb_param_t* hsb );



#endif


/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Source File for Virtual MAC
* Abstract : 
* Author : Louis Yung-Chieh Lo (yjlou@realtek.com.tw)               
* $Id: virtualMac.c,v 1.3 2012/10/24 04:50:56 ikevin362 Exp $
*/


#include "virtualMac.h"
#include "modelTrace.h"
#include "rtl865xc_testModel.h"


/*---------------------------------------------------------------------------
 *  virtualMacInit() is used to initial corresponding register for different
 *  configurations (example: MIILIKE, and VSV).
 *-------------------------------------------------------------------------*/
int32 virtualMacInit( void )
{
#ifdef CONFIG_RTL_8676HWNAT
	return rtl865xC_virtualMacInit();
#else
	#error please implement your platform here
#endif
}


/*---------------------------------------------------------------------------
 *  virtualMacInput() is used to input a complete packet, included L2 CRC,
 *  into FPGA/Model Code.
 *
 *
 *    +-----+--+---+----+-----+                            /-- FPGA/IC
 *    |ether|IP|TCP|data|L2CRC| === virtualMacInput() ==> +
 *    +-----+--+---+----+-----+                            \-- Model Code
 *
 *  SrcPort is needed.
 *-------------------------------------------------------------------------*/
int32 virtualMacInput( enum PORTID fromPort, uint8* packet, int32 len )
{
	int32 retval;
	uint8 szComment[128];

	sprintf( szComment, "Input from port %d with %d bytes", fromPort, len );
	MT_WATCH( szComment );

	/* The length must be larger than Ethernet minimum packet size. */
	if(len<64)
		return FAILED;

#ifdef CONFIG_RTL_8676HWNAT
	retval = rtl865xC_virtualMacInput(fromPort, packet, len);
#else
	#error please implement your platform here
#endif

	return retval;
}

/*---------------------------------------------------------------------------
 *  virtualMacOutput() will return a complete packet with a dstPort mask.
 *
 *
 *       FPGA/IC --\                            +-----+--+---+----+
 *                  +=== virtualMacInput() ===> |ether|IP|TCP|data|
 *    Model Code --/                            +-----+--+---+----+
 *
 *  INPUT:
 *    *toPort -- a pointer to the portlist that user candidates to check.
 *               DANGEROUS: If multiple bits are set, only one port will be served. The data of other ports will be DISCARD.
 *                          Therefore, to safely poll every port, please call virtualMacOutput() many times with only one port each time.
 *    *packet -- the buffer to store returned packet content.
 *    *len    -- pointer to store returned packet length.
 *
 *  OUTPUT:
 *    *toPort -- indicate the portlist that packet is exactly forwarded to.
 *    *len    -- packet length is returned in the storage (included L2 CRC)
 *    SUCCESS -- A packet is returned in *packet.
 *    FAILED  -- No packet is returned.
 *--------------------------------------------------------------------------*/
int32 virtualMacOutput( uint32 *toPort, uint8* packet, int32 *len )
{
	int32 retval;
	char szComment[128];

	sprintf( szComment, "Wanna get with portmask 0x%03x and buffer %d bytes", *toPort, *len );
	MT_WATCH( szComment );

	if( !toPort || *toPort==0 )
		return FAILED;

	if(!packet)
		return FAILED;

	if(!len)
		return FAILED;

#ifdef CONFIG_RTL_8676HWNAT
	retval = rtl865xC_virtualMacOutput(toPort, packet, len);
#else
	#error please implement your platform here
#endif
	
	if ( retval==FAILED )
	{
		*len = 0;
		*toPort = 0;
	}
	
	sprintf( szComment, "[%d] Got a packet with portmask 0x%03x and length %d bytes", retval, *toPort, *len );
	MT_WATCH( szComment );
	
	return retval;
}


/*---------------------------------------------------------------------------
 *  +------+  *this  +-------+         +-------+
 *  |  SW  | ------> |convert| ------> |  HSB  |
 *  +------+         +-------+         +-------+
 *-------------------------------------------------------------------------*/
int32 virtualMacSetHsb( hsb_param_t* hsb )
{
	hsb_t rawHsb;
	int32 ret = SUCCESS;
	
	convertHsbToAsic( hsb, &rawHsb );
	
#ifdef CONFIG_RTL_8676HWNAT
	ret = rtl865xC_virtualMacSetHsb(rawHsb);
#else
	#error please implement your platform here
#endif
	
	return ret;
}


/*---------------------------------------------------------------------------
 *  +------+         +-------+  *this  +-------+
 *  |  SW  | ------> |convert| ------> |  HSB  |
 *  +------+         +-------+         +-------+
 *-------------------------------------------------------------------------*/
int32 convertHsbToAsic( hsb_param_t* hsb, hsb_t* rawHsb )
{
#ifdef CONFIG_RTL_8676HWNAT
	return rtl865xC_convertHsbToAsic(hsb, rawHsb);
#else
	#error please implement your platform here
#endif
}



/*---------------------------------------------------------------------------
 *  +-------+  *this  +-------+         +------+
 *  |  HSA  | ------> |convert| ------> |  SW  |
 *  +-------+         +-------+         +------+
 *-------------------------------------------------------------------------*/
int32 convertHsaToSoftware( hsa_t* rawHsa, hsa_param_t* hsa )
{
#ifdef CONFIG_RTL_8676HWNAT
	return rtl865xC_convertHsaToSoftware(rawHsa, hsa);
#else
	#error please implement your platform here
#endif
}


/*---------------------------------------------------------------------------
 *  +-------+         +-------+  *this  +------+
 *  |  HSA  | ------> |convert| ------> |  SW  |
 *  +-------+         +-------+         +------+
 *-------------------------------------------------------------------------*/
int32 virtualMacGetHsa( hsa_param_t* hsa )
{	
	int32 ret = SUCCESS;

#ifdef CONFIG_RTL_8676HWNAT
	ret = rtl865xC_virtualMacGetHsa(hsa);
#else
	#error please implement your platform here
#endif	

	return ret;
}


/*---------------------------------------------------------------------------
 *  +------+         +-------+  *this  +-------+
 *  |  SW  | ------> |convert| ------> |  HSA  |
 *  +------+         +-------+         +-------+
 *-------------------------------------------------------------------------*/
int32 convertHsaToAsic( hsa_param_t* hsa, hsa_t* rawHsa )
{
#ifdef CONFIG_RTL_8676HWNAT
	return rtl865xC_convertHsaToAsic(hsa, rawHsa);
#else
	#error please implement your platform here
#endif
}






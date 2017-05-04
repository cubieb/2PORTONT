/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Header File for IC Export Function
* Abstract : 
* Author : Michael Mao-Lin Huang (michaelhuang@realtek.com.tw)
* $Id: icExport.h,v 1.1.1.1 2010/05/05 09:00:48 jiunming Exp $
*/

#include "rtl_types.h"
//#include "virtualMac.h"

enum EXPORT_REGISTER_FORMAT
{
	EXPORT_RTK_SIM,			/* Export registers for designer to simulate IC operation, said 'memw' commands. */
	EXPORT_RTK_EXPECT,		/* Export registers for designer to compare results, said 'memr' commands. */
	EXPORT_CLESHELL,		/* Export registers to import from CLE Shell command. */
	EXPORT_ICEMON,				/* Export registers to for ICE MON command. */
};

/*===========================================================================*
 *  Common Export Functions                                                  *
 *===========================================================================*/
char* modelExportRaw( char* str );
#define modelExportValue( addr, value ) modelExportValueMask( addr, value, 0xffffffff )
char* modelExportValueMask( uint32 addr, uint32 value, uint32 mask );
char* modelExportPointerRegister( uint32 addr );
#define modelExportRegister( addr ) modelExportRegisterMask( addr, 0xffffffff )
char* modelExportRegisterMask( uint32 addr, uint32 mask );
char* modelExportMemory( uint32 addr, uint32 len );
char* modelExportComment( char* pszComment );
char* modelExportNop( int32 delayCycles );

/*===========================================================================*
 *  IC-Specified Export Functions                                            *
 *===========================================================================*/


/*===========================================================================*
 *   VIRTUAL ADDRESS TO PHYSICAL ADDRESS                                     *
 *===========================================================================*/
uint32 queryModelToPhysical( uint32 virtual );
uint32 queryPhysicalToModel( uint32 physical );
int32 addModelToPhysical( uint32 virtualStart, uint32 virtualEnd, uint32 physicalStart );
int32 delModelToPhysical( uint32 virtualStart );

int32 runModelExport(uint32 userId,  int32 argc,int8 **saved);


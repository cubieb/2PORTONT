/****************************************************************************
*  Program/file: STD-DEF.H
*
*  Copyright (C) by RTX TELECOM A/S, Denmark.
*  These computer program listings and specifications, are the property of
*  RTX TELECOM A/S, Denmark and shall not be reproduced or copied or used in
*  whole or in part without written permission from RTX TELECOM A/S, Denmark.
*
*  Programmer: LHJ
*
*  MODULE: WIN32SIM
*  CONTROLLING DOCUMENT:
*  SYSTEM DEPENDENCIES:
*
*
*  DESCRIPTION: Standard include file
*
*
*
*
*
****************************************************************************/

/****************************************************************************
*                                  PVCS info
*****************************************************************************

$Author: krammer $
$Date: 2012/08/17 08:26:37 $
$Revision: 1.1 $
$Modtime:   28 May 2008 11:41:40  $

*/

#ifndef __STD_DEF_H__
#define __STD_DEF_H__

//#include "../../../../../../linux-2.6.x/drivers/char/sc1445x_ata/arch/cr16/"
typedef signed char       int8;
typedef unsigned char     uint8;
typedef unsigned short    uint16;
typedef signed short      int16;
typedef unsigned long     uint32;
typedef signed long       int32;
typedef unsigned char     boolean;

// The colors
#define CLR_BLUE           RGB( 0x00, 0x00, 0xff )
#define CLR_RED            RGB( 0xff, 0x00, 0x00 )
#define CLR_GREEN          RGB( 0x00, 0xff, 0x00 )
#define CLR_BLACK          RGB( 0x00, 0x00, 0x00 )
#define CLR_YELLOW         RGB( 0xff, 0xff, 0x00 )
#define CLR_CYAN           RGB( 0x00, 0xff, 0xff )
#define CLR_PINK           RGB( 0xff, 0x00, 0xff )
#define CLR_GRAY           RGB( 0x80, 0x80, 0x80 )
#define CLR_WHITE          RGB( 0xff, 0xff, 0xff )
#define CLR_DARKRED        RGB( 0x80, 0x00, 0x00 )
#define CLR_DARKGREEN      RGB( 0x00, 0x80, 0x00 )
#define CLR_DARKBLUE       RGB( 0x00, 0x00, 0x80 )
#define CLR_DARKPINK       RGB( 0x80, 0x00, 0x80 )
#define CLR_DARKGRAY       RGB( 0x50, 0x50, 0x50 )
#define CLR_LIGHTBLUE      RGB( 0x00, 0x80, 0xff )
#define CLR_LIGHTGRAY      RGB( 0xC0, 0xC0, 0xC0 )
#define CLR_LIGHTGREEN     RGB( 0x00, 0xff, 0x80 )
#define CLR_LIGHTRED       RGB( 0xff, 0x00, 0x80 )


#ifndef min
  #define min(x,y) ( (x<y) ? x : y )
  #define max(x,y) ( (x>y) ? x : y )
#endif

#ifndef BIT0
  #define BIT0               0x01
  #define BIT1               0x02
  #define BIT2               0x04
  #define BIT3               0x08
  #define BIT4               0x10
  #define BIT5               0x20
  #define BIT6               0x40
  #define BIT7               0x80

  #define BIT8             0x0100
  #define BIT9             0x0200
  #define BIT10            0x0400
  #define BIT11            0x0800
  #define BIT12            0x1000
  #define BIT13            0x2000
  #define BIT14            0x4000
  #define BIT15            0x8000

  #define BIT16        0x00010000
  #define BIT17        0x00020000
  #define BIT18        0x00040000
  #define BIT19        0x00080000
  #define BIT20        0x00100000
  #define BIT21        0x00200000
  #define BIT22        0x00400000
  #define BIT23        0x00800000

  #define BIT24        0x01000000
  #define BIT25        0x02000000
  #define BIT26        0x04000000
  #define BIT27        0x08000000
  #define BIT28        0x10000000
  #define BIT29        0x20000000
  #define BIT30        0x40000000
  #define BIT31        0x80000000
#endif

/******************************************************************************
* Enumerations.
******************************************************************************/

#define ENUM8(EnumName)  Enum_##EnumName; typedef uint8  EnumName
#define ENUM16(EnumName) Enum_##EnumName; typedef uint16 EnumName
#define ENUM32(EnumName) Enum_##EnumName; typedef uint32 EnumName

#define RSENUM8(EnumName)  Enum_##EnumName; typedef uint8  EnumName
#define RSENUM16(EnumName) Enum_##EnumName; typedef uint16 EnumName
#define RSENUM32(EnumName) Enum_##EnumName; typedef uint32 EnumName

#endif


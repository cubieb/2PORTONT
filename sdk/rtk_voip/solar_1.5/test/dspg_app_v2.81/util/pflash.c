/*
   pflash.c

   P-Flash support routines.
*/
/*!
*  \file       pflash.c
*  \brief      P-Flash support routines
*  \author     stein
*
*  @(#)  %filespec: pflash.c~DMZD53#2 %
*
*******************************************************************************
*  \par  History
*  \n==== History =============================================================\n
*  date        name     version   action                                       \n
*  ----------------------------------------------------------------------------\n
*
*******************************************************************************/

#include "tclib.h"
#include "embedded.h"
#include "csys0reg.h"
#include "csys5os.h"

#include "pflash.h"

#define FMEM_FCTR             0x00200000
#define FMEM_FSTAT            0x00200004
#define FMEM_FPTR             0x00200008
#define FMEM_FBWST            0x00200010
#define FMEM_FCRA             0x0020001C
#define FMEM_FMSSTART         0x00200020
#define FMEM_FMSSTOP          0x00200024
#define FMEM_FMSW0            0x0020002C
#define FMEM_FMSW1            0x00200030
#define FMEM_FMSW2            0x00200034
#define FMEM_FMSW3            0x00200038

#define FCTR_RES	            0x0005
#define FS_DCR                0x0010
#define FS_RY                 0x0004
#define FS_BSYN               0x0001

// P-Flash wait state register
#define FBWST_EN_2LC          0x8000
#define FBWST_SPEC_RD	      0x4000
#define FBWST_WST_MASK        0x00FF
#define FBWST_WST5	         0x0005
#define FBWST_WST4	         0x0004
#define FBWST_WST3	         0x0003
#define FBWST_WST2	         0x0002
#define FBWST_WST1	         0x0001
#define FBWST_WST0	         0x0000
#define FBWST_RES	            0xC003

#define FSTAT_CHECK1          0x00000080
#define FSTAT_CHECK0          0x00000040
#define FSTAT_EC              0x00000020
#define FSTAT_HVONN           0x00000010
#define FSTAT_HVONP           0x00000008
#define FSTAT_RY              0x00000004
#define FSTAT_PROGGNT         0x00000002
#define FSTAT_BSYN            0x00000001

#ifdef __arm
#pragma arm section code "IRAM"
#endif

//#define DELAY_WRITE()   __asm { nop;nop;nop;nop;nop;nop;nop;nop; }
#define DELAY_WRITE(n)  {u32 d=n; while(*(volatile u32 *)(d)--);}

static void       pflWriteRegister( u32 u32_RegAddr, u32 u32_Value );
static
PCPFLASH_TIMING   pflInit( u32 * pu32_Fctr );
static __pure
PCPFLASH_TIMING   pflGetFlashTiming( void );
static void       pflSetSectorProtection( u32 u32_Sector, u32 u32_Count, u32 u32_Protection );
static void       pflRunProgram( u32 u32_Program, u32 u32_Time );
static void       pflStop( u32 u32_Fctr );


static
void              pflWriteRegister( u32 u32_RegAddr, u32 u32_Value )
{
   *(volatile u32 *)u32_RegAddr = u32_Value;
   DELAY_WRITE(3);
}

/*
 * Init PFlash
 */

static
PCPFLASH_TIMING   pflInit( u32 * pu32_Fctr )
{
                                 // save original control register values
   *pu32_Fctr = *(volatile u32 *)FMEM_FCTR;

                                 // clear timer
   pflWriteRegister( FMEM_FPTR, 0 );

   return   pflGetFlashTiming();
}

void              pflErase( u32 u32_Sector, u32 u32_Count )
{
   u32            u32_Fctr;
   u32            irqs = SYS5_DISABLE_ALL;
   PCPFLASH_TIMING
                  pTiming = pflInit( &u32_Fctr );
                  
                                 // switch on CRA clock
   pflWriteRegister( FMEM_FCRA, pTiming->u16_FcraClock );

                                 // turn off sector protection
   pflSetSectorProtection( u32_Sector, u32_Count, 0 );

   while( u32_Count > 1 )        // mark all sectors but the last for erase
   {
                                 // write to sector
      *(u32 *)(u32_Sector << 16) = 0;
                                 // mark for erase
   				                  // FS_LOADREQ | FS_WPB | FS_WEB | FS_CS
      pflWriteRegister( FMEM_FCTR, 0x8085 );

      u32_Sector++;
      u32_Count--;
   }
   				                  // FS_WEB | FS_CS
   pflWriteRegister( FMEM_FCTR, 0x0005 );

                                 // write to last sector
   *(u32 *)(u32_Sector << 16) = 0;
                                 // start erasing
                                 // FS_PROGREQ | FS_WPB | FS_CS
   pflRunProgram( 0x1081, pTiming->u32_EraseTimer );

   pflStop( u32_Fctr );

   SYS5_RESTORE_ALL( irqs );
}


/*
 * Program a pflash page of 512 byte
 */
void              pflWrite( u32 * pu32_Destination, u32 * pu32_Source, u32 u32_Count )
{
   u32            irqs = SYS5_DISABLE_ALL;
   u32            *pu32_PageStart, *pu32_PageEnd;
   u32            u32_Fctr;

   PCPFLASH_TIMING
                  pTiming = pflInit( &u32_Fctr );

                                 // switch on CRA clock
   pflWriteRegister( FMEM_FCRA, pTiming->u16_FcraClock );
   
                                 // start programming
   while( u32_Count > 0 )        // max. 128 times per page
   {
                                 // reset page data input latches
                                 // FS_PLD | FS_WEB | FS_WRE | FS_CS
      pflWriteRegister( FMEM_FCTR, 0x407 );

                                 // enable data load
                                 // FS_WEB |FS_WRE | FS_CS
      pflWriteRegister( FMEM_FCTR, 0x07 );

      // Calculate page start/end address of destination page
      pu32_PageStart = (u32 *) ((u32)pu32_Destination & ~PFLASH_PAGE_MASK);
      pu32_PageEnd   = pu32_PageStart + PFLASH_PAGE_SIZE / 4;

      // write buffer to latch until page end or buffer is empty
      // e.g page(0) : PageEnd = 0x200
      while( (pu32_Destination < pu32_PageEnd) && (u32_Count > 0) )
      {
                                 // write data to page latches
         *pu32_Destination++ = *pu32_Source++;
         u32_Count--;
      }
                                 // fill remaining PFLASH words in page with 0xFFFFFFFF
      while( pu32_Destination < pu32_PageEnd )
         *pu32_Destination++ = 0xFFFFFFFF;

                                 // start page programming
                                 // FS_PROGREQ | FS_WPB | FS_WRE | FS_CS
      pflRunProgram( 0x1083, pTiming->u16_ProgTimer );
   }

   pflStop( u32_Fctr );

   SYS5_RESTORE_ALL(irqs);
}


void              pflUnprotect( u32 u32_Sector, u32 u32_Count )
{
   u32            u32_Fctr;
   PCPFLASH_TIMING
                  pTiming = pflInit( &u32_Fctr );

                                 // switch on CRA clock
   pflWriteRegister( FMEM_FCRA, pTiming->u16_FcraClock );

                                 // turn off sector protection
   pflSetSectorProtection( u32_Sector, u32_Count, 0 );
   pflStop( u32_Fctr );
}

void              pflProtect( u32 u32_Sector, u32 u32_Count )
{
   u32            u32_Fctr;
   PCPFLASH_TIMING
                  pTiming = pflInit( &u32_Fctr );

                                 // switch on CRA clock
   pflWriteRegister( FMEM_FCRA, pTiming->u16_FcraClock );

                                 // turn on sector protection
   pflSetSectorProtection( u32_Sector, u32_Count, 1 );
   pflStop( u32_Fctr );
}

/*
 * programming/erasing time table, depending on AHB_PLL_DIV value
 * just for 124.416 MHz clk_arm at the moment...
 */
static const
PFLASH_TIMING     PFlashTimingTable[] =
{                                   // t_erase = 100 ms, t_write = 5 ms
   {  0x105451, 621, 1215, 24300 }, // clk_arm:124.416 MHz hclk: 41.472 MHz
   {  0x105411, 621, 1215, 24300 }  // clk_arm:124.416 MHz hclk:124.416 MHz
};

/*
   This function returns the program/erase timings required
   for the current ahb_pll_div.
*/
static __pure
PCPFLASH_TIMING   pflGetFlashTiming( void )
{
   u8             i;
   
   for( i = 0; i < sizeof(PFlashTimingTable); i++ )
   {
      if( PFlashTimingTable[i].u32_AhbPllDiv == sys0_SCU.ahb_pll_div )
      {
         return &PFlashTimingTable[i];
      }
   }
   
   return NULL;
}

static void       pflSetSectorProtection( u32 u32_Sector, u32 u32_Count, u32 u32_Protection )
{
   while( u32_Count-- )
   {
                                 // write to sector, 0 unprotects
      *(u32 *)(u32_Sector << 16) = u32_Protection;
   				                  // FS_LUPREQ | FS_WPB | FS_WEB | FS_WRE | FS_CS
      pflWriteRegister( FMEM_FCTR, 0x8087 );

      u32_Sector++;
   }
}

static void       pflRunProgram( u32 u32_Program, u32 u32_Time )
{
                                 // bit 15 enables timer
   pflWriteRegister( FMEM_FPTR, 0x8000 | u32_Time );

                                 // start programming/erasing
   pflWriteRegister( FMEM_FCTR, u32_Program );

                                 // wait until timer expires
//   while( (*(volatile u32 *)FMEM_FPTR & 0x7FFF) == u32_Time )
//      ;
                                 // wait until programming/erasing is complete

//   while( (*(volatile u32 *)FMEM_FSTAT & 0x1F) != 0x05 )
//      ;

   while( !(*(volatile u32 *)FMEM_FSTAT & FSTAT_RY) );
   while( !(*(volatile u32 *)FMEM_FSTAT & FSTAT_BSYN) );
}

void              pflStop( u32 u32_Fctr )
{
                                 // clear timer
   pflWriteRegister( FMEM_FPTR, 0 );

                                 // reset control register values
   pflWriteRegister( FMEM_FCTR, u32_Fctr );

                                 // switch off CRA clock
   pflWriteRegister( FMEM_FCRA, 0 );
}

#ifdef __arm
#pragma arm section code
#endif

//*/

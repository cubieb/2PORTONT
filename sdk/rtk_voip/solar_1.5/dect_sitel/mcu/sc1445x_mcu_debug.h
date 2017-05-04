/*
 *-----------------------------------------------------------------------------
 *                                                                             
 *               @@@@@@@      *   @@@@@@@@@           *                                     
 *              @       @             @               *                            
 *              @             *       @      ****     *                                 
 *               @@@@@@@      *       @     *    *    *                              
 *        ___---        @     *       @     ******    *                                 
 *  ___---      @       @     *       @     *         *                             
 *   -_          @@@@@@@  _   *       @      ****     *                               
 *     -_                 _ -                                                     
 *       -_          _ -                                                       
 *         -_   _ -        s   e   m   i   c   o   n   d   u   c   t   o   r 
 *           -                                                                    
 *                                                                              
 * (C) Copyright SiTel Semiconductor BV, unpublished work.
 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:		sc1445x_mcu_debug.h
 * Purpose:		
 * Created:		Aug 2008
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef sc1445x_MCU_DEBUG_H
#define sc1445x_MCU_DEBUG_H
 
   
#define DEBUG						1
#define DEBUG_INFO			1
#define MCU_NO_OUTPUT		1

#  if defined( MCU_NO_OUTPUT )

#    define PRINT( ... )	do { } while( 0 )
#    define DPRINT( ... )	do { } while( 0 )
#    define PRINTINFO( ... )	do { } while( 0 )

#  else	

#    define PRINT( ... )	printf( __VA_ARGS__ )
#    if defined( DEBUG )
#      define DPRINT( ... )	printf( "debug: " __VA_ARGS__ )
#    else
#      define DPRINT( ... )	do { } while( 0 )
#    endif
#
#    if defined( DEBUG_INFO )
#		define PRINTINFO( ... )	printf( "MCUinfo: " __VA_ARGS__ )
#    else
#		define PRINTINFO( ... )	do { } while( 0 )
#    endif

#  endif  /* AUDIO_ENGINE_NO_OUTPUT */

#endif //sc1445x_amb_API_H

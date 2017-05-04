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
 * File:		si_dect.h
 * Purpose:		
 * Created:	9/2/2009
 * By:			EF
 * Country:	Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_DECT_H
#define SI_DECT_H
 
/*========================== Include files ==================================*/
#include "../common/operation_mode_defs.h"

/*========================== Local macro definitions & typedefs =============*/


/*========================== Global variables ===============================*/

#ifdef DECT_ENABLED
  #define sc1445x_dect_terminal_outgoing_req(attachedentity, dialnumber, portid, accountid, codec)\
  _sc1445x_dect_terminal_outgoing_req(attachedentity, dialnumber, portid, accountid, codec)
  #define sc1445x_dect_terminal_audio_peripheral_change(attachedentity, port, accountid, hookstatus, codec, appliedtocall) \
  _sc1445x_dect_terminal_audio_peripheral_change(attachedentity, port, accountid, hookstatus, codec, appliedtocall)
	#define sc1445x_dect_terminal_get_key_seq(attachedentity, key_seq, portid, accountid, codec)\
		_sc1445x_dect_terminal_get_key_seq(attachedentity, key_seq, portid, accountid, codec)\

#else
	#ifdef TEXT_USER_INTERFACE
 		#include "../TUI/scr_main_state_machine.h"
	#elif defined TFT_USER_INTERFACE
		#include "../TFT_GUI/scr_main_state_machine.h"
	#else 
		#include "../GUI/scr_main_state_machine.h"
	#endif

	#define sc_fileapi_LoadStarCodes(star_code_table)
  #define sc1445x_dect_terminal_outgoing_req(attachedentity, dialnumber, portid, accountid, codec)
  #define sc1445x_dect_terminal_audio_peripheral_change(attachedentity, port, accountid, hookstatus, codec, appliedtocall) \
    sc1445x_gui_dect_audio_peripheral_change(attachedentity, port, accountid, hookstatus, codec) 
	#define sc1445x_dect_terminal_get_key_seq(attachedentity, key_seq, portid, accountid, codec)


#endif



/*========================== Global function prototypes =====================*/


#endif /* SI_DECT */

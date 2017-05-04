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
 * File:		  si_dect_callcontrol.h
 * Purpose:		
 * Created:		26/01/2009
 * By:			  EF
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_DECT_CALLCONTROL_H
#define SI_DECT_CALLCONTROL_H
 
/*========================== Include files ==================================*/
#include <stdlib.h>  
#include <stdio.h>
#include <string.h>
#include "../CCFSM/ccfsm_api.h"
#include "../CCFSM/sc1445x_phone_api.h"

/*========================== Local macro definitions & typedefs =============*/


/*========================== Global variables ===============================*/
typedef enum{
  DECT_NARROWBAND_CODEC,
  DECT_WIDEBAND_CODEC
}ap_dect_codec_type;

typedef enum{
  CODEC_SUPPORTED,
  CODEC_NOT_SUPPORTED
}ap_dect_codec_supported;

/*========================== Global function prototypes =====================*/


void _sc1445x_dect_terminal_outgoing_req(ccfsm_attached_entity attachedentity, unsigned char * dialnumber, int portid, int accountid, ccfsm_codec_type codec);
sc1445x_phone_call_info *sc1445x_dect_terminal_get_next_call(sc1445x_phone_call_info *pCall, int portid, int accountid);
void sc1445x_dect_terminal_terminate_allcalls(ccfsm_attached_entity attachedentity, int portid, int accountid);

void sc1445x_dect_terminal_ring(ring_mode mode, int port, int accountid, int codec, int numofcalls, char* displayname);
void _sc1445x_dect_terminal_audio_peripheral_change(ccfsm_attached_entity attachedentity, int port, int accountid, ccfsm_audio_peripheral_status hookstatus, int codec, unsigned char appliedtocall); 
void sc1445x_dect_terminal_start_ring(int codec, int port, int accountid, char*displayname);
void sc1445x_dect_terminal_stop_ring(int port, int accountid);
void sc1445x_dect_terminate_call(int port, int accountid, int numofcalls, int term_reason);
int sc1445x_dect_set_pcm_line_type(int line, int codec);
void sc1445x_dect_change_configuration_codec_selection(char* codec ,int portid);
int sc1445x_dect_change_rfpi(unsigned char *MACAddress);

#endif /* SI_DECT_CALLCONTROL_H */


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
 * File:		sc1445x_media.h
 * Purpose:		
 * Created:		Dec 2007
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */
#ifndef sc1445x_MEDIA_H
#define sc1445x_MEDIA_H
#include <ortp/ortp.h>

 
	#include <alsa/asoundlib.h>
 
 #include "sc1445x_mcu_block.h"

#define PH_MEDIA_PCMU_PAYLOAD		0
#define PH_MEDIA_G72632_PAYLOAD		2
#define PH_MEDIA_PCMA_PAYLOAD		8
#define PH_MEDIA_G722_PAYLOAD		9 
#define PH_MEDIA_CN_PAYLOAD			13
#define PH_MEDIA_G729_PAYLOAD		18
#define PH_MEDIA_H263_PAYLOAD		34
#define PH_MEDIA_H264_PAYLOAD		40
#define PH_MEDIA_H263FLV1_PAYLOAD	96
#define PH_MEDIA_H263P_PAYLOAD		97
#define PH_MEDIA_ILBC_PAYLOAD		98
#define	PH_MEDIA_ILBC_PAYLOAD_20ms	99
#define PH_MEDIA_MPEG4_PAYLOAD		111
#define PH_MEDIA_DTMF_PAYLOAD		101
#define PH_MEDIA_SPEEXNB_PAYLOAD	105
#define PH_MEDIA_G726WB_PAYLOAD		102
#define PH_MEDIA_SPEEXWB_PAYLOAD	103
#define PH_MEDIA_AMR_PAYLOAD		123
#define PH_MEDIA_AMR_WB_PAYLOAD		124
#define PH_MEDIA_CN_16000_PAYLOAD	125 

#define PH_MEDIA_CN_PT_STR "13"

struct SC1445x_MEDIA_payload_s
{
  int  number;       /* payload code for rtp packets*/
  char string[32];   /* mime string (ex: ILBC/8000/1) */ 
  int  rate;         /* sampling rate */
  int  chans;        /* number of channels */
  int  ptime;        /* media duration in one packet in msecs */
  int  psize;        /* (optional) size of payload in one packet */
  int  mode;         /* (optional) codec specific mode parameter */
} ;
  
int sc1445x_media_init(void );
void sc1445x_media_vprofile_init(void);
int sc1445x_media_get_ptime(int payload); 	 
unsigned short sc1445x_media_find_codec_type(int payload, int vad) ;	 
int sc1445x_media_find_samplesize(int payload) ;

PayloadType * sc1445x_media_get_telephone_event( void );
RtpProfile * sc1445x_media_get_av_profile( void );

#endif //sc1445x_MEDIA_H

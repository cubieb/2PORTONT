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
 * File:		scr_phbook.h
 * Purpose:		
 * Created:		Oct 2008
 * By:			KF
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SC1445x_T38LAYER_H
#define SC1445x_T38LAYER_H

typedef struct __t38Layer_t
{
	int rxsocket;
	int txsocket;
	int rport;
	int lport;
	struct sockaddr_in rAddress;
}t38Layer_t;
 
#define  MAX_T38_PACKET_SIZE 1024

int sc1445x_t38_stream_open(unsigned char *rAddress, unsigned int lport, unsigned int rport, unsigned short alsaChannel);
void sc1445x_t38_stream_close(void);
int sc1445x_t38RecvData(unsigned char *t38DataBuffer);
int sc1445x_t38SendData (char *t38DataBuffer, int t38DataSize );

 
#endif  //SC1445x_T38LAYER_H

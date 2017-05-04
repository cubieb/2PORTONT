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
 * (C) Copyright SiTel Semiconductor BV, unpublished work.file: 

 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:		 		 sc1445x_t38layer.c
 * Purpose:		 		 
 * Created:		 		 Feb 2009
 * By:		 		 	 YP
 * Country:		 		 Greece
 *
 *-----------------------------------------------------------------------------
 */
/*========================== Include files ==================================*/
#include "sc1445x_mcu_api_defs.h"
#ifdef SC1445x_AE_SUPPORT_FAX 

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <values.h>     
 
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h> /* FIOSNBIO*/
#include <fcntl.h>     
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "sc1445x_mcu_debug.h"
#include "../sc1445x_src/SiATA/si_ata.h"
#include "sc1445x_t38layer.h"

t38Layer_t t38Layer;
int T38Running =0;

int sc1445x_t38_stream_open(unsigned char *rAddress, unsigned int lport, unsigned int rport, unsigned short alsaChannel)
{
	struct sockaddr_in ServerAddr;
  	int val;
 	int ret=0;

	t38Layer.lport = lport;
	t38Layer.rport = rport;
  	 
	t38Layer.rxsocket  = socket(AF_INET, SOCK_DGRAM, 0);
	if(t38Layer.rxsocket < 0)
	{
		DPRINT( "[%s]Couldn't create T38 stream  \n", __FUNCTION__);	 
  		return -1;
	}
 
	//ioctl(t38Layer.rxsocket,FIOSNBIO,&arg);
	val = fcntl(t38Layer.rxsocket, F_GETFL, 0);
	fcntl(t38Layer.rxsocket, F_SETFL, val | O_NONBLOCK);

 	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(lport);
	ServerAddr.sin_addr.s_addr = INADDR_ANY;
	ret = bind(t38Layer.rxsocket, (struct sockaddr *)&ServerAddr, sizeof(ServerAddr));
	if(ret < 0)
	{
		DPRINT( "[%s]Couldn't bind T38 socket  \n", __FUNCTION__);	 
 		close(t38Layer.rxsocket);
		return -1;
	}
 
	t38Layer.txsocket = socket(AF_INET, SOCK_DGRAM, 0);
	t38Layer.rAddress.sin_family = AF_INET;
	t38Layer.rAddress.sin_port =  htons((unsigned short) rport) ;
  	t38Layer.rAddress.sin_addr.s_addr =  inet_addr(rAddress);//);

  	//START T. 38 Entity 
	if (!T38Running) //TBC
	{
	  ata_t38_sdp_params_type sdp_params;
	
	  //T38Running =1;
	  ata_get_t38_params( &sdp_params );
	  sdp_params.T38LinkMode = t38UDP;
 	  ata_set_t38_params( alsaChannel, &sdp_params );
 	}
	return 0;
}
void sc1445x_t38_stream_close(void)
{
	close(t38Layer.txsocket);
	close(t38Layer.rxsocket);
}
int sc1445x_t38RecvData(unsigned char *t38DataBuffer)
{
	unsigned int ClientAddrLen ;
	struct sockaddr_in ClientAddr;
	
	if (t38Layer.rxsocket<=0) return 0;

	ClientAddrLen=sizeof(ClientAddr);
	return recvfrom(t38Layer.rxsocket, (unsigned char*)t38DataBuffer, MAX_T38_PACKET_SIZE, 0, (struct sockaddr *)&ClientAddr, &ClientAddrLen);
}
int sc1445x_t38SendData (char *t38DataBuffer, int t38DataSize )
{
	  int ret;

 	  if (t38Layer.txsocket ==NULL || t38DataBuffer==NULL  || t38DataSize<1) {
		DPRINT( "[%s]Invalid T.38 parameter  \n", __FUNCTION__);	 
 	   return -1;
	  }
 	  
	 ret = sendto(t38Layer.txsocket, t38DataBuffer, t38DataSize, 0, (struct sockaddr *)&t38Layer.rAddress , sizeof(t38Layer.rAddress ));
	 if(ret < 0){
		DPRINT( "[%s]Failed to transmit T38 packet  \n", __FUNCTION__);	 
  	 }
	 return ret;
}
 
#endif

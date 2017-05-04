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
 * File:		 		 si_ua_stun.c
 * Purpose:		 		 
 * Created:		 		 Dec 2007
 * By:		 		 	 YP
 * Country:		 		 Greece
 *
 *-----------------------------------------------------------------------------
 */
/*========================== Include files ==================================*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <si_print_api.h>
 
#include "si_ua_stun.h"
#include "../exosip2/src/eXosip2.h"
#include <ortp/stun.h>

char *ipv4tostr(char *buf, const StunAddress4 addr);

#define PORT_STRING_LEN 16
static char NatRouterAddr[32];
 
int isPublicAddress(char *Ip);
int isPublicAddress(char *Ip)
{
	return eXosip_is_public_address(Ip);
	
}
extern unsigned char SystemMACAddress[]; 
int stun_get_nattype(SICORE* pCore)
{
 	char *ntstr = "sym";
  	NatType ntype;
	bool_t resPort = 0, hairpin = 0;
	int needMappedAddress = 0;
	StunAddress4 stunServerAddr;

 	NatRouterAddr[0] = 0;
    stunServerAddr.port =  pCore->StunPort;
 	if (!stunParseServerName(pCore->StunAddress, &stunServerAddr))  
	{
		si_print(PRINT_LEVEL_ERR, "Unable to parse stun server name \n\n");
		return -1;
	}
{
	int min=0x4000;
	int max=0x7FFF;
	int ret, ret1;
	sleep(1);

	ret1 = stunGetSystemTimeSecs();
	srand( *((int*)&SystemMACAddress[0])+ret1);
  	ret = random();
	 
	ret = ret|min;
	ret = ret&max;
  

 	ntype = stunNatType( &stunServerAddr,  0, &resPort, &hairpin, ret, 0);//stun debug level
	si_print(PRINT_LEVEL_INFO, "Stun Nat Type : %d Hairpinning =%d PreServedPort=%d \n\n", ntype, hairpin,resPort);
}
	switch (ntype)
	{
	case StunTypeOpen:
		//si_print(PRINT_LEVEL_DEBUG, "StunTypeOpen \n ");	
		ntstr = 0;
		break;
	case StunTypeConeNat:
		//si_print(PRINT_LEVEL_DEBUG, "StunTypeConeNat \n ");	
		ntstr = "fcone";
		needMappedAddress = 1;
		break;

	case StunTypeRestrictedNat:
		//si_print(PRINT_LEVEL_DEBUG, "StunTypeRestrictedNat \n");	
		ntstr = "rcone";
		needMappedAddress = 1;
		break;

	case StunTypePortRestrictedNat:
		//si_print(PRINT_LEVEL_DEBUG, "StunTypePortRestrictedNat \n");	
		ntstr = "prcone";
		needMappedAddress = 1;
		break;
 	case StunTypeSymFirewall:
	case StunTypeSymNat:
		//si_print(PRINT_LEVEL_DEBUG, "StunTypeSymNat \n ");	
		needMappedAddress = 1;
		break;
 	case StunTypeFailure:
		;//si_print(PRINT_LEVEL_DEBUG, "default1");	
	case StunTypeUnknown:
		;//si_print(PRINT_LEVEL_DEBUG, "default2");	
	case StunTypeBlocked:
		;//si_print(PRINT_LEVEL_DEBUG, "default3");	
	default:
		;//si_print(PRINT_LEVEL_DEBUG, "default4");	
		break;
	}
 
	if (needMappedAddress)
	{
		StunAddress4 mappedAddr;
		Socket  sock;

		 
		sock = stunOpenSocket(&stunServerAddr, &mappedAddr,/*pCore->SipServerPort */0, NULL, 0);
		if (-1 !=  (int) sock)
		{
		//	ipv4tostr(pCore->firewallIP, mappedAddr);
			//snprintf(NatSipPortStr, sizeof(NatSipPortStr), "%d", mappedAddr.port);
 			pCore->SipServerPort= mappedAddr.port;
 			closesocket(sock);
		}
	}
  	return 0;
}

int select_audio_port(SICORE* pCore, sicall_t* pCall, char *LocalAudioBuffer, char *PublicAudioBuffer)
{
   int ret=0;
   //YPAPA 
   // a semaphore to be added to protect the following port selection procedure
   get_local_audio_sdp_port(pCore, LocalAudioBuffer);
 	 strcpy(PublicAudioBuffer, LocalAudioBuffer);
   if (isPublicAddress((char *)pCore->IPAddress ))
	   ret =  getPublicPort(pCore, LocalAudioBuffer, PublicAudioBuffer);       

   if (emptystr(PublicAudioBuffer) || invalidport((int)atoi(PublicAudioBuffer))){ //SIPIT 25
  		strcpy(PublicAudioBuffer, LocalAudioBuffer);
   }
  
   pCall->local_sdp_audio_port =(int) atoi(LocalAudioBuffer);
   pCall->public_sdp_audio_port =(int) atoi(PublicAudioBuffer);
  
   return ret;
}
int isPortInuse(int port)
{
	//YPAPA
	//TO BE DEFINED
	return 0;
}
void get_local_audio_sdp_port(SICORE* pCore, char *buf)
{
// To get arround the problem of connect in winsock ( connect returns
//	* an error: 10048: Address already in use connect is called an the local address is in used
//	* within the last 2 or 4 minutes), we should not bind the rtp socket to a specific local port. Hence,
//	* we should return "0" here to let the system choose a random port number.
 	//base port number
	int port = pCore->localAudioRtpPort ;
 	sprintf(buf, "%d", port);
	pCore->localAudioRtpPort +=2;
	return;
 	 
	buf[0] = 0;
	while(1)
	{
		if (!isPortInuse(port))
		{
			sprintf(buf, "%d", port);
			return;
		}
		else
		{
			port += 2;    // try next pair
		}
	}
}

int getPublicPort(SICORE* pCore, char * local_voice_port, char * public_voice_port )
{
 	Socket  sock;
	Socket  sock1;
 	StunAddress4 mappedAddr;
	StunAddress4 mappedAddr1;
	StunAddress4 stunServerAddr;
	int res;
 
	int loop = 0;
 	if (pCore->StunAddress=='\0') {
		return -1;
	}

	stunParseServerName(pCore->StunAddress, &stunServerAddr);
 	do {
 		res = stunOpenSocketPair(&stunServerAddr, &mappedAddr, &mappedAddr1,&sock, &sock1,atoi(local_voice_port), NULL, 1);
 		if (-1 !=  res)
		{
 			 snprintf(public_voice_port, 9, "%d", mappedAddr.port);
 			closesocket(sock1);
			closesocket(sock); 
 		}
		else
		{
			closesocket(sock);
			closesocket(sock1);			
 			snprintf(local_voice_port, 9, "%i",atoi(local_voice_port)+2);
			loop++;
		}
	} while (res <0 && loop < 4 );

	if (res <0)
	{
		return -1;
	}
  	//si_print(PRINT_LEVEL_DEBUG, "get public port public voice_port: %s  ------ \n \n ",public_voice_port );
	return 1;
}


int stun_get_localip(SICORE* pCore, char *result, int *port)
{
	const char *server=pCore->StunAddress;
	StunAddress4 addr;
	StunAddress4 mapped;
	StunAddress4 changed;
  
	if (server!=NULL){
		if (stunParseServerName((char*)server,&addr)){
  			if (stunTest(&addr,1,TRUE,NULL,&mapped,&changed)==0)
			{
				struct in_addr inaddr;
				char *tmp;
				inaddr.s_addr=ntohl(mapped.addr);
				tmp=inet_ntoa(inaddr);
				strncpy(result,tmp,SIPHONE_IPADDR_SIZE);
				*port = mapped.port;
		 		return 1;
			}else{
				si_print(PRINT_LEVEL_ERR, "stun lookup failed. \n");
			}
		}else{
			si_print(PRINT_LEVEL_ERR, "Fail to resolv or parse %s \n",server);
		}
	}
	return 0;
}

char *ipv4tostr(char *buf, const StunAddress4 addr)
{
      unsigned int  ip = (unsigned int)addr.addr;

      sprintf(buf, "%d.%d.%d.%d", (unsigned int)((ip >> 24) & 255),  (unsigned int)((ip >> 16) & 255), (unsigned int)((ip >> 8) & 255), (unsigned int)(ip & 255));

      return buf;
}


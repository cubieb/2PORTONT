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
 * File:		si_ua_client_api.c
 * Purpose:				 		 
 * Created:		06/12/2007
 * By:		 	KF
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

/*========================== Include files ==================================*/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include <si_print_api.h>

#include "../socketclient/si_ua_client_api.h"
#include "../mmi_sm/si_mmi_phonesm.h"
#include "../call_sm/si_mmi_call_sm.h"


/*========================== Local macro definitions ========================*/


/*========================== Global definitions =============================*/

  
/*========================== Local function prototypes ======================*/


/*========================== Local data definitions =========================*/
int ServerAPIisON = 0;
int client_sock;
int server_sock;
char CallBackRcvMsg[512];

/*========================== Function definitions ===========================*/
int si_api_ua_client_init()
{
	int i;

	pthread_t thread;

	if(client_init() == -1)
		return -1;

	if(server_init() == -1)
		return -1;

	i = pthread_create (&thread, NULL, receive_loop, (void *) NULL);
	pthread_detach(thread) ;

	if (i != 0)
	{
		si_print(PRINT_LEVEL_INFO, "API_pthread_create failed\n"); 
  	return -1;
	}

	if(ConnectToUAServer() == -1)
		return -1;
	
	UA_AskRegStatus();
	
	return 0;
}

int client_init(void)
{
	client_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(client_sock < 0)
	{
		si_print(PRINT_LEVEL_ERR, "Client socket not created\n");
		return -1;
	}

	return 0;
}

int server_init(void)
{
	struct sockaddr_in ServerAddr;
	unsigned short ServerPort = CONSOLE_SERVER_PORT;
	int ret;

	server_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(server_sock < 0)
	{
		si_print(PRINT_LEVEL_ERR, "Server socket not created\n");
		return -1;
	}

	memset(&ServerAddr, 0, sizeof(ServerAddr));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(ServerPort);
 	ServerAddr.sin_addr.s_addr = INADDR_ANY;

	ret = bind(server_sock, (struct sockaddr *)&ServerAddr, sizeof(ServerAddr));
	if(ret < 0)
	{
		si_print(PRINT_LEVEL_ERR, "Server socket bind failed\n");
		close(server_sock);
		return -1;
	}

	return 0;
}

int receive_loop(void * arg)
{
	CallBackParams *pCallBack=0;
	struct sockaddr_in ClientAddr;
	unsigned int ClientAddrLen;
	int ret;

	ServerAPIisON = 1;
	while(ServerAPIisON) {
		ClientAddrLen = sizeof(ClientAddr);
	   ret = recvfrom(server_sock, CallBackRcvMsg, sizeof(CallBackRcvMsg), 0, (struct sockaddr *)&ClientAddr, &ClientAddrLen);
	   if(ret < 0) {
			si_print(PRINT_LEVEL_ERR, "FATAL ERROR on data reception\n");
			continue;
		}else{
			pCallBack=(CallBackParams *)CallBackRcvMsg ;
			si_print(PRINT_LEVEL_INFO, " CallBACK - [%d]: info [%s] vline[%x] \n\n",(int)pCallBack->callbackID, pCallBack->info,pCallBack->vline);// ((pCallBack->vline)&0x00000014));
			
			ParsePacket(pCallBack);
			memset(CallBackRcvMsg, 0, sizeof(CallBackRcvMsg));
		}
	}

	close(server_sock);

	return 0;
}

int ConnectToUAServer(void)
{
	ConnectPacket m_connect_packet;
	int ret;

	memset(&m_connect_packet , 0, sizeof(m_connect_packet));
	m_connect_packet.actionID = CONNECT_ACTION;
	strcpy(m_connect_packet.ClientName, "MMI");
	m_connect_packet.PortToSendAnswer = CONSOLE_SERVER_PORT;

	ret = SendCmd((char *)&m_connect_packet, sizeof(m_connect_packet));
	if(ret == -1)
		return -1;

	return 0;
}

int SendCmd(char *buffer, int buf_len)
{
	struct sockaddr_in ServerAddr;
	unsigned short ServerPort = UA_SERVER_PORT;
	int ret;

	memset(&ServerAddr, 0, sizeof(ServerAddr));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(ServerPort);
 	ServerAddr.sin_addr.s_addr = inet_addr(UA_SERVER_IP_ADDRESS);

	ret = sendto(client_sock, buffer, buf_len, 0, (struct sockaddr *)&ServerAddr, sizeof(ServerAddr));
	if(ret < 0)
	{
		si_print(PRINT_LEVEL_ERR, "Failed to send command\n");
		return -1;
	}
	return 0;
}

void ParsePacket(CallBackParams  *pCallBack)
{

	switch((int)pCallBack->callbackID)
		{
			case SI_CALL_BACK_REGISTRATIONENDED:
				UA_PerformAction(SI_CALL_BACK_REGISTRATIONENDED, pCallBack->vline, pCallBack->info, pCallBack->reason);				
			break;
			case SI_CALL_ACTION_REGISTRATION:
				UA_PerformAction(SI_CALL_ACTION_REGISTRATION, pCallBack->vline, pCallBack->info, pCallBack->reason);				
			break;
			case SI_CALL_BACK_NEWCALL:
				UA_PerformAction(SI_CALL_BACK_NEWCALL, pCallBack->vline, pCallBack->info, pCallBack->reason);				
			break;
			case SI_CALL_BACK_PROCEEDING:
				UA_PerformAction(SI_CALL_BACK_PROCEEDING, pCallBack->vline, pCallBack->info, pCallBack->reason);				
			break;
			case SI_CALL_BACK_RINGING:
				UA_PerformAction(SI_CALL_BACK_RINGING, pCallBack->vline, pCallBack->info, pCallBack->reason);				
			break;
			case SI_CALL_BACK_EARLYMEDIA:
				UA_PerformAction(SI_CALL_BACK_ANSWERED, pCallBack->vline, pCallBack->info, pCallBack->reason);				
			break;
			case SI_CALL_BACK_ANSWERED:
				UA_PerformAction(SI_CALL_BACK_ANSWERED, pCallBack->vline, pCallBack->info, pCallBack->reason);				
			break;
			case SI_CALL_BACK_HELD:
				UA_PerformAction(SI_CALL_BACK_HELD, pCallBack->vline, pCallBack->info, pCallBack->reason);				
			break;
			case SI_CALL_BACK_REMOTEHELD:
				UA_PerformAction(SI_CALL_BACK_REMOTEHELD, pCallBack->vline, pCallBack->info, pCallBack->reason);				
			break;
			case SI_CALL_BACK_INACTIVE:
			break;
			case SI_CALL_BACK_RESUMED:
				UA_PerformAction(SI_CALL_BACK_RESUMED,pCallBack->vline, pCallBack->info, pCallBack->reason);				
			break;
			case SI_CALL_BACK_RELEASED:
				UA_PerformAction(SI_CALL_BACK_RELEASED,pCallBack->vline, pCallBack->info, pCallBack->reason);				
			break;
			case SI_SYSTEM_GETSTATUS:
				//UA_ReadRegStatus(pCallBack->info);			
			break;
			case SI_CALL_BACK_TERMINATED:
			break;
			default:
			break;
		}
	//memset(CallBackRcvMsg, 0, sizeof(CallBackRcvMsg));
}

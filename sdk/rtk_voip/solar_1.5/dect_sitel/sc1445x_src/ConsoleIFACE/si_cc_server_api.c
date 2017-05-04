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
 * File:			si_cc_server_api.c
 * Purpose:				 		 
 * Created:		20/10/2008
 * By:		 	  EF
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

/*========================== Include files ==================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <net/if.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/ioctl.h> 
#include <arpa/inet.h> 
#include <stdbool.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>

#include <si_print_api.h>

#include "../CCFSM/sc1445x_phone_api.h"
#include "../common/operation_mode_defs.h"

#ifdef GUI_ENABLED
	#ifdef TEXT_USER_INTERFACE
 		#include "../TUI/scr_main_state_machine.h"
	#elif defined TFT_USER_INTERFACE
		#include "../TFT_GUI/scr_main_state_machine.h"
	#else 
		#include "../GUI/scr_main_state_machine.h"
	#endif
#endif

#include "si_cc_server_api.h"

/*========================== Local macro definitions ========================*/
 #define CC_SERVER_PORT          17180

/*========================== Global definitions =============================*/
int cc_si_api_server_init(void);
int receive_loop(void * arg);
int cc_WriteClientInfoToTable(ccfsm_req_type *msg, struct sockaddr_in *ClientAddr);
int cc_EraseClientInfoToTable(ccfsm_req_type *msg);
int cc_GetClientId(struct sockaddr_in *ClientAddr);
int cc_SendCallBack(char *buffer, int buf_len);
int cc_client_init(int clientID);
int cc_receive_loop(void * arg);

/*========================== Local function prototypes ======================*/


/*========================== Local data definitions =========================*/
volatile int CC_ServerAPIisON = 0;
int server_sock;
struct sockaddr_in ClientAddr;
unsigned int ClientAddrLen ;
int client_sock[MAX_NUM_OF_CLIENTS];
ClientInfo m_ClientInfo[MAX_NUM_OF_CLIENTS];

/*========================== Function definitions ===========================*/

int si_cc_api_process(void)
{
  pthread_t thread;
  int i;

  cc_si_api_server_init();
  i = pthread_create (&thread, NULL, cc_receive_loop, (void *) NULL);
 	pthread_detach( thread ) ;

  if (i != 0)
  {
	  si_print(PRINT_LEVEL_ERR, "API_pthread_create failed\n"); 
  	return -1;
  }

 	return 0;
}

int cc_si_api_server_init(void)
{
	struct sockaddr_in ServerAddr;
 
	int ret;

	server_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(server_sock < 0)
	{
		si_print(PRINT_LEVEL_ERR, "Socket not created\n");
		return -1;
	}
 
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(CC_SERVER_PORT);
 	ServerAddr.sin_addr.s_addr = INADDR_ANY;
  
	ret = bind(server_sock, (struct sockaddr *)&ServerAddr, sizeof(ServerAddr));
	if(ret < 0)
	{
		si_print(PRINT_LEVEL_ERR, "Socket bind failed\n");
		close(server_sock);
		return -1;
	}

 	return 0;
}
 void myprint(char*text, int size);
void CC_RECEIVE_Exit(int sig)
{
	//edo
	CC_ServerAPIisON=0;
 	exit(0); 
}
int cc_receive_loop(void * arg)
{
	ccfsm_req_type mRequest;
 	struct timeval tv;
	int clientID;
 	fd_set readSock;
 	int ret=0;

	signal(SIGINT, CC_RECEIVE_Exit);
	signal(SIGTERM, CC_RECEIVE_Exit);
	signal(SIGABRT, CC_RECEIVE_Exit);  

	ClientAddrLen=sizeof(ClientAddr);
    CC_ServerAPIisON = 1; 
 
	FD_ZERO(&readSock);
 	tv.tv_sec = 0;  
	tv.tv_usec = 1000000;
  
	FD_SET(server_sock, &readSock);

	while(CC_ServerAPIisON) 
	{
		ret = select(server_sock+1, &readSock ,NULL, NULL, &tv);
		ret = recvfrom(server_sock, (unsigned char*)&mRequest, sizeof(ccfsm_req_type), 0, (struct sockaddr *)&ClientAddr, &ClientAddrLen);
 		if(ret < 0) continue;
       	if (mRequest.ccfsm_connect_iface_req.req_id  ==  CCFSM_CONNECT_REQ) {
					 
 			ret = cc_WriteClientInfoToTable(&mRequest, &ClientAddr);
		}else if (mRequest.ccfsm_disconnect_iface_req.req_id ==  CCFSM_DISCONNECT_REQ) {
  			ret = cc_EraseClientInfoToTable(&mRequest);
  		}else{
			clientID = cc_GetClientId(&ClientAddr);
			if(clientID == -1) {
				si_print(PRINT_LEVEL_ERR, "Application client not connected. Please first connect the client \n");
				continue;
			}
			parse_iface_request(&mRequest);
			memset(&mRequest, 0, sizeof(&mRequest));
		}
    }   		
 
 	close(server_sock);
 	return 0;
}
void myprint(char*text, int size)
{
	int k;
	for (k=0;k<size;k++)
	{
		printf("%d ", text[k]);
		if (!(k%16)) printf("\n\n");
	}
}
 
void sc1445x_CONSOLE_RETRANSMIT(void* data, int size)
{
 	cc_SendCallBack((char*)data,size); 
}
// fix 14 April 2009
void parse_iface_request(ccfsm_req_type *pReq)
{
	int reqId = pReq->req_id; 
	sc1445x_phone_call_info *pCall;
  	switch(reqId)
	{
		case  CCFSM_CONNECT_REQ:
 		case  CCFSM_DISCONNECT_REQ:
			break;
    case  CCFSM_FLASH_IND:
	 	case  CCFSM_KEY_PRESS_IND:
	 	case  CCFSM_STAR_CODE_REQ:
 	 	case  CCFSM_DECT_CODEC_IND:
	 	case  CCFSM_ALLOCATE_CALL_REQ:
	 	case  CCFSM_DEALLOCATE_CALL_REQ:
		case  CCFSM_CALL_FORWARD_REQ:
		case  CCFSM_CONFERENCE_REQ:
 		case  CCFSM_SWITCH_TO_LINE_REQ:
  	case  CCFSM_VOLUMEUP_REQ:
 		case  CCFSM_VOLUMEDOWN_REQ:
			si_print(PRINT_LEVEL_INFO, "request not supported \n");
		break;
		case  CCFSM_SELECT_AUDIO_PATH_REQ:
			if (pReq->ccfsm_select_audio_path_req.path==CCFSM_AUD_HANDSFREE_ID)
 				sc1445x_gui_audio_peripheral_change((ccfsm_audio_peripheral_input)pReq->ccfsm_select_audio_path_req.path, 0, 2);	
			else 
				sc1445x_gui_audio_peripheral_change((ccfsm_audio_peripheral_input)pReq->ccfsm_select_audio_path_req.path, 0, 1);	
			break;
 		case  CCFSM_HOOKON_REQ:
			sc1445x_gui_audio_peripheral_change(CCFSM_AUD_HANDSET_ID, 0, 0);	
			break;
		case  CCFSM_HOOKOFF_REQ:
			sc1445x_gui_audio_peripheral_change(CCFSM_AUD_HANDSET_ID, 0, 1);	
			break;
		case  CCFSM_OUTGOING_CALL_REQ:
			si_print(PRINT_LEVEL_DEBUG, "CCFSM_OUTGOING_CALL_REQ = [%s]\n",(unsigned char*)pReq->ccfsm_outgoing_call_req.dialled_num);
 			sc1445x_phoneapi_create_newcall(pReq->ccfsm_outgoing_call_req.attachedentity,
			(unsigned char*)pReq->ccfsm_outgoing_call_req.dialled_num, 
			pReq->ccfsm_outgoing_call_req.portid,
			pReq->ccfsm_outgoing_call_req.accountid, pReq->ccfsm_outgoing_call_req.codec,0);	  
 			break;
		case  CCFSM_ANSWER_REQ:
			sc1445x_phoneapi_call_answer(pReq->ccfsm_call_answer_req.callid,CCFSM_ATTACHED_CONSOLE, 0, CCFSM_CONFIG_CODEC, 0);
			break;
		case  CCFSM_BLINDTRANSFER_REQ:
			sc1445x_phoneapi_blindtransfer(pReq->ccfsm_blindtransfer_req.callid, (char *)pReq->ccfsm_blindtransfer_req.number);
			break;
		case  CCFSM_ATTENDEDTRANSFER_REQ:
			sc1445x_phoneapi_attendedtransfer(pReq->ccfsm_attendedtransfer_req.calltotransfer, pReq->ccfsm_attendedtransfer_req.callid);  
 			break;
		case  CCFSM_CALL_HOLD_REQ:
			si_print(PRINT_LEVEL_DEBUG, "CCFSM_CALL_HOLD_REQ = %d \n",pReq->ccfsm_call_hold_req.callid );
			if (pReq->ccfsm_call_hold_req.callid==-1)
			{
				pCall=sc1445x_phoneapi_find_call(0);
				si_print(PRINT_LEVEL_DEBUG, "CCFSM_CALL_HOLD_REQ callid= %d \n",pCall->callid );
				if (pCall) 
					sc1445x_phoneapi_call_hold(pCall->callid);
			}else 
				sc1445x_phoneapi_call_hold(pReq->ccfsm_call_hold_req.callid);

   			break;
		case  CCFSM_CALL_RESUME_REQ:
			if (pReq->ccfsm_call_resume_req.callid==-1)
			{
				pCall=sc1445x_phoneapi_find_call(0);
				if (pCall) 
					sc1445x_phoneapi_call_resume(pCall->callid);
			}else 
				sc1445x_phoneapi_call_resume(pReq->ccfsm_call_resume_req.callid);
  			break;
		case  CCFSM_TERMINATE_REQ:
			sc1445x_phoneapi_call_terminate(pReq->ccfsm_call_terminate_req.callid,CCFSM_ATTACHED_CONSOLE);
			break;
		case CCFSM_IM_SEND_REQ:
	 		sc1445x_phoneapi_send_im(pReq->ccfsm_send_im_req.attachedentity, (unsigned char *) pReq->ccfsm_send_im_req.dialled_num, pReq->ccfsm_send_im_req.text,pReq->ccfsm_send_im_req.portid ,pReq->ccfsm_send_im_req.accountid );  
 
	}	  
}

int cc_EraseClientInfoToTable(ccfsm_req_type *msg)
{
  int i=0, ret=0;

  for(i = 0 ; i < MAX_NUM_OF_CLIENTS ; i++) {

    if(!strcmp(m_ClientInfo[i].ClientName, msg->ccfsm_connect_iface_req.ClientName)) {             

      if(m_ClientInfo[i].ConnectStatus == 1) {

        if (!(strcmp(msg->ccfsm_connect_iface_req.ClientName, "phone"))){ 
          //ret=init_keypad();
          if (ret==0){
            si_print(PRINT_LEVEL_INFO, "Keypad driver successfully opened \n");
          }else si_print(PRINT_LEVEL_ERR, "Keypad driver could not be opened \n");
  
        }else if (!(strcmp(msg->ccfsm_connect_iface_req .ClientName, "web"))){
          ;
        }

				si_print(PRINT_LEVEL_INFO, "%s application was disconnected !\n", msg->ccfsm_connect_iface_req.ClientName);
        m_ClientInfo[i].ClientName[0]='\0';
		    m_ClientInfo[i].PortToSendAnswer = 0;
		    m_ClientInfo[i].ClientPort = 0;
		    m_ClientInfo[i].ClientIPAddress = 0;
  		  m_ClientInfo[i].ConnectStatus = 0;
        return 0;

      }else si_print(PRINT_LEVEL_INFO, "Application is not currently connected \n");
    }
  }

	si_print(PRINT_LEVEL_ERR, "Application requested to disconneted was not found \n");

  return -1;
}

int cc_WriteClientInfoToTable(ccfsm_req_type *msg, struct sockaddr_in *ClientAddr)
{
	int i, ret=0;

	for(i = 0 ; i < MAX_NUM_OF_CLIENTS ; i++) {
		if(m_ClientInfo[i].ConnectStatus == 1) {
       si_print(PRINT_LEVEL_INFO, "reconnect name is %s\n",m_ClientInfo[i].ClientName);
			if(!strcmp(m_ClientInfo[i].ClientName, msg->ccfsm_connect_iface_req.ClientName )) {
        si_print(PRINT_LEVEL_INFO, "%s application already connected ! Reconnecting... \n",m_ClientInfo[i].ClientName);
				m_ClientInfo[i].ConnectStatus=0;
 				break;
			}
		}
	}

	for(i = 0 ; i < MAX_NUM_OF_CLIENTS ; i++) {

		if(m_ClientInfo[i].ConnectStatus == 0) {

      if (!(strcmp(msg->ccfsm_connect_iface_req.ClientName, "phone"))){                     
					//ret=term_keypad_driver();
          if (ret==0){
						si_print(PRINT_LEVEL_INFO, "Keypad driver was terminated \n");
            si_print(PRINT_LEVEL_INFO, "Passing control to the console application \n");
          }else si_print(PRINT_LEVEL_ERR, "Keypad driver could not be terminated \n");
        
       }else if (!(strcmp(msg->ccfsm_connect_iface_req.ClientName , "web"))){
        ;

      }else {

        si_print(PRINT_LEVEL_ERR, "Connection request from undetermined application \n");
				si_print(PRINT_LEVEL_ERR, "Connection was not established \n");
        return 0;
      }

      si_print(PRINT_LEVEL_INFO, "%s application was connected !\n", msg->ccfsm_connect_iface_req.ClientName);
      strcpy(m_ClientInfo[i].ClientName, msg->ccfsm_connect_iface_req.ClientName);
		  m_ClientInfo[i].PortToSendAnswer = msg->ccfsm_connect_iface_req.PortToSendAnswer;
		  m_ClientInfo[i].ClientPort = ntohs(ClientAddr->sin_port);
		  m_ClientInfo[i].ClientIPAddress = ntohl(ClientAddr->sin_addr.s_addr);
		  m_ClientInfo[i].ConnectStatus = 1;
      return 0;

     }
 			//if(cc_client_init(i) == -1) return -1;
	}

	si_print(PRINT_LEVEL_ERR, "Cannot connect client. Maximum number of connected clients already reached\n");

	return -1;
}


int cc_client_init(int clientID)
{
	client_sock[clientID] = socket(AF_INET, SOCK_DGRAM, 0);
	if(client_sock[clientID] < 0)
	{
		si_print(PRINT_LEVEL_ERR, "Client socket %d not created\n", clientID);
		return -1;
	}

	return 0;
}


int cc_GetClientId(struct sockaddr_in *ClientAddr)
{
	int i;
 	for(i = 0 ; i < MAX_NUM_OF_CLIENTS ; i++) {
		if(m_ClientInfo[i].ConnectStatus == 1) {
	   	if((m_ClientInfo[i].ClientPort == ntohs(ClientAddr->sin_port)) && (m_ClientInfo[i].ClientIPAddress == ntohl(ClientAddr->sin_addr.s_addr))) 
				return i;
		}
	}
 	return -1;
}

 
int cc_SendCallBack(char *buffer, int buf_len)
{
	struct sockaddr_in remoteAddr;
	int remote_sock = socket(AF_INET, SOCK_DGRAM, 0);
	int ret, i;
  
 	if ((buffer==NULL) || (buf_len<1))	return -1;

     for (i=0;i<MAX_NUM_OF_CLIENTS;i++)
	{
 		if (m_ClientInfo[i].ConnectStatus==1)
		{
			   
  		remoteAddr.sin_family = AF_INET;
			remoteAddr.sin_port =  htons(m_ClientInfo[i].PortToSendAnswer) ;
			remoteAddr.sin_addr.s_addr =  htonl(m_ClientInfo[i].ClientIPAddress);
 
 			ret = sendto(remote_sock, buffer, buf_len, 0, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr));
 			if(ret < 0)
			{
				si_print(PRINT_LEVEL_ERR, "Failed to send packet to client %d \n", ret);
				// return -1;
			} else si_print(PRINT_LEVEL_INFO, "Send a reply [%d][%d] ....\n", m_ClientInfo[i].PortToSendAnswer,m_ClientInfo[i].ClientIPAddress);
		}
	}
 
	return 0;
}



#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
//#include "apmib.h"
#include "cwmpevt.h"
#include "cwmp_main_tr104.h"
#include "mib_tr104.h"

#define CWMP_CLINET_PATH "/var/cwmpclient.chanl"
#define SOLAR_CHANNEL_PATH "/var/solar.chanl"
static int ipcSocket = 0;
cwmpEvtMsg pEvtMsg;
int gVoipReq = 0;
extern void cwmpDiagnosticDone();

/*init the solar listener*/
int cwmp_solarInit(void){
	struct sockaddr_un ipcAddr;

    fprintf(stderr,"<%s:%d>INFO: Prepare to init maserati listener.\n", __FUNCTION__, __LINE__);
	ipcSocket = socket(PF_LOCAL, SOCK_DGRAM, 0);
	if(0 > ipcSocket){
		perror("Error:open IPC socket fail");
		return 0;
	}
	unlink(CWMP_CLINET_PATH);
	bzero(&ipcAddr, sizeof(ipcAddr));
	ipcAddr.sun_family = PF_LOCAL;
	strncpy(ipcAddr.sun_path, CWMP_CLINET_PATH, sizeof(CWMP_CLINET_PATH));
	if(bind(ipcSocket, (struct sockaddr*)&ipcAddr, sizeof(ipcAddr)) == -1){
		close(ipcSocket);
		perror("Error:bind IPC socket fail");
		return 0;
	}

	return 1;
}

/*create the solar listener thread*/
void *cwmp_solarListener(void *data){
	if(cwmp_solarInit()){
		fprintf(stderr,"<%s:%d>INFO: Start maserati event listener thread.\n", __FUNCTION__, __LINE__);
		while(1){
			unsigned char *buf = NULL;
			if((buf = (unsigned char*)malloc(cwmpEvtMsgSizeof())) != NULL){
				cwmpEvtMsg *evtMsg;
				if (recvfrom(ipcSocket, (void*)buf, cwmpEvtMsgSizeof(), 0, NULL, NULL) > 1){
					evtMsg = (cwmpEvtMsg*) buf;
                    switch(evtMsg->event){
                       	case EVT_VOICEPROFILE_LINE_GET_STATUS:
                            {
                                memcpy((void*)&pEvtMsg, (void*)evtMsg,  cwmpEvtMsgSizeof());
                                fprintf(stderr,"<%s:%d>INFO: Received GET Event\n", __FUNCTION__, __LINE__);
                            }
                            break;
                        case EVT_VOICEPROFILE_LINE_SET_STATUS:
                            fprintf(stderr,"<%s:%d>INFO: Received Wrong SET Event\n", __FUNCTION__, __LINE__);
                            break;
                        case EVT_VOICEPROFILE_LINE_SET_DIAGNOSTICS:
                            {
                                //Notitfy ACS Server 
                                cwmpDiagnosticDone();
                                fprintf(stderr,"<%s:%d>INFO: Received Diagnostics SET Event\n", __FUNCTION__, __LINE__);
                            }
                            break;
                        default:
                            fprintf(stderr,"<%s:%d>Error: Ignore Unknown Type %d\n", __FUNCTION__, __LINE__, evtMsg->event);
                            continue;
                            break;
                    }
				}
				free(buf);
			}
		}
	}
	return NULL;
}


/*open the connection from solar to cwmpclient*/
void cwmp_solarOpen( void )
{
	pthread_t cwmp_solar_pid;
	
	if( pthread_create(&cwmp_solar_pid, NULL, cwmp_solarListener, NULL) != 0)
		fprintf(stderr,"<%s:%d>Error:initial solar listener fail.\n", __FUNCTION__, __LINE__);

#if 0 /* Added by Alex, 20111027, if support fifo 'X' commands*/
    /* Init flash server variables */
   	if ( voip_flash_server_init_variables()== -1)
    {
		fprintf(stderr,"<%s:%d>Warning:voip_flash_server_init_variables fail.\n", __FUNCTION__, __LINE__);
    }
#endif    
}

/*close the connection from solar to cwmpclient*/
void cwmp_solarClose(void){
	close(ipcSocket);
}

/* send the request to solar */
void cwmpSendRequestToSolar(void){
	cwmpEvtMsg *sendEvtMsg=NULL;
	int sendSock=0;
	struct sockaddr_un addr;

	if((sendEvtMsg = cwmpEvtMsgNew()) != NULL){
		sendEvtMsg->event = EVT_VOICEPROFILE_LINE_GET_STATUS;
		sendSock=socket(PF_LOCAL, SOCK_DGRAM, 0);
		bzero(&addr, sizeof(addr));
		addr.sun_family = PF_LOCAL;
		strncpy(addr.sun_path, SOLAR_CHANNEL_PATH, sizeof(SOLAR_CHANNEL_PATH));
		sendto(sendSock, (void*)sendEvtMsg, cwmpEvtMsgSizeof(), 0, (struct sockaddr*)&addr, sizeof(addr));
		close(sendSock);
		free(sendEvtMsg);
	}
    fprintf( stderr, "<%s:%d>INFO: Event (Type %d) has send to maserati.\n", __FUNCTION__, __LINE__, sendEvtMsg->event);
}

/* send the event to solar */
//void cwmpSendEventToSolar(int evtID, void *data){
static void * cwmpSendEventToSolar(void *arg){
	cwmpEvtMsg *sendEvtMsg=(cwmpEvtMsg *) arg;
	int sendSock=0;
	struct sockaddr_un addr;

	if(sendEvtMsg != NULL){
		sendSock=socket(PF_LOCAL, SOCK_DGRAM, 0);
		bzero(&addr, sizeof(addr));
		addr.sun_family = PF_LOCAL;
		strncpy(addr.sun_path, SOLAR_CHANNEL_PATH, sizeof(SOLAR_CHANNEL_PATH));
		sendto(sendSock, (void*)sendEvtMsg, cwmpEvtMsgSizeof(), 0, (struct sockaddr*)&addr, sizeof(addr));
		close(sendSock);
	} else {
        fprintf( stderr, "<%s:%d>Error: Event Content is NULL !\n", __FUNCTION__, __LINE__);
    }
    fprintf(stderr, "<%s:%d>INFO: Event (Type %d) has send to maserati.\n", __FUNCTION__, __LINE__, sendEvtMsg->event);
    return NULL;
}

void cwmpStartVoipDiag(void) {
	pthread_t voip_pid;

	/* clean config get from mib */
    cwmpDoRefresh();

	if(gVoipReq == EVT_VOICEPROFILE_LINE_SET_STATUS){
		if(&pEvtMsg == NULL){
            fprintf( stderr, "<%s:%d>Error: Event Content is Empty !\n", __FUNCTION__, __LINE__);
			return;
	    }

		if( pthread_create( &voip_pid, NULL, cwmpSendEventToSolar, (void *)&pEvtMsg) != 0 )
		{
			fprintf( stderr, "<%s:%d>Error: Event Thread Create Failed !\n", __FUNCTION__, __LINE__);
			return;
		}
        fprintf( stderr, "<%s:%d>INFO: Start voip diag thread.\n", __FUNCTION__, __LINE__);
		pthread_detach(voip_pid);
    }

    /* clean request type */
    fprintf( stderr, "<%s:%d>INFO: Clear Request Type.\n", __FUNCTION__, __LINE__);
    gVoipReq = EVT_VOICEPROFILE_LINE_NONE;	
}

void cwmpDoRefresh(void){
    mib_config_clean();
}

extern int getVoipFlash(void);
/* main() init/exit functions */
void tr104_main_init( void )
{
	/* voip flash client initial */
	getVoipFlash();
	//if(voip_flash_client_init(&g_pVoIPShare, VOIP_FLASH_WRITE_CLIENT_TR104) == -1){
	//	fprintf( stderr, "<%s>initial flash fail!\n",__FILE__ );
	//	return;
	////}
	
	/* open IPC to communicate solar */
	cwmp_solarOpen();
}

void tr104_main_exit( void )
{
	/* close IPC to communicate solar */
	cwmp_solarClose();
	
	/* close the flash voip client */
	//voip_flash_client_close();
}


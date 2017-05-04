#include <time.h>
#include <stdio.h>
//#include <conio.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <string.h>
//vm
//#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#include "std-def.h"
#include "MailLog.h"
#include "ISip450Api.h"
#include "natalie_app.h"
int devdectfd;
int nvsfd;
#define DECTDEV "/dev/dect"
#define NVSFILE "/mnt/flash/nvs.bin"
uint8 mailbuf[API_FP_MAX_MAIL_SIZE];	
	
/*Function prototypes*/
static void log_mail(const char *prefix_string, void *mailbuf, int maillen);
static void send_dect_mail(void *mailptr, int len);
//void dect_nvs_init(void);
void dect_dev_init();
//void dect_demo_mails_init(void);
static void handle_dect_mail(void);
/*Defined in cvmcon.c*/
#define CvmApiMailType Sip450ApiMailType	
extern void mail_switch(uint16 Length, uint8 *MailPtr);
extern void HandleResetInd(CvmApiMailType *MailPtr);
/****************************************************************************
Interprocess communication functions
****************************************************************************/
void rsx_SendMail(uint32 iTaskId, uint32 iLength, uint8 *bDataPtr)
{
  send_dect_mail(bDataPtr, iLength);
}

void rsx_SendMailP0(uint32 iTaskId, PrimitiveType Primitive)
{
  send_dect_mail((uint8*) &Primitive, sizeof(PrimitiveType));
}

void rsx_SendMailP1(uint32 iTaskId, PrimitiveType Primitive, uint8 bParm1)
{
   recSendMailP1Type tmp;
   tmp.PrimitiveIdentifier = Primitive;
   tmp.bParm1 = bParm1;

   send_dect_mail((uint8*) &tmp, sizeof(recSendMailP1Type));   
}

void rsx_SendMailP2(uint32 iTaskId, PrimitiveType Primitive, uint8 bParm1, uint8 bParm2)
{
   recSendMailP2Type tmp;
   tmp.PrimitiveIdentifier = Primitive;
   tmp.bParm1 = bParm1;
   tmp.bParm2 = bParm2;

   send_dect_mail((uint8*) &tmp, sizeof(recSendMailP2Type));    
}

void rsx_SendMailP3(uint32 iTaskId, PrimitiveType Primitive, uint8 bParm1, uint8 bParm2, uint8 bParm3)
{
   recSendMailP3Type tmp;
   tmp.PrimitiveIdentifier = Primitive;
   tmp.bParm1 = bParm1;
   tmp.bParm2 = bParm2;
   tmp.bParm3 = bParm3;

   send_dect_mail((uint8*) &tmp, sizeof(recSendMailP3Type));    
}

/****************************************************************************
Logging functions
****************************************************************************/
// Log mail in mailbuf with prefix string 
static void log_mail(const char *prefix_string, void *mailbuf, int maillen)
{
//    int i;
//    printf("%s: %4.4hx", prefix_string, *(PrimitiveType*)mailbuf);
//    for (i = 2; i < maillen && i < LOG_MAIL_MAX; i++)
//    {
//       printf(" %2.2hhx", ((uint8*)mailbuf)[i]);
//    }
//    if (i < maillen)
//    {
//       printf(" ...");
      
//    }
//    printf("\n");

   PRINT_MESSAGE("%s: %s\n", prefix_string, MailLog(mailbuf, maillen));
}


/****************************************************************************
Communication with driver functions
****************************************************************************/
static void send_dect_mail(void *mailptr, int len)
{
   log_mail("TX", mailptr, len);
   if (-1 == write(devdectfd, mailptr, len))
   {
      perror("write to " DECTDEV " failed");
      exit(-1);
   }
}
// Open the NVS file, if it does not exist create it an initialize to all
// ff's. Then send it to the dect device driver.
void dect_nvs_init(void)
{
   ApiFpInitReqType InitReq = {.Primitive = API_FP_INIT_REQ};
   
   nvsfd = open(NVSFILE, O_RDONLY);//O_RDWR);
   if (-1 == nvsfd)
   {      
	  PRINT_INFO("Create empty NVS file\n");
      nvsfd = open(NVSFILE, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
      if (-1 == nvsfd)
      {
//          nvsfd = open(RONVSFILE, O_RDWR);
//          if (-1 != nvsfd)
//          {
//             printf("Using READ-ONLY NVS file\n");
//             goto Label_ReadNvs;
//          }
         perror("Failed to open/create NVS file!!!");
         return;
      }
      memset(InitReq.NvsData, 0xff, sizeof(InitReq.NvsData));
      write(nvsfd, InitReq.NvsData, sizeof(InitReq.NvsData));
   }
   else
   {
      //      Label_ReadNvs:
      read(nvsfd, InitReq.NvsData, sizeof(InitReq.NvsData));
   }
   close(NVSFILE);nvsfd=-1;  
   send_dect_mail(&InitReq, sizeof(InitReq));
}
void dect_dev_init()
{
   devdectfd = open(DECTDEV, O_RDWR);
   if (devdectfd == -1)
   {
      perror("failed to open /dev/dect");
      exit(-1);
   }
   else
	DEBUG_PRINT(" %s opened for rdwr \n", DECTDEV);
}
// void dect_demo_mails_init(void)
// {
   // uint8 i;
   ////Allows us to restart dect_demo_app without requiring handsets to locate
   // for (i = 0; i < NO_HS; i++)
   // {
      // CallState[i] = HS_OUT_OF_RANGE;
   // }
// }

static void handle_dect_mail(void)
{
   int len;
   len = read(devdectfd, mailbuf, sizeof(mailbuf));  
   if (-1 == len)
   {
      perror("read from " DECTDEV " failed");
      exit(-1);      
   }

   //assert(len >= 2);

   log_mail("RX", mailbuf, len);
   
   switch (*(PrimitiveType*)mailbuf)
   {
      case API_FP_NVS_UPDATE_IND:
         #define M ((ApiFpNvsUpdateIndType*)mailbuf)
     	 nvsfd = open(NVSFILE, O_RDWR);
         pwrite(nvsfd, M->NvsData, M->Length, M->NvsOffset);
         close(NVSFILE);nvsfd=-1;
         M->Primitive = API_FP_NVS_UPDATE_RES;
         send_dect_mail(M, sizeof(ApiFpNvsUpdateResType));
         #undef M
		 PRINT_MESSAGE("Received mail API_FP_NVS_UPDATE_IND \n");
         break;
      case API_FP_INIT_CFM:
         // #define M ((ApiFpInitCfmType*)mailbuf)
         // memcpy(RxAdpcmBuffersAddr, M->AdpcmRxBufferAddr, sizeof(RxAdpcmBuffersAddr));
         // memcpy(TxAdpcmBuffersAddr, M->AdpcmTxBufferAddr, sizeof(TxAdpcmBuffersAddr));
         // #undef M
		 PRINT_MESSAGE("Received mail API_FP_INIT_CFM \n");
		 HandleResetInd(mailbuf);		 
         break;
      default:
		DEBUG_PRINT("Received mail  \n");
         //dect_demo_mails((Sip450ApiMailType*)mailbuf, len);
		 mail_switch(len, mailbuf);
         break;
   }
}
#if 0
static void event_loop()
{
   while (1)
   {
      fd_set rfds;
      FD_ZERO(&rfds);
      //FD_SET(STDIN_FILENO, &rfds);
      FD_SET(devdectfd, &rfds);
   
      if (-1 == select(devdectfd+1, &rfds, NULL, NULL, NULL))
      {
         perror("select() failer");
         exit(-1);
      }

      // if (FD_ISSET(STDIN_FILENO, &rfds))
      // {
         // handle_user_input();
      // }
	DEBUG_PRINT("%s: \n", __FUNCTION__);
	if (FD_ISSET(devdectfd, &rfds))
      {
         handle_dect_mail();
      }
   }
}
#endif

/****************************************************************************
Main application
****************************************************************************/
/****************************************************************************
*  FUNCTION: natalie_main
*
*  INPUTS  : none
*  OUTPUTS : none
*  RETURNS :  none
*
*  DESCRIPTION: This function initializes bus, mutexes, hdlc fsm and creates the tx, rx threads.
*               
****************************************************************************/
/*Mutexes*/
pthread_mutexattr_t     attrBusmHdlcSem;
pthread_mutex_t 		semBusmHdlcSem;     // mutex semaphore to protect hdlc data
pthread_mutex_t 		semTestbusFrameQueueSem;     // mutex semaphore to protect TestbusFrameQueue

/* Thread handles */
pthread_t NatalieId;  
// Used to stop the tasks.
volatile bool StopNatalieTask;
void *NatalieThread(void);

void natalie_main(void)
{
	int i;
	char *NatalieIdmessage = "Natalie Thread";

	// Stop the threads
	StopNatalieTask = TRUE;

	//Initialize
	dect_dev_init();
	dect_nvs_init();
	//dect_demo_mails_init();

	/* Initialize mutexes */
	// pthread_mutex_init(&semTestbusFrameQueueSem,0);
	// pthread_mutexattr_init(&attrBusmHdlcSem); 
	// pthread_mutexattr_setkind_np(&attrBusmHdlcSem, PTHREAD_MUTEX_RECURSIVE_NP);//make it a recursive mutex
	// pthread_mutex_init(&semBusmHdlcSem,&attrBusmHdlcSem);

	if( (i= pthread_create( &NatalieId, NULL, &NatalieThread, NULL) ))
	{
		PRINT_INFO("Natalie Thread creation failed\n");
	}

	/* Initialize threads */	
 	
	StopNatalieTask = FALSE;

	DEBUG_PRINT("\n %s: threads created", __FUNCTION__);
   
	//event_loop();

}

/****************************************************************************
*  FUNCTION: NatalieThread
*
*  INPUTS  : none
*  OUTPUTS : none
*  RETURNS :  none
*
*  DESCRIPTION: This function is the code of the thread responsible for the receive path of the MCU - CVM communication.
*               
****************************************************************************/
void *NatalieThread(void)
{
	fd_set rfds;
	FD_ZERO(&rfds);
	//FD_SET(STDIN_FILENO, &rfds);
	FD_SET(devdectfd, &rfds);
	int ret;
	
	DEBUG_PRINT("%s: entry \n", __FUNCTION__);
	while(StopNatalieTask == TRUE);//On thread creation wait until StopNatalieTask == FALSE
	while(StopNatalieTask == FALSE)
	{
		// multi thread read/write
		//BUS_READ_BYTE_BLOCKING(&tmp);

		ret=select(devdectfd+1, &rfds, NULL, NULL, NULL);
		if (ret < 0){//error
			if (ret == -1)  {//EINTR
				//do_nothing. Fixed timer expiration
			}else
			{
				perror("select() failer");
				exit(-1);
			}
		}
		else{
			// if (FD_ISSET(STDIN_FILENO, &rfds))
			// {
			 // handle_user_input();
			// }
			//DEBUG_PRINT("%s: \n", __FUNCTION__);
			if (FD_ISSET(devdectfd, &rfds))
			{
				handle_dect_mail();
			}
		}
	}
	StopNatalieTask = FALSE;   // To indicate that the task has stopped
	DEBUG_PRINT("%s: exit: \n", __FUNCTION__);	
	pthread_exit(0);
}

/****************************************************************************
*  Program/file: BUS232.CPP
*
*  Copyright (C) by RTX TELECOM A/S, Denmark.
*  These computer program listings and specifications, are the property of
*  RTX TELECOM A/S, Denmark and shall not be reproduced or copied or used in
*  whole or in part without written permission from RTX TELECOM A/S, Denmark.
*
*  Programmer: LHJ
*
*  MODULE: WIN32SIM
*  CONTROLLING DOCUMENT:
*  SYSTEM DEPENDENCIES:
*
*
*  DESCRIPTION: RS232 driver
*
*
*
*
*
****************************************************************************/

#define CVMCON



#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <termios.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>	// getpid

#include <sys/ioctl.h>
#include <signal.h>

#include <endian.h>

#include "std-def.h"
#include "dbg_mail.h"
//#include "cmclient.h"
#include "hdlc.h"
#ifndef mips
#include "regs_sc14450.h"
#endif


/****************************************************************************
*                     Definitions
****************************************************************************/
/*HDLC Packet definitions */
#define MAX_PACKET_LENGTH 350
#define MIN_PACKET_LENGTH 1   // Control frames have length 1

//#define BUSM_FLOW_FINAL         0x08
//#define BUSM_FLOW_RX_SEQ_MASK   0x07
//#define BUSM_FLOW_TX_SEQ_MASK   0x70
//#define BUSM_FLOW_CONTROL_FRAME 0x80
//#define BUSM_FLOW_CONTROL_MASK  0x70
//#define BUSM_FLOW_CONTROL_SHIFT 4
#define BUSM_HDLC_SEQ_MOD 		0x07

#define BUSM_MAX_OUTSTANDING_FRAMES 		0x06
//#define BUSM_MAX_FRAMES_BEFORE_FINAL_FLAG	0x03
//#define NO_ACKNOWLEDGE_TIMEOUT				0.2 //200 ms
#define NO_ACKNOWLEDGE_TIMEOUT_NS			800000000	// 800ms
//#define NO_ACKNOWLEDGE_TIMEOUT_NS			200000000	// 200ms

/*Mutexes */
#define MUTEXOBTAIN_semTestbusFrameQueueSem D_PRINT("%s: semTestbusFrameQueueSem lock req \n", __FUNCTION__); pthread_mutex_lock( &semTestbusFrameQueueSem ); D_PRINT("%s: semTestbusFrameQueueSem lock ack \n", __FUNCTION__); 
#define MUTEXRELEASE_semTestbusFrameQueueSem pthread_mutex_unlock( &semTestbusFrameQueueSem ); D_PRINT("%s: semTestbusFrameQueueSem lock rel \n", __FUNCTION__); 
#define MUTEXOBTAIN_semBusmHdlcSem D_PRINT("%s: semBusmHdlcSem lock req \n", __FUNCTION__); pthread_mutex_lock( &semBusmHdlcSem ); D_PRINT("%s: semBusmHdlcSem lock ack \n", __FUNCTION__); 
#define MUTEXRELEASE_semBusmHdlcSem pthread_mutex_unlock( &semBusmHdlcSem ); D_PRINT("%s: semBusmHdlcSem lock rel \n", __FUNCTION__); 

/*General*/
#define FALSE 0
#define TRUE 1

/* Handle 0x4000 RESET_REQ... sleep a moment */
//#define RX_THREAD_SLEEP_ON_RESET_REQ

/****************************************************************************
*                     Enumerations/Type definitions/Structs
****************************************************************************/

typedef enum
{
  BUSM_UNLOCKED,
  BUSM_LOCK_PENDING,
  BUSM_LOCKED
}ENUM8(eBusmState);

typedef enum
{
  INFORMATION_FRAME,
  SUPERVISORY_FRAME,
  UNNUMBERED_FRAME,
  MAX_FRAME
} ENUM8(eFrameTypeEnum);

typedef enum
{
  BUSM_FLOW_CTRL_RR  = 0 ,
  BUSM_FLOW_CTRL_REJ = 1 ,
  BUSM_FLOW_CTRL_RNR = 2 ,
} ENUM8(BusmCtrlSpecType);

typedef enum
{
  BUSM_FLOW_UNNMB_SABM = 0,  // Set Asynchronous Balanced Mode, i.e. ask peer to reset counters
} ENUM8(BusmUnNmbSpecType);

#if defined( __BYTE_ORDER ) && ( __BYTE_ORDER == __BIG_ENDIAN )
typedef struct
{
  uint8 InfoControl : 1; // Must be 0
  uint8 TxSeq       : 3; // Sequence of this frame
  uint8 PollFinal   : 1; // If set -> peer must be answered
  uint8 RxSeq       : 3; // Next sequence expected from peer
} BUSM_InfoFrameType;
#else
typedef struct
{
  uint8 RxSeq       : 3; // Next sequence expected from peer
  uint8 PollFinal   : 1; // If set -> peer must be answered
  uint8 TxSeq       : 3; // Sequence of this frame
  uint8 InfoControl : 1; // Must be 0
} BUSM_InfoFrameType;
#endif

#if defined( __BYTE_ORDER ) && ( __BYTE_ORDER == __BIG_ENDIAN )
typedef struct
{
  uint8 InfoControl         : 1; // Must be 1
  uint8 Supervisory         : 1; // Must be 0
  BusmCtrlSpecType CtrlSpec : 2; 
  uint8 PollFinal           : 1; // If set -> peer must be answered -
  uint8 RxSeq               : 3; // Next sequence expected from peer
} BUSM_SuperVisoryControlFrameType;
#else
typedef struct
{
  uint8 RxSeq               : 3; // Next sequence expected from peer
  uint8 PollFinal           : 1; // If set -> peer must be answered -
  BusmCtrlSpecType CtrlSpec : 2; 
  uint8 Supervisory         : 1; // Must be 0
  uint8 InfoControl         : 1; // Must be 1
} BUSM_SuperVisoryControlFrameType;
#endif

#if defined( __BYTE_ORDER ) && ( __BYTE_ORDER == __BIG_ENDIAN )
typedef struct
{
  uint8 InfoControl             : 1; // Must be 1
  uint8 Unnumbered              : 1; // Must be 1
  BusmUnNmbSpecType UnNmbSpec   : 2;
  uint8 PollFinal               : 1; // If set -> peer must be answered - 
  uint8 Modifier                : 3; // Not used
} BUSM_UnnumberedControlFrameType;
#else
typedef struct
{
  uint8 Modifier                : 3; // Not used
  uint8 PollFinal               : 1; // If set -> peer must be answered - 
  BusmUnNmbSpecType UnNmbSpec   : 2;
  uint8 Unnumbered              : 1; // Must be 1
  uint8 InfoControl             : 1; // Must be 1
} BUSM_UnnumberedControlFrameType;
#endif

// Types for length and sequence fields in buffer
typedef uint16 BUSM_PacketLengthType;
typedef uint8 BUSM_SequenceNrType;
typedef uint8 BUSM_DleType;
// Type for paylaod header
typedef struct 
{
  uint8 uProgId;
  uint8 uTaskId;
} BUSM_PayloadHeaderType;

// Buffer header, a mail in the buffer is organized as: BUSM_BufferHeaderType - Mail - BUSM_TailMarkerType
typedef struct
{
  //BUSM_PacketLengthType length; // length filtered out in RSX
  BUSM_SequenceNrType uControl;
  BUSM_PayloadHeaderType payloadHead;
} BUSM_BufferHeaderType;


typedef struct
{
  BUSM_DleType Dle;
  uint8 uLengthMSB;
  uint8 uLengthLSB;
  BUSM_InfoFrameType uControl;
  BUSM_PayloadHeaderType payloadHead;
  uint8 payloadTail[1];  // Equals zero length package with 1 CRC byte
} BUSM_FrameType;

typedef struct 
{
  uint8   bExpectedRxSeq; 
  uint8   bNextTxSeq;
  uint8   bTxNoAckCnt;
  uint8   bExpectedAck;
  //clock_t tNoAckNowledge;
  struct timespec tNoAckNowledge;
 
  eBusmState eHdlcState;
} __attribute__((packed))sHdlcControlType;

typedef struct 
{
  int  iTaskId;
  int  iLength;
  void *Next;
  uint8 Data[1]; // variable length
} sUnsentInfoframes;

// Queue stuff.
struct QueueStorage {
  struct QueueStorage *Next;
  void *Data;
};

typedef struct {
  struct QueueStorage *First,*Last;
} QueueRecord;

typedef struct {
  uint8 bLength;
  uint8 bData[1];
} QueueElement;


/****************************************************************************
*                     Variable declarations
****************************************************************************/
/* Global variables */
sHdlcControlType sHdlcControl;
sUnsentInfoframes *FirstUnsent=NULL;
sUnsentInfoframes *LastUnsent=NULL;
QueueRecord TestbusFrameQueue;

uint8 WrongFrame;
int RtsTimeout = 0;
uint32 iTxDelay = 0;

BUSM_FrameType *NotAcknowledgeFramePtr[BUSM_HDLC_SEQ_MOD+1];

/*Mutexes*/
pthread_mutexattr_t     attrBusmHdlcSem;
pthread_mutex_t 		semBusmHdlcSem;     // mutex semaphore to protect hdlc data
pthread_mutex_t 		semTestbusFrameQueueSem;     // mutex semaphore to protect TestbusFrameQueue

/* Thread handles */
pthread_t Rx232Id, Tx232Id, RtsTaskId;  
// Used to stop the tasks.
volatile bool StopTxTask, StopRxTask, StopRtsTask;

/****************************************************************************
*                      Function Prototypes
****************************************************************************/
void hdlc_Init(void);
void hdlc_Exit(int dummy);
void hdlc_ResetHDLCprotocol(void);
void hdlc_RetransmitFrame(BUSM_FrameType *bInputDataPtr, bool boolPollFinal);
void hdlc_SendPacketToUnit(int iTaskId, int iLength, unsigned char *bInputDataPtr);
void hdlc_SendSupervisorFrame(int iTaskId, bool boolPollFinal);
void hdlc_SendUnnumberedFrame(int iTaskId, bool boolPollFinal);
bool RsxSwpCheckReceivedFrame(uint8 *RxFrame);
bool hdlc_Busy(void);
static bool Between(unsigned char a, unsigned int b, unsigned int c);
void *RxThread(void *dummy);
void *TxThread(void *dummy);
/****************************************************************************
*                      Function declarations
****************************************************************************/


/****************** General functions section **********************************/
void EnQueue(QueueRecord *rec,void *vdata)
{
	struct QueueStorage *tmp;
	
	tmp=(struct QueueStorage *) malloc(sizeof(struct QueueStorage));
	tmp->Next=NULL;
	tmp->Data=vdata;
	if(rec->First==NULL) {
		rec->First=tmp;
		rec->Last=tmp;
	} 
	else {
		rec->Last->Next=tmp;
		rec->Last=tmp;
	}
}

void *DeQueue(QueueRecord *rec)
{
	void *tmp;
	struct QueueStorage *tmpqe;

	if(rec->First==NULL)
		return NULL;
	tmpqe=rec->First;
	rec->First=tmpqe->Next;
	tmp=tmpqe->Data;
	free(tmpqe);
	return tmp;
}

/****************** HDLC functions section **********************************/

/****************************************************************************
*  FUNCTION: hdlc_Exit
*
*  INPUTS  : none
*  OUTPUTS : none
*  RETURNS :  none
*
*  DESCRIPTION: This function initializes bus, mutexes, hdlc fsm and creates the tx, rx threads.
*               
****************************************************************************/
//#define CALC_CONTEXT

#ifdef CALC_CONTEXT
int context_max = 0;
#endif

void hdlc_Exit(int dummy)
{
	// Stop the threads
	StopTxTask = TRUE;
	StopRxTask = TRUE;
	StopRtsTask = TRUE;

#ifdef CALC_CONTEXT	
	printf( "context_max=%d\n", context_max );
#endif

	//  PurgeComm(hComPortHandle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	#if 0
		while(StopTxTask || StopRxTask || StopRtsTask)
		{
			usleep(100000);
		}

		/* Wait till threads are complete before main continues. Unless we  */
		/* wait we run the risk of executing an exit which will terminate   */
		/* the process and all threads before the threads have completed.   */

		pthread_join( Rx232Id, NULL);
		pthread_join( Tx232Id, NULL); 
		pthread_join( RtsTaskId, NULL);  
	#endif	
	printf ("%s: exiting ...(pid=%d) \n", __FUNCTION__, getpid());
	BUS_CLOSE();
	//Exit
	exit(EXIT_FAILURE);
}
/****************************************************************************
*  FUNCTION: hdlc_Init
*
*  INPUTS  : none
*  OUTPUTS : none
*  RETURNS :  none
*
*  DESCRIPTION: This function initializes bus, mutexes, hdlc fsm and creates the tx, rx threads.
*               
****************************************************************************/
void hdlc_Init(void)
{
	extern int pthread_mutexattr_setkind_np(pthread_mutexattr_t *attr, int kind);
	
	int i;
	//char *Tx232Idmessage = "Tx232 Thread";
	//char *Rx232Idmessage = "Rx232 Thread";
	
 
	/* Initialize bus */
	if (BUS_OPEN() < 0)
		return;


		// Stop the threads
	StopTxTask = TRUE;
	StopRxTask = TRUE;


	/* Initialize mutexes */
	pthread_mutex_init(&semTestbusFrameQueueSem,0);
	pthread_mutexattr_init(&attrBusmHdlcSem); 
	pthread_mutexattr_setkind_np(&attrBusmHdlcSem, PTHREAD_MUTEX_RECURSIVE_NP);//make it a recursive mutex
	pthread_mutex_init(&semBusmHdlcSem,&attrBusmHdlcSem);

	/* Initialize HDLC state */
	sHdlcControl.eHdlcState = BUSM_UNLOCKED;	
	for (i=0;i<BUSM_HDLC_SEQ_MOD;i++)
	{
		NotAcknowledgeFramePtr[i] = NULL;
	}

	if( (i= pthread_create( &Tx232Id, NULL, &TxThread, NULL) ))
	{
		printf("Tx232Id Thread creation failed\n");
	}
	pthread_detach(Tx232Id) ;
	/* Create independent threads each of which will execute function */
	if( (i= pthread_create( &Rx232Id, NULL, &RxThread, NULL) ))
	{
		printf("Rx232Id Thread creation failed\n");
	}
	pthread_detach(Rx232Id) ;


		/* Initialize threads */	
 	
	StopTxTask = FALSE;
	StopRxTask = FALSE;

	DEBUG_PRINT("\n %s: threads created", __FUNCTION__);

	if (sHdlcControl.eHdlcState == BUSM_UNLOCKED)
	{
//		hdlc_SendUnnumberedFrame(0, 1); // Final flag set, wiat for Unnumbered frame response
//		hdlc_SendUnnumberedFrame(0, 0); // Final flag set, wiat for Unnumbered frame response		//giag
		sHdlcControl.eHdlcState = BUSM_LOCK_PENDING;
	}	

}

/****************************************************************************
*  FUNCTION : hdlc_SendPacketToUnit(int iTaskId, int iLength, unsigned char *bInputDataPtr)
*
*  INPUTS : iTaskId (not used)
*  		iLength, the lenght of the packet data
*		bInputDataPtr, ptr to packet data
*  DESCRIPTION: This function creates and enqueues a packet for transmition to the CVM module.
*               
****************************************************************************/
void hdlc_SendPacketToUnit(int iTaskId, int iLength, unsigned char *bInputDataPtr)
{
	BUSM_FrameType *bDataPtr,*pTempNoAckPtr;
	uint16 wDataLength = (uint16) (iLength + 3);
	BUSM_InfoFrameType bControl;
	uint8 bCheckSum,i;

	MUTEXOBTAIN_semBusmHdlcSem;

//	printf("%s\n", __FUNCTION__);

	if (sHdlcControl.bTxNoAckCnt<BUSM_MAX_OUTSTANDING_FRAMES && sHdlcControl.eHdlcState == BUSM_LOCKED)
	{     
		bDataPtr = (BUSM_FrameType*) malloc(iLength + sizeof(BUSM_FrameType));
		pTempNoAckPtr = (BUSM_FrameType*) malloc(iLength + sizeof(BUSM_FrameType));

		if(bDataPtr==NULL || pTempNoAckPtr==NULL)
			return;

		sHdlcControl.bTxNoAckCnt++;
		bControl.InfoControl = 0;
		bControl.PollFinal   = 1;
		bControl.RxSeq       = sHdlcControl.bExpectedRxSeq;  // Acknowledge received frames
		bControl.TxSeq       = sHdlcControl.bNextTxSeq;       
		NotAcknowledgeFramePtr[bControl.TxSeq] = pTempNoAckPtr;
		sHdlcControl.bNextTxSeq = (sHdlcControl.bNextTxSeq + 1 ) & BUSM_HDLC_SEQ_MOD;
		bDataPtr->Dle = 0x10;                                     // DLE
		bDataPtr->uLengthMSB = (uint8) ((wDataLength&0xff00)>>8); // Length MSB
		bDataPtr->uLengthLSB = (uint8) (wDataLength&0x00ff);      // Length LSB
		bDataPtr->uControl   = (BUSM_InfoFrameType)bControl;      // Sequence number
		bDataPtr->payloadHead.uProgId = 0;                        // Program ID
		bDataPtr->payloadHead.uTaskId = (uint8) iTaskId;          // Task ID
		if (iLength)
			memcpy(bDataPtr->payloadTail, bInputDataPtr, iLength);      // Data       

		bCheckSum = (*(uint8*)&bDataPtr->uControl) + bDataPtr->payloadHead.uProgId + bDataPtr->payloadHead.uTaskId;
		for(i=0; i<iLength; i++)
			bCheckSum += bDataPtr->payloadTail[i];

		bDataPtr->payloadTail[iLength] = bCheckSum;                 // Checksum

		if (bControl.PollFinal) // if we have set final flag then start retransmit timer.
		{
			clock_gettime(CLOCK_REALTIME, &sHdlcControl.tNoAckNowledge);
			TIME_PRINT("%s: %d sec %d nse\n", __FUNCTION__,sHdlcControl.tNoAckNowledge.tv_sec,sHdlcControl.tNoAckNowledge.tv_nsec);
		}

		// Store unacknowledged frames
		memcpy(pTempNoAckPtr,bDataPtr,iLength + sizeof(BUSM_FrameType));

		MUTEXOBTAIN_semTestbusFrameQueueSem;
		EnQueue(&TestbusFrameQueue, bDataPtr);
		MUTEXRELEASE_semTestbusFrameQueueSem;
	}
	else
	{
		sUnsentInfoframes *NextUnsendFrame;

		NextUnsendFrame = (sUnsentInfoframes *)malloc(sizeof(sUnsentInfoframes)+iLength);
		if (NextUnsendFrame != NULL) { 
			// protected by the semaphore semBusmHdlcSem
			memcpy(NextUnsendFrame->Data, bInputDataPtr,iLength);
			NextUnsendFrame->iTaskId = iTaskId;
			NextUnsendFrame->iLength = iLength;
			NextUnsendFrame->Next = NULL;       
			if (FirstUnsent==NULL) // First in list
			{
				FirstUnsent = NextUnsendFrame;
				LastUnsent = FirstUnsent;	 
			}
			else
			{
				LastUnsent->Next = NextUnsendFrame;
				LastUnsent = NextUnsendFrame;
			}
		}
	}
	MUTEXRELEASE_semBusmHdlcSem;
}


/****************************************************************************
*  FUNCTION: hdlc_SendSupervisorFrame(int iTaskId, bool boolPollFinal)
*
*  INPUTS:	iTaskId (not used)
*  		boolPollFinal, denotes whether the pollfinal bit should be raised or not.
*
*  DESCRIPTION: This function creates and enqueues a supervisor frame for transmition to the CVM module.
*               
****************************************************************************/
void hdlc_SendSupervisorFrame(int iTaskId, bool boolPollFinal)
{
	BUSM_FrameType *bDataPtr = NULL;
	uint16 wDataLength = (uint16) (1);
	BUSM_SuperVisoryControlFrameType bControl;
	uint8 bCheckSum;

	MUTEXOBTAIN_semBusmHdlcSem;

//	printf("%s\n", __FUNCTION__);

	if (sHdlcControl.eHdlcState == BUSM_LOCKED)
	{
		bDataPtr = (BUSM_FrameType*) malloc(sizeof(BUSM_FrameType));

		if(bDataPtr==NULL)
			return;

		bControl.CtrlSpec = BUSM_FLOW_CTRL_RR;
		bControl.InfoControl = 1; // Supervisory frame
		bControl.Supervisory = 0;
		bControl.PollFinal   = boolPollFinal; 
		bControl.RxSeq       = sHdlcControl.bExpectedRxSeq;  // Acknowledge received frames

		bDataPtr->Dle = 0x10;                                     // DLE
		bDataPtr->uLengthMSB = (uint8) ((wDataLength&0xff00)>>8); // Length MSB
		bDataPtr->uLengthLSB = (uint8) (wDataLength&0x00ff);      // Length LSB
		bDataPtr->uControl   = *(BUSM_InfoFrameType*)(void*)&bControl;   // Sequence number

		bCheckSum = (*(uint8*)&bDataPtr->uControl);
		bDataPtr->payloadHead.uProgId = bCheckSum;                 // Checksum
	}

	MUTEXRELEASE_semBusmHdlcSem;
	
	if( bDataPtr == NULL )
		return;
	
	MUTEXOBTAIN_semTestbusFrameQueueSem;
	EnQueue(&TestbusFrameQueue, bDataPtr);
	MUTEXRELEASE_semTestbusFrameQueueSem;
}

/****************************************************************************
*  FUNCTION: hdlc_SendUnnumberedFrame(int iTaskId, bool boolPollFinal)
*
*  INPUTS:	iTaskId (not used)
*  		boolPollFinal, denotes whether the pollfinal bit should be raised or not.
*
*  DESCRIPTION: This function creates and enqueues a SABM frame for transmition to the CVM module.
*               
****************************************************************************/
void hdlc_SendUnnumberedFrame(int iTaskId, bool boolPollFinal)
{
	BUSM_FrameType *bDataPtr;
	uint16 wDataLength = (uint16) (1);
	BUSM_UnnumberedControlFrameType bControl;
	uint8 bCheckSum;

	MUTEXOBTAIN_semBusmHdlcSem;

//	printf("%s\n", __FUNCTION__);

	bDataPtr = (BUSM_FrameType*) malloc(sizeof(BUSM_FrameType));

	if(bDataPtr==NULL)
		return;

	bControl.InfoControl = 1; //
	bControl.Unnumbered  = 1; // Unnumbered frame 
	bControl.PollFinal   = boolPollFinal;    
	bControl.Modifier    = 0;
	bControl.UnNmbSpec   = BUSM_FLOW_UNNMB_SABM;   // SABM control frame

	bDataPtr->Dle = 0x10;                                     // DLE
	bDataPtr->uLengthMSB = (uint8) ((wDataLength&0xff00)>>8); // Length MSB
	bDataPtr->uLengthLSB = (uint8) (wDataLength&0x00ff);      // Length LSB
	bDataPtr->uControl   = *(BUSM_InfoFrameType*)(void*)&bControl;   // Sequence number

	bCheckSum = (*(uint8*)&bDataPtr->uControl);
	bDataPtr->payloadHead.uProgId = bCheckSum;                 // Checksum

	MUTEXRELEASE_semBusmHdlcSem;
	if (bControl.PollFinal) // if we have set final flag then start retransmit timer.
	{
		clock_gettime(CLOCK_REALTIME, &sHdlcControl.tNoAckNowledge);
		TIME_PRINT("%s: %d sec %d nse\n", __FUNCTION__,sHdlcControl.tNoAckNowledge.tv_sec,sHdlcControl.tNoAckNowledge.tv_nsec);
	}

	MUTEXOBTAIN_semTestbusFrameQueueSem;
	EnQueue(&TestbusFrameQueue, bDataPtr);
	MUTEXRELEASE_semTestbusFrameQueueSem;
}

/****************************************************************************
*  FUNCTION: hdlc_RetransmitFrame(BUSM_FrameType *bInputDataPtr, bool boolPollFinal)
*
*  INPUTS:	iLength, the lenght of the packet data
*		bInputDataPtr, ptr to packet data
*
*  DESCRIPTION: This function retransmits the frame pointed by the bInputDataPtr.
*               
****************************************************************************/
void hdlc_RetransmitFrame(BUSM_FrameType *bInputDataPtr, bool boolPollFinal)
{
	BUSM_FrameType *bDataPtr = NULL;

	uint16 iLength;
	uint8 bCheckSum,i;

//	printf("%s\n", __FUNCTION__);

	if (sHdlcControl.eHdlcState == BUSM_LOCKED)
	{
		iLength = (bInputDataPtr->uLengthMSB<<8) + bInputDataPtr->uLengthLSB-3;

		bDataPtr = (BUSM_FrameType*)malloc(iLength+sizeof(BUSM_FrameType)); 

		if(bDataPtr==NULL)
			return;

		memcpy(bDataPtr,bInputDataPtr,iLength+sizeof(BUSM_FrameType)); // Copy mail

		bDataPtr->uControl.PollFinal = boolPollFinal;

		bDataPtr->uControl.RxSeq = sHdlcControl.bExpectedRxSeq;

		bCheckSum = (*(uint8*)&bDataPtr->uControl) + bDataPtr->payloadHead.uProgId + bDataPtr->payloadHead.uTaskId;
		for(i=0; i<iLength; i++)
		{
			bCheckSum += bDataPtr->payloadTail[i];
		}
		bDataPtr->payloadTail[iLength] = bCheckSum;                 // Checksum

	}
	
	if( bDataPtr == NULL )
		return;

	MUTEXOBTAIN_semTestbusFrameQueueSem;
	EnQueue(&TestbusFrameQueue, bDataPtr);
	MUTEXRELEASE_semTestbusFrameQueueSem;
}

/****************************************************************************
*  FUNCTION: hdlc_Busy(void)
*
*  INPUTS:	none
*  RETURNS : 	1 ,if the bus is locked,
*			0, otherwise
*  DESCRIPTION: 
*               
****************************************************************************/
bool hdlc_Busy(void)
{
  return (sHdlcControl.eHdlcState != BUSM_LOCKED);
}

/****************************************************************************
*  FUNCTION: hdlc_ResetHDLCprotocol(void)
*
*  INPUTS:	none
*
*  DESCRIPTION: This function resets the HDLC protocol.
*               
****************************************************************************/
void hdlc_ResetHDLCprotocol(void)
{
  MUTEXOBTAIN_semBusmHdlcSem;
  memset((void*)&sHdlcControl,0x00,sizeof(sHdlcControl)); // Reset variables
  sHdlcControl.eHdlcState = BUSM_LOCK_PENDING;
  MUTEXRELEASE_semBusmHdlcSem;
  hdlc_SendUnnumberedFrame(0, 1); // Final flag set, wiat for Unnumbered frame response    
}

/****************************************************************************
*  FUNCTION: RsxSwpCheckReceivedFrame(uint8 *RxFrame)
*
*  INPUTS:	RxFrame, ptr to the received frame
*
*  DESCRIPTION: This function checks the received frames, updates hdlc state and sends responses to CVM .
*               
****************************************************************************/
bool RsxSwpCheckReceivedFrame(uint8 *RxFrame)
{
	BUSM_BufferHeaderType *FramePtr=(BUSM_BufferHeaderType *)RxFrame;
	bool boolReturnValue=FALSE,boolTestAcknowledge=FALSE;
	uint8 uControl;
	eFrameTypeEnum eFrameType;

	uControl = (uint8)FramePtr->uControl;

//	printf("%s\n", __FUNCTION__);

	if (((BUSM_InfoFrameType*)&uControl)->InfoControl == 0)
	{
		eFrameType = INFORMATION_FRAME;
	}
	else if (((BUSM_SuperVisoryControlFrameType*)&uControl)->Supervisory == 0)
	{
		eFrameType = SUPERVISORY_FRAME;    
	}
	else
	{
		eFrameType = UNNUMBERED_FRAME;    
	}

	MUTEXOBTAIN_semBusmHdlcSem;
	switch (eFrameType){
	case INFORMATION_FRAME:
	{
		if (sHdlcControl.eHdlcState == BUSM_LOCKED)  
		{
			if (((BUSM_InfoFrameType*)&uControl)->TxSeq == sHdlcControl.bExpectedRxSeq)
			{
				sHdlcControl.bExpectedRxSeq = (sHdlcControl.bExpectedRxSeq + 1) & BUSM_HDLC_SEQ_MOD; 
				boolTestAcknowledge = TRUE;
				boolReturnValue = TRUE;
			}
			if (((BUSM_InfoFrameType*)&uControl)->PollFinal)  // if final send acknowledge frame
			{
				hdlc_SendSupervisorFrame(0,0);
			}
		}
		else
		{
			// Try sending reset frame again
			hdlc_SendUnnumberedFrame(0, 1); // Final flag set, wiat for Unnumbered frame response
			sHdlcControl.eHdlcState = BUSM_LOCK_PENDING;
		}
	}
	break;
	case SUPERVISORY_FRAME:
		if (sHdlcControl.eHdlcState == BUSM_LOCKED)  
		{
			boolTestAcknowledge = TRUE;
		}
		else
		{
			// Try sending reset frame again
			hdlc_SendUnnumberedFrame(0, 1); // Final flag set, wiat for Unnumbered frame response
			sHdlcControl.eHdlcState = BUSM_LOCK_PENDING;
		}
		break;
	case UNNUMBERED_FRAME:
		//
		if (((BUSM_UnnumberedControlFrameType*)&uControl)->PollFinal) 
		{

				

			// reset HDLC
			memset((void*)&sHdlcControl,0x00,sizeof(sHdlcControl)); // Reset variables
			sHdlcControl.eHdlcState = BUSM_LOCKED;
			hdlc_SendUnnumberedFrame(0,0); //Answer
			// Check for unsent frames

			if (FirstUnsent)
			{

				// protected by the semaphore semBusmHdlcSem
				sUnsentInfoframes *TempUnsendFrame;  
				hdlc_SendPacketToUnit(FirstUnsent->iTaskId,FirstUnsent->iLength,FirstUnsent->Data);
				TempUnsendFrame = FirstUnsent;
				FirstUnsent = (sUnsentInfoframes*)FirstUnsent->Next; // Remove from list
				free(TempUnsendFrame);              
			}
			
	
		}
		else
		{        
			if (sHdlcControl.eHdlcState == BUSM_LOCK_PENDING)
			{
				memset((void*)&sHdlcControl,0x00,sizeof(sHdlcControl)); // Reset variables
				sHdlcControl.eHdlcState = BUSM_LOCKED;
				// Check for unsent frames
				if (FirstUnsent)
				{
					// protected by the semaphore semBusmHdlcSem
					sUnsentInfoframes *TempUnsendFrame;  
					hdlc_SendPacketToUnit(FirstUnsent->iTaskId,FirstUnsent->iLength,FirstUnsent->Data);
					TempUnsendFrame = FirstUnsent;
					FirstUnsent = (sUnsentInfoframes*)FirstUnsent->Next; // Remove from list
					free(TempUnsendFrame);              
				}
			}
			else
			{
			// Ignore 
			}
		}
	break;
	}

	if (boolTestAcknowledge)
	{
		while(Between(sHdlcControl.bExpectedAck, (((BUSM_InfoFrameType*)&uControl)->RxSeq+BUSM_HDLC_SEQ_MOD)&BUSM_HDLC_SEQ_MOD, sHdlcControl.bNextTxSeq))
		{
			// Free acknowledged frames
			sHdlcControl.bTxNoAckCnt--;        

			if (NotAcknowledgeFramePtr[sHdlcControl.bExpectedAck])
			{
				free(NotAcknowledgeFramePtr[sHdlcControl.bExpectedAck]);
				NotAcknowledgeFramePtr[sHdlcControl.bExpectedAck] = NULL;
			}

			if (sHdlcControl.bTxNoAckCnt==0) // If we are below the no acknowledge limit clear acknowledge timer
			{
				//    sHdlcControl.tNoAckNowledge = 0; 
				TIME_PRINT("\n %s: Stop TX timer", __FUNCTION__);	
				sHdlcControl.tNoAckNowledge.tv_sec = 0;
				sHdlcControl.tNoAckNowledge.tv_nsec = 0;
			}
			sHdlcControl.bExpectedAck=(sHdlcControl.bExpectedAck+1) & BUSM_HDLC_SEQ_MOD;
			// Check for unsent frames
			if (FirstUnsent&& sHdlcControl.eHdlcState == BUSM_LOCKED)
			{
				// protected by the semaphore semBusmHdlcSem
				sUnsentInfoframes *TempUnsendFrame;  
				hdlc_SendPacketToUnit(FirstUnsent->iTaskId,FirstUnsent->iLength,FirstUnsent->Data);

				TempUnsendFrame = FirstUnsent;
				FirstUnsent = (sUnsentInfoframes*)FirstUnsent->Next; // Remove from list
				free(TempUnsendFrame);              
			}
		}
	}
	MUTEXRELEASE_semBusmHdlcSem;
	return boolReturnValue; 
}

/****************************************************************************
*  FUNCTION: Between
*
*  INPUTS  : a,b,c
*  OUTPUTS : none
*  RETURNS : Return:  TRUE if  a <= b < c
*
*  DESCRIPTION: This function is used to check that the received acknowledgment
*               number is within the acceptable vindow, the check is circulary.
*               
****************************************************************************/
static bool Between(unsigned char a, unsigned int b, unsigned int c)
{
 return ( ((a<=b) && (b<c)) || ((c<a) && (a<=b)) || ((b<c) && (c<a)));
}




/****************** HDLC threads section **********************************/

/****************************************************************************
*  FUNCTION: TxThread
*
*  INPUTS  : none
*  OUTPUTS : none
*  RETURNS :  none
*
*  DESCRIPTION: This function is the code of the thread responsible for the transmit path of the MCU - CVM communication.
*               
****************************************************************************/
void *TxThread(void *dummy)
{
	uint16 bSenderSize;
	uint8 bFirstUnAcknowledged, *Rs232FrameQueuePtr, bTransmit232ElementArr[500], i;
	bool boolFinal;	
//	struct timespec timeOut, t_Now, remains;	

	
	
	DEBUG_PRINT("%s: entry (pid=%d) \n", __FUNCTION__, getpid());
	while(StopTxTask == TRUE) usleep (10000);
	while(StopTxTask == FALSE)
	{
		

		MUTEXOBTAIN_semTestbusFrameQueueSem;//Acquire FRAME queue lock and check if there is anything to send 

		
		/* Packets exist in the TX queue */
		if(TestbusFrameQueue.First != NULL)
		{
			

			Rs232FrameQueuePtr = (uint8*) DeQueue(&TestbusFrameQueue);
			bSenderSize = (uint16) (256*Rs232FrameQueuePtr[1] + Rs232FrameQueuePtr[2] + 4);
			memcpy(bTransmit232ElementArr, Rs232FrameQueuePtr, bSenderSize);

			//BUS_WRITE_BYTE_BLOCKING(bTransmit232ElementArr, bSenderSize);
			free(Rs232FrameQueuePtr);//free memory used to store the msg
			MUTEXRELEASE_semTestbusFrameQueueSem;//Release lock 
			BUS_WRITE_BYTE_BLOCKING(bTransmit232ElementArr, bSenderSize);

#if 1			
			/* Dump TX bytes - begin */			
			DUMP_TX_BYTES("Tx: ");
			for(i=0; i<bSenderSize; i++)
			   DUMP_TX_BYTES("%02X ",(int) bTransmit232ElementArr[i]);
			DUMP_TX_BYTES("\n ");       
			/* Dump TX bytes - end */
#endif		
		}
		/* NO Packets exist in the TX queue */
		else
		{
				
			MUTEXRELEASE_semTestbusFrameQueueSem;//Release lock 
			{
			struct timespec timeOut,remains;
			timeOut.tv_sec = 0;
			timeOut.tv_nsec = 10000000; /* 50 milliseconds */
			nanosleep(&timeOut, &remains); //suspend thread execution for a while
			}
			
		}
		
		
		MUTEXOBTAIN_semBusmHdlcSem;//Acquire HDLC queue lock and check if there is anything to send 
		/* Check if TX ACK timer has started (and should be updated)*/
		if ((sHdlcControl.tNoAckNowledge.tv_sec) || (sHdlcControl.tNoAckNowledge.tv_nsec))
		{
			struct timespec t_Now;	

			//Get cur time to check for tmout
			clock_gettime(CLOCK_REALTIME, &t_Now);
			TIME_PRINT("%s: %ld sec %ld nsec\n", __FUNCTION__,t_Now.tv_sec,t_Now.tv_nsec);
			/* TX ACK timer has expired */
			if(((t_Now.tv_sec - sHdlcControl.tNoAckNowledge.tv_sec) * 1000000000) + ((t_Now.tv_nsec - sHdlcControl.tNoAckNowledge.tv_nsec)) >NO_ACKNOWLEDGE_TIMEOUT_NS )
			{   
				//If HDLC connection is not established, send a SABM frame
				if (sHdlcControl.eHdlcState == BUSM_LOCK_PENDING){
					hdlc_SendUnnumberedFrame(0, 1); // Final flag set, wiat for Unnumbered frame response
				}	
				else{
					TIME_PRINT("Ooops Timeout \n");
					// Transmit all outstanding frames again.
					bFirstUnAcknowledged = sHdlcControl.bExpectedAck;
					for (i=0;i<sHdlcControl.bTxNoAckCnt;i++)
					{ 
						if ((i+1)== sHdlcControl.bTxNoAckCnt) // if last
						{
						  boolFinal=TRUE;
						}
						else
						{
						  boolFinal=FALSE;
						}
						hdlc_RetransmitFrame(NotAcknowledgeFramePtr[bFirstUnAcknowledged],boolFinal);
						bFirstUnAcknowledged=(bFirstUnAcknowledged+1)&BUSM_HDLC_SEQ_MOD;
					}
				}
				clock_gettime(CLOCK_REALTIME, &sHdlcControl.tNoAckNowledge);//store TX starting time 
				TIME_PRINT("%s: %d sec %d nse\n", __FUNCTION__,sHdlcControl.tNoAckNowledge.tv_sec,sHdlcControl.tNoAckNowledge.tv_nsec);
			}
		}
		MUTEXRELEASE_semBusmHdlcSem;
		
	}

	StopTxTask = FALSE;   // To indicate that the task has stopped
	pthread_exit(0);
}

/****************************************************************************
*  FUNCTION: RxThread
*
*  INPUTS  : none
*  OUTPUTS : none
*  RETURNS :  none
*
*  DESCRIPTION: This function is the code of the thread responsible for the receive path of the MCU - CVM communication.
*               
****************************************************************************/

// To know more, see file end 
#define SYNC_RX_DLE				// sync RX DLE in bitstream 
//#define LOG_100_UNKNOWN_BYTES	// log 100 unknwon bytes in state 0 
//#define HANDLE_FWU_CRC_ERROR	// During FWU CRC error occurs (old design)

#ifdef SYNC_RX_DLE
volatile int bSyncRxDLE = 0;
#endif
#ifdef LOG_100_UNKNOWN_BYTES
volatile int bPrintRX = 0;
#endif

typedef struct {
	uint16 wReceive232Pos;			// receive bytes number 
	uint16 wDataLength;				// data length from receive packet 
	uint8 *bReceive232ElementArr;	// point to buffer 
	uint8 bReceiveState;			// state, 0~4 normal, 5 complete, 6 error 
	uint8 bCkeckSum;				// checksum for receive packet
} RxStateContext_t;

static inline void RxStateMachineInit(	RxStateContext_t *me, 
										uint8 *pReceiveElementBase )
{
	me ->bReceiveState = 0;
	me ->wReceive232Pos = 0;
	me ->wDataLength = 0;
	me ->bCkeckSum = 0;
	me ->bReceive232ElementArr = pReceiveElementBase;
}

static uint8 RxStateMachineRun( RxStateContext_t *me, uint8 tmp )
{
	switch(me ->bReceiveState)
	{
	case 0:   // Receive DLE
		me ->bReceive232ElementArr ++;
		
		//Dump RX bytes
		DUMP_RX_BYTES("Rx:");
		DUMP_RX_BYTES(" %02x",tmp);
	//	DUMP_RX_BYTES(" S[%02x] %02x",bReceiveState,tmp);

		if(tmp == 0x10)
		{
			me ->bReceiveState = 1;
			me ->wReceive232Pos = 0;
			me ->bCkeckSum = 0;
		} else
			me ->bReceiveState = 6;
		break;
		
	case 1:   // Receive MSB of the length
		me ->bReceive232ElementArr ++;
		
		DUMP_RX_BYTES(" %02x",tmp);
	//	DUMP_RX_BYTES(" (1) S[%02x] %02x",bReceiveState,tmp);
		me ->wDataLength = (uint16) (tmp*256);
		if(me ->wDataLength > MAX_PACKET_LENGTH)
			me ->bReceiveState = 6;
		else
			me ->bReceiveState = 2;
		break;
		
	case 2:   // Receive LSB of the length
		me ->bReceive232ElementArr ++;
		
		DUMP_RX_BYTES(" %02x",tmp);
	//	DUMP_RX_BYTES(" (2) S[%02x] %02x",bReceiveState,tmp);
		me ->wDataLength += tmp;
		if(me ->wDataLength > MAX_PACKET_LENGTH)
			me ->bReceiveState = 6;
		else if(me ->wDataLength < MIN_PACKET_LENGTH)
			me ->bReceiveState = 6;
		else
			me ->bReceiveState = 3;
		break;
		
	case 3:   // Receive Data
		DUMP_RX_BYTES(" %02x",tmp);
	//	DUMP_RX_BYTES(" (3) S[%02x] %02x",bReceiveState,tmp);
		//bReceive232ElementArr[wReceive232Pos] = tmp;
		me ->bCkeckSum += tmp;
		me ->wReceive232Pos++;
		if(me ->wReceive232Pos == me ->wDataLength)
		{
		// Calculate checksum.
		me ->bReceiveState = 4;
		}
		break;
		
	case 4:   // Receive checksum
		DUMP_RX_BYTES(" %02x",tmp);
	//	DUMP_RX_BYTES(" (4) S[%02x] %02x",bReceiveState,tmp);


		if(me ->bCkeckSum == tmp)
		{
			DUMP_RX_BYTES("  CRC ok...\n");

			if (RsxSwpCheckReceivedFrame(me ->bReceive232ElementArr))
			{
				if( me ->wDataLength < 5 ) {
					// 5 is for frame header(1), pid(1), tid(1) and primitive(2). 
					me ->bReceiveState = 6;
					goto label_checksum_ok_done;
				}

				// Received frame O.k.                   
				// Sendmail program
				sendmail_ReceivePacket((uint16) (me ->wDataLength-3), &me ->bReceive232ElementArr[3]);
			}
			else
			{
				//WrongFrame++;
				DUMP_RX_BYTES("  WrongFrame\n");
			}
			
			me ->bReceiveState = 5;

label_checksum_ok_done:
			;
		}
		else
		{
#ifdef LOG_100_UNKNOWN_BYTES
			printf("  CRC error...\n");
#endif
			DUMP_RX_BYTES("  CRC error...\n");

		#if 0
			printf ("0xff442e = %x \n", *(unsigned short *)0xff442e);
			fflush(stdout);
			*(unsigned short*)0xFF442C = 0;//stop dma
			while(1);			
		#endif
		
			me ->bReceiveState = 6;
		}
		
		break;
	}
	
	return 	me ->bReceiveState;
}

#ifndef SYNC_RX_DLE
static inline int RxSyncDLEHandleDone( int context_used, uint8 tmp )
{
	return 0;
}
#else
int RxSyncDLEHandler( int context_used, uint8 tmp )
{
	static uint16 rx_bits = 0;
	static uint16 forbid_sync = 0;
	int i;
	
	rx_bits = ( rx_bits << 8 ) | tmp;
	
	if( tmp == 0x10 ) {
		rx_bits = 0;
		forbid_sync = 0;
		return 0;
	} 
	
	if( bSyncRxDLE == 0 )	// global switch 
		return 0;
	
	if( context_used )		// do only if no context 
		return 0;
	
	if( forbid_sync ) {
		forbid_sync --;
		return 0;
	}
	
	// yes! do sync below here!! 
	
	//fprintf( stderr, "DS:%02X ", tmp );
	//printf( "DS:%02X ", tmp );
	
	if( tmp & 0x80 )	// to to more efficient 
		return 1;
		
	for( i = 1; i < 8; i ++ ) {
		if( ( ( rx_bits >> i ) & 0xFF ) == 0x10 ) {
			//printf( "rx_bits hit %d! ", 8 - i );
			Bus_Seek_bits( 8 - i );
			forbid_sync = 0x20;
		}
	}
	
	return 1;
}
#endif

void *RxThread(void *dummy)
{
#define NUM_OF_RX_CONTEXT	10	// in our test, FWU is 5 
#define FLUSH_ALL_CONTEXT()	{ context_used = 0; wReceive232Pos = 0; }

	uint8 tmp = 0;
	uint16 wReceive232Pos=0;
	uint8 bReceive232ElementArr[1000];
	
	RxStateContext_t context[ NUM_OF_RX_CONTEXT ];
	int context_used = 0;
	int i, t;

	DEBUG_PRINT("%s: entry (pid=%d)\n", __FUNCTION__, getpid());
	while(StopRxTask == TRUE) usleep (10000);
	while(StopRxTask == FALSE)
	{
		// multi thread read/write
		BUS_READ_BYTE_BLOCKING(&tmp);

#ifdef LOG_100_UNKNOWN_BYTES
		if( bPrintRX )
			//fprintf( stderr, "R:%02X ", tmp );
			printf( "R:%02X ", tmp );
#endif
		
		// rx sync DLE handler
		if( RxSyncDLEHandler( context_used, tmp ) )
			continue;
		
		// init context (multi-FSM mode during firmware upgrade only)
		if( context_used < ( FwuFileOpen ? NUM_OF_RX_CONTEXT : 1 ) ) {
			RxStateMachineInit( &context[ context_used ++ ], &bReceive232ElementArr[ wReceive232Pos ] );
		} else {
			//fprintf( stderr, "RxThread: Context is not enough!!\n" );
			//printf( "RxThread: Context is not enough!!\n" );
		}

#ifdef CALC_CONTEXT
		if( context_max < context_used )
			context_max = context_used;
#endif
		
		// save tmp 
		bReceive232ElementArr[ wReceive232Pos ++ ] = tmp;
		
		// run all state machine 
		for( i = 0, t = 0; i < context_used; i ++ ) {
			switch( RxStateMachineRun( &context[ i ], tmp ) ) {
			case 5:
				// a complete frame, so flush all context
				FLUSH_ALL_CONTEXT();
				break;
				
			case 6:
				t ++;	// count for gabage collection 
				break;
			}
		}
		
		if( t == 0 )
			goto label_gabage_collection_done;
		
		// collect gabage (state==6)
		for( i = 0, t = 0; i < context_used; i ++ )
			if( context[ i ].bReceiveState != 6 ) {
				if( i != t )
					context[ t ] = context[ i ];
				t ++;
			} 
			
		context_used = t;

label_gabage_collection_done:
		
		// flush all 
		if( context_used == 0 )
			FLUSH_ALL_CONTEXT();
	}
	
	StopRxTask = FALSE;   // To indicate that the task has stopped
	pthread_exit(0);
}


#if 0
void *RxThread(void *dummy)
{
	//unsigned long dwBytesRead;
	/*volatile*/ uint8 tmp = 0;
	uint8 tmp_pre;
	uint16 wReceive232Pos=0;
	//uint16 wDataLength;
	uint8 bReceive232ElementArr[1000];
	/*volatile*/ //uint8 bReceiveState = 0;
	//uint8 bCkeckSum;

#ifdef LOG_100_UNKNOWN_BYTES
	uint32 UnknownBytes = 0;
	uint8 UnknownBytesBuffer[ 100 ];
#endif
#ifdef HANDLE_FWU_CRC_ERROR
	uint16 FwuCrcErrorRand = 0;
	uint8 FwuCrcErrorIgnoreDLE = 0;
	uint8 FwuCrcErrorFlags = 0;
	uint16 FwuCrcErrorSyncFF10;
	#define FCE_SYNC_FF10		0x01	// bit 0: sync FF 10
	#define FCE_SYNC_FF10_PRE	0x02	// bit 1: memorize before to avoid continous sync. 
	#define FCE_NOT_SYNC		0x04	// bit 2: not do rx sync 
#endif
#ifdef SYNC_RX_DLE
	uint16 rx_bits = 0;
	uint16 forbid_sync = 0;
#endif
	int i;

	DEBUG_PRINT("%s: entry (pid=%d)\n", __FUNCTION__, getpid());
	while(StopRxTask == TRUE) usleep (10000);
	while(StopRxTask == FALSE)
	{
		// multi thread read/write
		
		tmp_pre = tmp;
		BUS_READ_BYTE_BLOCKING(&tmp);
		
#ifdef LOG_100_UNKNOWN_BYTES
		if( bPrintRX )
			//fprintf( stderr, "R:%02X ", tmp );
			printf( "R:%02X ", tmp );
#endif
	
		switch(bReceiveState)
		{
			case 0:   // Receive DLE
				
				//Dump RX bytes
				DUMP_RX_BYTES("Rx:");
				DUMP_RX_BYTES(" %02x",tmp);
			//	DUMP_RX_BYTES(" S[%02x] %02x",bReceiveState,tmp);

#ifdef SYNC_RX_DLE
				rx_bits = ( rx_bits << 8 ) | tmp;
#endif

				if(tmp == 0x10)
				{
#ifdef HANDLE_FWU_CRC_ERROR
					if( FwuCrcErrorFlags & FCE_SYNC_FF10 ) {
						if( tmp_pre == 0xFF || FwuCrcErrorSyncFF10 == 0 ) {
							FwuCrcErrorFlags &= ~FCE_SYNC_FF10;
						} else {
							FwuCrcErrorSyncFF10 --;
							FwuCrcErrorFlags |= FCE_NOT_SYNC;
							goto label_ignore_this_DLE;
						}
					} 
					
					if( FwuCrcErrorIgnoreDLE ) {
						FwuCrcErrorIgnoreDLE --;
						FwuCrcErrorFlags |= FCE_NOT_SYNC;
						goto label_ignore_this_DLE;
					} else
						FwuCrcErrorFlags &= ~FCE_NOT_SYNC;
#endif
#ifdef LOG_100_UNKNOWN_BYTES
					if( UnknownBytes >= 3 ) {
						printf( "-----------\n" );
						for ( i = 0; i < UnknownBytes && i < 100; i ++ )
							printf( "%02X ", UnknownBytesBuffer[ i ] );
						printf( "\n" );
					}
					UnknownBytes = 0;
#endif				
					///////////////////////////////////////
					// original code is here 
					bReceiveState = 1;
					wReceive232Pos = 0;
					bCkeckSum = 0;
					///////////////////////////////////////
#ifdef SYNC_RX_DLE
					rx_bits = 0;
					forbid_sync = 0;
#endif
				} else {
#ifdef SYNC_RX_DLE
					if( bSyncRxDLE == 0 )
						goto label_do_not_sync_rx;
						
  #ifdef HANDLE_FWU_CRC_ERROR
					if( FwuCrcErrorFlags & FCE_NOT_SYNC )
						goto label_do_not_sync_rx;
  #endif
  					if( forbid_sync ) {
  						forbid_sync --;
  						goto label_do_not_sync_rx;
  					}
  					
  					if( tmp & 0x80 )	// to to more efficient 
  						goto label_do_not_sync_rx;
  					
  					//printf( "rx_bits:%X ", rx_bits );
  					for( i = 1; i < 8; i ++ )
  						if( ( ( rx_bits >> i ) & 0xFF ) == 0x10 ) {
  							//printf( "rx_bits hit %d! ", 8 - i );
  							Bus_Seek_bits( 8 - i );
  							forbid_sync = 0x20;
  							break;
  						}
label_do_not_sync_rx:
					;
#endif // SYNC_RX_DLE

#ifdef HANDLE_FWU_CRC_ERROR
label_ignore_this_DLE:
					;
#endif
#ifdef LOG_100_UNKNOWN_BYTES
					if( UnknownBytes < 100 )
						UnknownBytesBuffer[ UnknownBytes ] = tmp;
					UnknownBytes ++;

					if( UnknownBytes == 100 ) {
						int i;
						printf( "HDLC: RxThread 100 unknwon bytes\n" );
						for ( i = 0; i < 100; i ++ )
							printf( "%02X ", UnknownBytesBuffer[ i ] );
						printf( "\n" );
						printf( "-----------\n" );
						bPrintRX = 1;
					}
#endif					
				} // not 0x10
				break;
				
			case 1:   // Receive MSB of the length
				
				DUMP_RX_BYTES(" %02x",tmp);
			//	DUMP_RX_BYTES(" (1) S[%02x] %02x",bReceiveState,tmp);
				wDataLength = (uint16) (tmp*256);
				if(wDataLength > MAX_PACKET_LENGTH)
				{
					if(tmp == 0x10)
					{
						//try staying in same state
					}
					else
					{
						bReceiveState = 0;
					}
				}
				else
					bReceiveState = 2;
				break;
				
			case 2:   // Receive LSB of the length
				DUMP_RX_BYTES(" %02x",tmp);
			//	DUMP_RX_BYTES(" (2) S[%02x] %02x",bReceiveState,tmp);
				wDataLength += tmp;
				if(wDataLength > MAX_PACKET_LENGTH)
				{
					if(tmp == 0x10)
					{
						bReceiveState = 1;
					}
					else
					{
						bReceiveState = 0;
					}
				}
				else if(wDataLength < MIN_PACKET_LENGTH)
				{
					bReceiveState = 0;
				}
				else
					bReceiveState = 3;
				break;
				
			case 3:   // Receive Data
				DUMP_RX_BYTES(" %02x",tmp);
			//	DUMP_RX_BYTES(" (3) S[%02x] %02x",bReceiveState,tmp);
				bReceive232ElementArr[wReceive232Pos] = tmp;
				bCkeckSum += tmp;
				wReceive232Pos++;
				if(wReceive232Pos == wDataLength)
				{
				// Calculate checksum.
				bReceiveState = 4;
				}
				break;
				
			case 4:   // Receive checksum
				DUMP_RX_BYTES(" %02x",tmp);
			//	DUMP_RX_BYTES(" (4) S[%02x] %02x",bReceiveState,tmp);


				if(bCkeckSum == tmp)
				{
					DUMP_RX_BYTES("  CRC ok...\n");

					if (RsxSwpCheckReceivedFrame(bReceive232ElementArr))
					{
						// Received frame O.k.                   
						// Sendmail program
						sendmail_ReceivePacket((uint16) (wDataLength-3), &bReceive232ElementArr[3]);
					}
					else
					{
						WrongFrame++;
						DUMP_RX_BYTES("  WrongFrame\n");
					}
#ifdef HANDLE_FWU_CRC_ERROR
					FwuCrcErrorFlags &= ~FCE_SYNC_FF10_PRE;
#endif
				}
				else
				{
#ifdef LOG_100_UNKNOWN_BYTES
					printf("  CRC error...\n");
#endif
					DUMP_RX_BYTES("  CRC error...\n");
#ifdef HANDLE_FWU_CRC_ERROR
					if( FwuFileOpen ) {
						if( FwuCrcErrorFlags & FCE_SYNC_FF10_PRE ) {
							FwuCrcErrorRand += ( bCkeckSum >> 6 ) ^ ( bCkeckSum >> 3 ) ^ bCkeckSum;
							FwuCrcErrorRand = ( FwuCrcErrorRand & 0xFF ) + ( FwuCrcErrorRand >> 8 );
							
							FwuCrcErrorIgnoreDLE = ( FwuCrcErrorRand >> 1 ) & 0x03;
						} else {
							FwuCrcErrorFlags |= FCE_SYNC_FF10;
							FwuCrcErrorFlags |= FCE_SYNC_FF10_PRE;
							FwuCrcErrorSyncFF10 = 20;	// don't over 20 DLE to reduce risk!  
						}
					}
#endif
				#if 0
					printf ("0xff442e = %x \n", *(unsigned short *)0xff442e);
					fflush(stdout);
					*(unsigned short*)0xFF442C = 0;//stop dma
					while(1);			
				#endif
				}
				bReceiveState = 0;
				
				break;
		}


		
	}
	StopRxTask = FALSE;   // To indicate that the task has stopped
	pthread_exit(0);
}
#endif // 0


void EnableSyncRxDLE( int bEnable )
{
#ifdef SYNC_RX_DLE
	//printf( "[Sync RX DLE:%d]\n", bEnable );
	bSyncRxDLE = bEnable;
  #ifdef LOG_100_UNKNOWN_BYTES
	bPrintRX = bEnable;
  #endif
#endif
}

/*
 * HANDLE_FWU_CRC_ERROR
 * --------------------
 * In firmware upgrade iteration, it may miss '10 00' in preceding of  
 * FF 10 00 12 3F 00 00 07 4F 00 00 49 40 38 30 57 10 00 78 00 00 00 65 .
 * This message is 4F07 (API_FP_FWU_GET_BLOCK_IND). 
 * Unfortunately, there is '10 00 78...' in its payload, and be 
 * recognized as a bad frame with 120 (0x0078) bytes. 
 * Actually, frame length is 0x12, and add header and padding is 23.
 * 120 / 23 =  5 ... 5
 * Then, last 120 bytes will be "00 00 00 65 FF" and "10" is checksum, 
 * so CRC will be error.
 * Again, it will recognize bad frames infinitely. 
 *
 * Solution: 
 * First, we try to synchronize FF 10. 
 * Second, we ignore DLE (10) with intermission or random, 
 * and now random is adopted. 
 * Random is to ignore DLE within 0~3 times. 
 * Only works on firmware upgrade and CRC error. 
 *
 * Better Solution:
 * Now, we use FSM with different context, so it is not needed to 
 * guess DLE ramdonly!! :-)
 *
 * Risk:
 * If ignore with intermission, it may still be infinite in special case. 
 * FF 10 00 12 7B 00 00 07 4F 00 00 49 40 38 10 B3 10 00 78 00 00 00 DD 
 *
 *
 * 
 * SYNC_RX_DLE
 * -----------
 * Firmware upgrade still meets alignment problem, so we try to sync. 
 * 
 * Solution:
 * We try to sync on receive state 0, and recevied character is not 0x10. 
 * 
 * Risk: 
 * Missing DLE frame will trigger this mechanism, and rises alignment
 * issue. 
 * 
 * IMPORTANT!! This is a dangerous definition!! 
 * In my testing, it rises alignments issue (it is fine originally). 
 * We may need to use a flag to turn on this mechanism in some
 * special situations, such as RESET, FWU and so on. 
 * So we handle only *PREDICTABLE* case only!!   
 *
 * Special situations: 
 *  1. FWU
 *     Start:
 *       API_FP_FWU_GET_CRC_RES with Address: 0, Size: 0, CRC: FFFF
 *     End: 
 *       API_FP_FWU_COMPLETE_IND
 *       API_FP_RESET_IND
 */


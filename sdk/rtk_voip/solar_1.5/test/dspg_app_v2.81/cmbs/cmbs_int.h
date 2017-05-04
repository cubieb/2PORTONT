/*!
*  \file       cmbs_int.h
*  \brief      This file contains internal structures and definitions of CMBS
*
*  \author     stein
*
*  @(#)  %filespec: cmbs_int.h~DMZD53#6 %
*
*******************************************************************************
*  \par  History
*  \n==== History ============================================================\n
*  date        name     version   action                                          \n
*  ----------------------------------------------------------------------------\n
* 25-Feb-09    stein    61       Restructuration                             \n
* 18-Feb-09    kelbch   3        add target build version to HOST API structure \n
* 16-Feb-09    kelbch   2        Integration to host/target build\n
*******************************************************************************/

#if   !defined( CFR_CMBS_H )
#define  CFR_CMBS_H
#include "cmbs_api.h"               /* CMBS API definition */
#include "cfr_ie.h"                 /* CMBS IE handling */

#include "cfr_mssg.h"

#if defined ( WIN32 )
#include "windows.h"
#endif

/*!
   brief endianess
*/
typedef enum
{
   E_CMBS_ENDIAN_LITTLE,            /*!< little endian */
   E_CMBS_ENDIAN_BIG,               /*!< big endian */
   E_CMBS_ENDIAN_MIXED              /*!< mixed endian */
} E_CMBS_ENDIAN;


/*!
   \brief return value of DnA CMBS framework
*/
typedef enum
{
   CFR_E_RETVAL_OK,                 /*!< successful return value*/
   CFR_E_RETVAL_ERR,                /*!< general error occured */
   CFR_E_RETVAL_ERR_MEM,            /*!< not enough memory available */

} CFR_E_RETVAL;

/*! currently default value of target module */
#ifdef UART_DECT_CMBS_UART_BAUD_B115200
#define  CMBS_UART_BAUD B115200
#elif defined (UART_DECT_CMBS_UART_BAUD_B57600)
#define  CMBS_UART_BAUD B57600
#elif defined (UART_DECT_CMBS_UART_BAUD_B38400)
#define  CMBS_UART_BAUD B38400
#elif defined (UART_DECT_CMBS_UART_BAUD_B19200)
#define  CMBS_UART_BAUD B19200
#elif defined (UART_DECT_CMBS_UART_BAUD_B14400)
#define  CMBS_UART_BAUD B14400
#elif defined (UART_DECT_CMBS_UART_BAUD_B9600)
#define  CMBS_UART_BAUD B9600
#else
#error "Baud Rate Config is not supported!"
#endif

/*! UART state handling */
#define  CMBS_RCV_STATE_IDLE    0
#define  CMBS_RCV_STATE_SYNC    1
#define  CMBS_RCV_STATE_DATA    2


#define  CMBS_BUF_SIZE  1024        /*!< used message buffer size of each direction, specially used on target side */

/*!   \brief local application slot */
typedef struct
{
   void *            pv_AppRefHandle;        /*!< store application reference pointer */
   PFN_CMBS_API_CB   pFnAppCb;               /*!< store to be called function (reception of CMBS events)*/
   ST_CB_LOG_BUFFER  pFnCbLogBuffer;         /*!< storage for callback of log buffer */
   u16               u16_AppAPIVersion;      /*!< requested API version of application (further needed to get compatibility)*/
   ST_CFR_IE_LIST    st_TransmitterIEList;   /*!< IE list of transmit direction */
#if defined ( CMBS_API_TARGET )
   u8                u8_IEBuffers[1][CMBS_BUF_SIZE];  /*!< 1 buffer of a complete IE list (one buffer for each thread, max. 1 thread) */
#else
   u8                u8_IEBuffers[4][CMBS_BUF_SIZE];  /*!< 4 buffers of a complete IE list (one buffer for each thread, max. 4 threads) */
   u32               u32_ThreadIdArray[4];            /*!< Array to store thread id */
#endif // defined ( CMBS_API_TARGET )
} ST_CMBS_API_SLOT, * PST_CMBS_API_SLOT;

/*!
   brief enum of commuication flow control
*/
typedef enum
{
   E_CMBS_FLOW_STATE_GO,            /*!< transmitter/receiver works fine */
   E_CMBS_FLOW_STATE_STOP,          /*!< transmitter/receiver is busy, no packet transmission*/
   E_CMBS_FLOW_STATE_MAX
} E_CMBS_FLOW_STATE;

/*! \brief CMBS API Instance (local)*/
typedef  struct
{
   ST_CMBS_API_SLOT  st_ApplSlot;
   u16               u16_TargetVersion;      /*!< target CMBS API version */
   u32               u32_CallInstanceCount;
//#if !defined ( CMBS_API_TARGET )
   u16               u16_TargetBuild;        /*!< contains the build version of target side*/
//#endif
   E_CMBS_API_MODE   e_Mode;                 /*!< request CMBSMode */
   E_CMBS_FLOW_STATE e_OrigFlowState;        /*!< Originator transmission state */
   E_CMBS_FLOW_STATE e_DestFlowState;        /*!< Destination transmission state */
   E_CMBS_ENDIAN     e_Endian;               /*!< endianess: 0 = little */
#if defined( __linux__ )
   E_CMBS_DEVTYPE    eDevCtlType;            /*!< control device type properties*/
   int               fdDevCtl;               /*!< handle of such device */

   E_CMBS_DEVTYPE    eDevMediaType;          /*!< media device type properties */
   int               fdDevMedia;             /*!< handle of such device */

   pthread_t         serialThreadId;         /*!< thread ID of serial pump */
   pthread_t         callbThreadId;          /*!< thread ID of callback function */

   pthread_mutex_t   cond_Mutex;
   pthread_cond_t    cond_UnLock;

   int               msgQId;                 /*!< message queue information*/
#else
# if defined ( WIN32 )
	E_CMBS_DEVTYPE    eDevCtlType;            /*!< control device type properties*/
	E_CMBS_DEVTYPE    eDevMediaType;          /*!< media device type properties */
   HANDLE            h_InitBlock;            /*!< handle to wait until CMBS is connected */
	HANDLE			   h_RecPath;			      /*!< handle of synchronization path */
	HANDLE			   h_RecThread;	  	      /*!< handle of receive data collector thread */
	DWORD			   dw_ThreadID;
	BOOL			   bo_Run;
# endif
#endif

   CFR_CMBS_CRITICALSECTION
                     h_CriticalSectionTransmission;  /*!< Critical section for transmission of packets */

} ST_CMBS_API_INST, * PST_CMBS_API_INST;

/*!
   The following serialized message format shall be used:
      0    1    2    3      4    5      6    7      8    9      10   11
   | 0xda 0xda 0xda 0xda | 0xLO 0xHI | 0xLO 0xHI | 0xXX 0xYY | 0xLO 0xHI | parameter data
          sync.             total len   packet nr.   command     param len
*/
#define  CMBS_SYNC_LENGTH  4              /*!< Synchronization size */
#define  CMBS_SYNC          0xDADADADA    /*!< Synchronization value */

#define  CMBS_RCV_STATE_IDLE    0
#define  CMBS_RCV_STATE_SYNC    1
#define  CMBS_RCV_STATE_DATA    2

/*! \brief CMBS API Instance (local)*/
/*! \brief Serialization message header
    \note shall not use enumerator because then the size depends on compiler
         and its flags */
typedef  struct
{
   u16            u16_TotalLength;        /*!< total length without synchronisation dword */
   u16            u16_PacketNr;           /*!< packet number of message */
   u16            u16_EventID;            /*!< CMBS Event ID or internal Command ID */
   u16            u16_ParamLength;        /*!< Parameter length of followed IEs */
} ST_CMBS_SER_MSGHDR, * PST_CMBS_SER_MSGHDR;

/*! \brief Serialization message; Size = CMBS_BUF_SIZE */
typedef  struct
{
   ST_CMBS_SER_MSGHDR
                  st_MsgHdr;
   u8             u8_Param[CMBS_BUF_SIZE - sizeof(ST_CMBS_SER_MSGHDR)];
} ST_CMBS_SER_MSG, * PST_CMBS_SER_MSG;

/*! \brief Union representing a complete serial cmbs api message
   the union uses to work on the message with byte or structure access */
typedef  union
{
   char           serialBuf[sizeof(u32)+sizeof(ST_CMBS_SER_MSG)];
   struct
   {
      u32         u32_Sync;
      ST_CMBS_SER_MSG
                  st_Msg;
   }st_Data;
} U_CMBS_SER_DATA, * PU_CMBS_SER_DATA;

#define     CMBS_CMD_MASK  0xFF00
/*! \brief enumeration of internal used commands */
typedef enum
{
   CMBS_CMD_HELLO = 0x01,     /*!< Host send this command to target for registration*/
   CMBS_CMD_HELLO_RPLY,       /*!< Target replies on an Hello with this command */
   CMBS_CMD_FLOW_NOK,         /*!< The reception was not successful */
   CMBS_CMD_FLOW_RESTART,     /*!< Restart with contained packet number */
   CMBS_CMD_RESET,            /*!< Reset the communication between host and target */
} CMBS_CMD;

#if defined(__linux__)
typedef  struct
{
   int            nLength;
   u8             u8_Data[CMBS_BUF_SIZE + sizeof(u32)];
} ST_CMBS_LIN_MSGDATA, * PST_CMBS_LIN_MSGDATA;

typedef  struct
{
   long           msgType;
   ST_CMBS_LIN_MSGDATA
                  msgData;
} ST_CMBS_LIN_MSG, * PST_CMBS_LIN_MSG;
#endif

/*! \brief DECT message; Unsolicited, from target -> host */
typedef  struct
{
   u16            u16_EventID;
   void *         pv_Msg;
} ST_CMBS_DECT_MSG, * PST_CMBS_DECT_MSG;


/*! \brief IE structure for generic events */
typedef  struct
{
   u8       u8_SubType;
   u8       u8_MultiPlicity;
   u8       u8_LineId;
} ST_IE_GEN_EVENT, * PST_IE_GEN_EVENT;


#define CMBS_PROP_EVENT_LENGTH 20   // same as LA10_MAC_GENERIC_DATA

/*! \brief IE structure for proprietary events */
typedef  struct
{
   u16      u16_PropEvent;
   u8       u8_DataLen;
   u8       u8_Data[CMBS_PROP_EVENT_LENGTH];
} ST_IE_PROP_EVENT, * PST_IE_PROP_EVENT;


#if defined( __cplusplus )
extern "C"
{
#endif

extern ST_CMBS_API_INST g_CMBSInstance;

/*****************************************************************************
 * CMBS API Internal functions
 *****************************************************************************/

E_CMBS_ENDIAN     cmbs_int_EndiannessGet( void );
u16               cmbs_int_EndianCvt16( u16 u16_Value );
u32               cmbs_int_EndianCvt32( u32 u32_Value );

void              cmbs_int_HdrEndianCvt( ST_CMBS_SER_MSGHDR *pst_Hdr );

E_CMBS_RC         cmbs_int_ParamValid( E_CMBS_PARAM e_Param, u8 * pu8_Data, u16 u16_DataLen );
// OS and environment dependent function
E_CMBS_RC         cmbs_int_EnvCreate( E_CMBS_API_MODE e_Mode, ST_CMBS_DEV * pst_DevCtl, ST_CMBS_DEV * pst_DevMedia );
E_CMBS_RC         cmbs_int_EnvDestroy( void );  // Cleaning up. on target side EnvDestroy will never be executed
// start-up blocking function to wait until CMBS is available
void              _cmbs_int_StartupBlockSignal( PST_CMBS_API_INST pst_CMBSInst );

void *            cmbs_int_RegisterCb( void * pv_AppRef, PFN_CMBS_API_CB pfn_api_Cb, u16 u16_bcdVersion );
void              cmbs_int_RegisterLogBufferCb( void * pv_AppRef, PST_CB_LOG_BUFFER pfn_log_buffer_Cb );
void              cmbs_int_UnregisterCb( void * pv_AppRefHandle );

u16               cmbs_int_ModuleVersionGet( void );

E_CMBS_RC         cmbs_int_EventSend( E_CMBS_EVENT_ID e_EventID, u8 *pBuf, u16 u16_Length );
void              cmbs_int_EventReceive ( u8 * pu8_Mssg, u16 u16_Size );

void              cmbs_int_EventTrace ( u8 * pu8_Buffer, u16 u16_Size );
void              cmbs_int_CmdTrace ( u8 * pu8_Buffer, u16 u16_Size );

E_CMBS_RC         cmbs_int_ResponseSend( E_CMBS_EVENT_ID e_ID, E_CMBS_RESPONSE e_RSPCode );
E_CMBS_RC		cmbs_int_ResponseWithCallInstanceSend(	E_CMBS_EVENT_ID e_ID, 
															E_CMBS_RESPONSE e_RSPCode, 
															u32 u32CallInstance);


E_CMBS_RC         cmbs_int_cmd_Send( u8 u8_Cmd, u8 * pBuf, u16 u16_Length );
void              cmbs_int_cmd_Dispatcher( u8 u8_Cmd, u8 * pu8_Buffer, u16 u16_Size );
void              cmbs_int_cmd_ReceiveEarly ( u8 * pu8_buffer );

void              cmbs_int_cmd_FlowRestartHandle ( u16 u16_Packet );
void              cmbs_int_cmd_FlowNOKHandle ( u16 u16_Packet );
u8                cmbs_int_cmd_FlowStateGet ( void );

E_CMBS_RC         cmbs_int_OnHsRegistration( void );

char *            cmbs_int_Dbg_IE_Print( E_CMBS_IE_TYPE e_IE );

void              cmbs_int_ie_HandsetInfoAdd( void * pv_RefIEList, ST_IE_HANDSETINFO * pst_HandsetInfo );
void              cmbs_int_ie_ParameterAdd( void * pv_RefIEList, ST_IE_PARAMETER * pst_Param );
void              cmbs_int_ie_ParameterAreaAdd( void * pv_RefIEList, ST_IE_PARAMETER_AREA * pst_ParamArea );
void              cmbs_int_ie_ResponseAdd( void * pv_RefIEList, ST_IE_RESPONSE * pst_Response );
E_CMBS_RC         cmbs_int_ie_DisplayStringGet( void * pv_RefIE, ST_IE_DISPLAY_STRING * pst_DisplayString );
E_CMBS_RC         cmbs_int_ie_ToneGet( void * pv_RefIE, ST_IE_TONE * pst_Tone );
void              cmbs_int_ie_IntValueAdd( void * pv_RefIEList, u32 u32_Value );
E_CMBS_RC         cmbs_int_ie_IntValueGet( void * pv_RefIE, u32 * pu32_Value );
E_CMBS_RC         cmbs_int_ie_ByteValueAdd( void * pv_RefIEList, u8 u8_Value, E_CMBS_IE_TYPE e_IETYPE );
E_CMBS_RC         cmbs_int_ie_ByteValueGet( void * pv_RefIE, u8 * pu8_Value );
E_CMBS_RC         cmbs_int_ie_ShortValueAdd( void * pv_RefIEList, u16 u16_Value, E_CMBS_IE_TYPE e_IETYPE );
E_CMBS_RC         cmbs_int_ie_ShortValueGet( void * pv_RefIE, u16 * pu16_Value );
void              cmbs_int_ie_FwVersionAdd( void * pv_RefIEList, ST_IE_FW_VERSION * pst_FwVersion );
void              cmbs_int_ie_SysLogAdd( void * pv_RefIEList, ST_IE_SYS_LOG * pst_SysLog );
E_CMBS_RC         cmbs_int_ie_SysStatusSet( void * pv_RefIE, ST_IE_SYS_STATUS * pst_SysStatus );
void              cmbs_int_ie_GenEventAdd( void * pv_RefIEList, ST_IE_GEN_EVENT * pst_GenEvent );
E_CMBS_RC         cmbs_int_ie_GenEventGet( void * pv_RefIE, ST_IE_GEN_EVENT * pst_GenEvent );
void              cmbs_int_ie_PropEventAdd( void * pv_RefIEList, u16 u16_PropEvent, u8 * pu8_Data, u8 u8_DataLen );
E_CMBS_RC         cmbs_int_ie_PropEventGet( void * pv_RefIE, ST_IE_PROP_EVENT * pst_PropEvent );
E_CMBS_RC         cmbs_int_ie_DateTimeAdd( void * pv_RefIEList, ST_IE_DATETIME * pst_DateTime );
E_CMBS_RC         cmbs_int_ie_DataSessionTypeAdd( void * pv_RefIEList, ST_IE_DATA_SESSION_TYPE * pst_DataSessionType );
E_CMBS_RC         cmbs_int_ie_DataAdd( void * pv_RefIEList, ST_IE_DATA * pst_Data );
E_CMBS_RC         cmbs_int_ie_LAFieldsAdd( void * pv_RefIEList, ST_IE_LA_FIELDS * pst_LAFields, E_CMBS_IE_TYPE e_IEType );
E_CMBS_RC         cmbs_int_ie_LASearchCriteriaAdd( void * pv_RefIEList, ST_IE_LA_SEARCH_CRITERIA * pst_LASearchCriteria );
E_CMBS_RC         cmbs_int_ie_CallStateAdd( void * pv_RefIEList, ST_IE_CALL_STATE * pst_CallState );
E_CMBS_RC         cmbs_int_ie_GpioGet( void * pv_RefIE, u16 * pu16_Gpio );
E_CMBS_RC         cmbs_int_ie_GpioAdd( void * pv_RefIEList, u16 u16_Gpio );
E_CMBS_RC         cmbs_int_ie_ATESettingsGet( void * pv_RefIE, ST_IE_ATE_SETTINGS * pst_AteSettings );
E_CMBS_RC         cmbs_int_ie_ATESettingsAdd( void * pv_RefIEList, ST_IE_ATE_SETTINGS * pst_AteSettings );
E_CMBS_RC         cmbs_api_ie_ReadDirectionAdd( void * pv_RefIEList, ST_IE_READ_DIRECTION * pst_ReadDirection );
E_CMBS_RC         cmbs_api_ie_ReadDirectionGet( void * pv_RefIE, ST_IE_READ_DIRECTION * pst_ReadDirection );
E_CMBS_RC         cmbs_api_ie_MarkRequestAdd( void * pv_RefIEList, ST_IE_MARK_REQUEST * pst_MarkRequest );
E_CMBS_RC         cmbs_api_ie_MarkRequestGet( void * pv_RefIE, ST_IE_MARK_REQUEST * pst_MarkRequest );



#if defined( __cplusplus )
}
#endif

#endif   // API_INTERNAL_H

// EOF

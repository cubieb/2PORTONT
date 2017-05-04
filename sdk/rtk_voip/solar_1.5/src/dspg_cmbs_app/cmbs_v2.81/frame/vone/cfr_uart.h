/*!
*	\file			cfr_uart.h
*	\brief			
*	\Author		kelbch 
*
*	@(#)	%filespec: cfr_uart.h~DMZD53#7 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                          \n
*	----------------------------------------------------------------------------\n
* 14-Feb-09		kelbch		1		 Initialize\n
*******************************************************************************/

#if	!defined( CFR_UART_H )
#define	CFR_UART_H

#define		FALSE		0
#define		TRUE		1

#define		CFR_BUFFER_PACKET_WRAPSYNC		0xFFFF
#define		CFR_BUFFER_PACKET_HEADER_SIZE	2

/*!
   \brief   structure to handle a packet buffer inline
*/
typedef struct
{
	u16  u16_WRIDX;	    /*!< write index of buffer */
	u16  u16_RDIDX;	    /*!< read index of buffer */
	u16  u16_Size;        /*!< size of buffer */
	u8 * pu8_Buffer;      /*!< pointer to buffer */
                                 // packet window handler
   u16  u16_LastPackIDX; /*!< index of begin last packet */
   u8   u8_INPack;       /*!< number of submit packets*/
   u8   u8_OUTPack;      /*!< number of consumed packets */
   
} ST_CFR_BUFFER, * PST_CFR_BUFFER;


#define		CFR_BUFFERWRITE_SIZE	   512
#define		CFR_BUFFERREAD_SIZE	   800

#define     CFR_BUFFER_WINDOW_SIZE  15

#define		CFR_BUFFER_UART_REC		0
#define		CFR_BUFFER_UART_TRANS	1
#define		CFR_BUFFER_UART_NUMBERS 2

#define		CFR_UART_SYNCBUFFER_SIZE	6
typedef enum
{
	CFR_E_UART_IDLE,
	CFR_E_UART_PENDING_SIZE,
	CFR_E_UART_PENDING_PACKET,
	CFR_E_UART_MAX
} CFR_E_UART_STATE;

typedef struct
{
	u8				      u8_BufferWrite[CFR_BUFFERWRITE_SIZE];
	u8				      u8_BufferRead[CFR_BUFFERREAD_SIZE];
	ST_CFR_BUFFER	   st_Buffer[CFR_BUFFER_UART_NUMBERS];
	CFR_E_UART_STATE  e_ReceiveState;
	u8				      u8_ReceiveSyncCnt;
	u16				   u16_ReceivedLength;
	u8				      u8_ReceiveSync[CFR_UART_SYNCBUFFER_SIZE];
	u16				   u16_TransmitCnt;	
	
} ST_CFR_UARTHDL, * PST_CFR_UARTHDL;

#if defined( __cplusplus )
extern "C"
{
#endif
void	            cfr_uartInitalize( void );
int	    	      cfr_uartDataReceiveGet( u8 * pu8_Data, u16 u16_DataLength );

u8                cfr_uartDataTransmitOngoingCheck( void );

u8*			      cfr_uartDataTransmitGet( u16 * pu16_DataLength, u16 u16_Size );
void			      cfr_uartDataTransmitKick( void );

u8*	    	      cfr_uartPacketRead( u8 u8_BufferIDX, u16 * pu16_DataLength );
void			      cfr_uartPacketFree( u8 u8_BufferIDX );
CFR_E_RETVAL	   cfr_uartPacketWrite( u8 * pu8_Buffer, u16 u16_Size );

void		         cfr_uartPacketPartWrite( u8 * pu8_Buffer, u16 u16_Size );
CFR_E_RETVAL      cfr_uartPacketPrepare( u16 u16_size );

void              cfr_uartPacketWriteFinish( u8 u8_BufferIDX );
u8                cfr_uartPacketWindowReached( u8 u8_BufferIDX );
void              cfr_uartPacketTrace( u8 u8_BufferIDX );


#if defined( __cplusplus )
}
#endif

#endif	//	CFR_UART_H
//*/

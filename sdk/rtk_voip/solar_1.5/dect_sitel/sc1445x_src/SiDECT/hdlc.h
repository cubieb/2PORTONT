/****************************************************************************
*  Program/file: hdlc.h
*
*  DESCRIPTION: Exports hdlc function prototypes to other modules.
*
****************************************************************************/


#ifndef __HDLC_H__
#define __HDLC_H__

/* Switches */
#define SPI_BUS // Select SPI_BUS or RS232_BUS

/* Debug prints */
#define TIME_PRINT(args...) 	//printf(args)
#define DEBUG_PRINT(args...) 	printf(args)
#define D_PRINT(args...) 		//printf(args)
#define DUMP_TX_BYTES(args...)	 //printf(args)
#define DUMP_RX_BYTES(args...)	 //printf(args)
#define DUMP_TX_PRIM(args...)	//printf(args)
#define DUMP_RX_PRIM(args...)	//printf(args)
#define PRINT_MESSAGE(args...) 	printf(args)
#define PRINT_INFO(args...)  printf(args)

/*Typedefs*/
#define u16 unsigned short

/*Function Prototypes */
/* Exported by bus232.c*/
/*Function Prototypes */
/* Exported by bus.c*/

int Bus_Open(void);
int Bus_Close(void);
int Bus_Read_byte_blocking(unsigned char *Buf);
int Bus_Write_byte_blocking(unsigned char *Buf, unsigned short Len);	
int Bus_Seek_bits( unsigned short Len );

#define BUS_OPEN				Bus_Open
#define BUS_CLOSE				Bus_Close
#define BUS_READ_BYTE_BLOCKING	Bus_Read_byte_blocking
#define BUS_WRITE_BYTE_BLOCKING	Bus_Write_byte_blocking	

/* Exported by busSPI.c*/
void sendmail_ReceivePacket(unsigned short Length, unsigned char *MailPtr);

/* Indicate firmware is opened, so it does upgrade now. */
extern int FwuFileOpen;
extern void EnableSyncRxDLE( int bEnable );


#endif // __HDLC_H__

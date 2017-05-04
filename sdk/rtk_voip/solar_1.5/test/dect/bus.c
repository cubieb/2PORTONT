#include <stdio.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <string.h>
#include <fcntl.h>
#include "bus.h"

/****************************************************************************
*                     Enumerations/Type definitions/Structs
****************************************************************************/

/****************************************************************************
*                     Variable declarations
****************************************************************************/
int hComPortHandle = NULL;
#ifdef RS232_BUS
	struct termios oldtio, newtio;//used for storing old and new serial port configurations
#endif //RS232_BUS

/****************************************************************************
*                      Function Prototypes
****************************************************************************/
/*Defined here*/

/****************************************************************************
*                      Function declarations
****************************************************************************/
int Bus_Open(void)
{
#ifdef RS232_BUS
	/* 
	  Open modem device for reading and writing and not as controlling tty
	  because we don't want to get killed if linenoise sends CTRL-C.
	*/
	hComPortHandle = open(MODEMDEVICE, O_RDWR | O_NOCTTY /*| O_NONBLOCK*/); 
	if (hComPortHandle <0) {
		perror(MODEMDEVICE); 
		return hComPortHandle; 
	}
	tcgetattr(hComPortHandle,&oldtio); /* save current serial port settings */
	bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */


	/* 
	BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
	CRTSCTS : output hardware flow control (only used if the cable has
			all necessary lines. See sect. 7 of Serial-HOWTO)
	CS8     : 8n1 (8bit,no parity,1 stopbit)
	CLOCAL  : local connection, no modem contol
	CREAD   : enable receiving characters
	*/
	// newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;

	/*
	IGNPAR  : ignore bytes with parity errors
	ICRNL   : map CR to NL (otherwise a CR input on the other computer
			will not terminate input)
	otherwise make device raw (no other input processing)
	*/
	newtio.c_iflag = IGNPAR;// | ICRNL;

	/*
	Raw output.
	*/
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;//ICANON;

	/* 
	initialize all control characters 
	default values can be found in /usr/include/termios.h, and are given
	in the comments, but we don't need them here
	*/
	newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */ 
	newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
	newtio.c_cc[VERASE]   = 0;     /* del */
	newtio.c_cc[VKILL]    = 0;     /* @ */
	newtio.c_cc[VEOF]     = 0;//4;//4;     /* Ctrl-d */
	newtio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
	newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
	newtio.c_cc[VSWTC]    = 0;     /* '\0' */
	newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */ 
	newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
	newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
	newtio.c_cc[VEOL]     = 0;     /* '\0' */
	newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
	newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
	newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
	newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
	newtio.c_cc[VEOL2]    = 0;     /* '\0' */
	/* 
	now clean the modem line and activate the settings for the port
	*/
	tcflush(hComPortHandle, TCIFLUSH);
	tcsetattr(hComPortHandle,TCSANOW,&newtio);
    
	DEBUG_PRINT ( "RS232 bus opened \n" );
#endif// RS232_BUS	

#ifdef SPI_BUS	
	hComPortHandle = open(MODEMDEVICE, O_RDWR ); 
	if (hComPortHandle <0) {
		perror(MODEMDEVICE); 
		return hComPortHandle; 
	}
#endif// SPI_BUS	
	return 0;
}

/****************************************************************************
*  FUNCTION: 	Rs232_Read(unsigned char *Buf, rsuint16 Len)
*   INPUTS  :         Buf:  specifies the address of the buffer where to data that 
*                       	should be read from the bus. 
*  RETURNS :  	-1, on Error
*			The number of bytes read, on success
*  DESCRIPTION: This function is used for reading from the RS232 bus. NOTE: IT IS EXPECTED TO BLOCK !!
****************************************************************************/
int Bus_Read_byte_blocking(unsigned char *Buf)
{
	int result;
	
	result = read( hComPortHandle, Buf, 1);
#ifdef	DEBUG_BUS
	if ( result < 0){	
		printf("read from " MODEMDEVICE " failed");
	}
#endif	//DEBUG_BUS
	return result;
}

int Bus_Read_blocking(unsigned char *Buf, unsigned char len)
{
	int result;
	
	result = read( hComPortHandle, Buf, len);
#ifdef	DEBUG_BUS
	if ( result < 0){	
		printf("read from " MODEMDEVICE " failed");
	}
#endif	//DEBUG_BUS
	return result;
}

/****************************************************************************
*  FUNCTION: 	Rs232_Write(unsigned char *Buf, rsuint16 Len)
*   INPUTS  :         Buf:  specifies the address of the buffer where to data that 
*                       	should be written to the bus. 
*  RETURNS :  	-1, on Error
*			The number of bytes written, on success
*  DESCRIPTION: This function is used for writing to the RS232 bus. NOTE: IT IS EXPECTED TO BLOCK !!
****************************************************************************/
int Bus_Write_byte_blocking(unsigned char *Buf, unsigned short Len)
{
	int result;
	
	result = write( hComPortHandle, Buf, Len);
#ifdef	DEBUG_BUS	
	if ( result < 0){	
		printf("write to " MODEMDEVICE " failed");
	}
#endif //DEBUG_BUS	
	return result;
}
		
int Bus_Close(void)
{
  close(hComPortHandle);
  return 0;
}



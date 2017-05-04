/*!
*	\file			
*	\brief			
*	\Author		kelbch 
*
*	@(#)	%filespec: -1 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                          \n
*	----------------------------------------------------------------------------\n
*******************************************************************************
*	COPYRIGHT DOSCH & AMAND RESEARCH GMBH & CO.KG
*	DOSCH & AMAND RESEARCH GMBH & CO.KG Confidential
*
*******************************************************************************/

#if	!defined( CMBS_PLATF_H )
#define	CMBS_PLATF_H

#include <string.h>

#if defined( __linux__ )

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/time.h> //we need <sys/select.h>; should be included in <sys/types.h> ???
#include <signal.h>
#include <sys/msg.h>
#include <errno.h>

#ifdef _UNICODE
typedef wchar_t                  TCHAR;
#else
typedef char                     TCHAR;
#endif 

#elif defined(WIN32)

#include <windows.h>
#include <conio.h>

#elif defined(CMBS_API_TARGET)

#if   !defined( NULL )
#define	NULL	0
#endif

typedef	unsigned char				BYTE;
typedef	unsigned char *			PBYTE;
typedef	unsigned char *   		LPBYTE;
typedef	unsigned char *   		HPBYTE;
typedef	char							CHAR;
typedef	char *						PCHAR;
typedef	char *						PSTR;
typedef	char *   					LPSTR;
typedef	unsigned short 			WORD;
typedef	unsigned short * 			PWORD;
typedef	unsigned short *     	LPWORD;
typedef	unsigned short * 	      HPWORD;
typedef	short 						SHORT;
typedef	short * 						PSHORT;
typedef	unsigned long				DWORD;
typedef	unsigned long				ULONG;
typedef	unsigned long *			PDWORD;
typedef	long							LONG;
typedef	long *						PLONG;
typedef	unsigned int				UINT;
typedef	unsigned int *				PUINT;
typedef	int *							PINT;

#if	!defined( WDM ) && !defined( NT ) && !defined( VOID )
typedef	void							VOID;
#endif

typedef	void *						PVOID;
typedef	void  *					   LPVOID;
typedef	void *    					HPVOID;
typedef	int							BOOL;
typedef	int *							PBOOL;
typedef	void *						HANDLE;
typedef	HANDLE *						PHANDLE;

typedef unsigned char   u8;   /* unsigned int [0,255] */
typedef signed char     s8;   /* signed int [-128,127] */
typedef unsigned short  u16;  /* unsigned int [0,65535] */
typedef signed short    s16;  /* signed int [-32768,32767] */
typedef unsigned int    u32;  /* unsigned int [0,4294967295] */
typedef signed int      s32;  /* signed int [-2147483648, 2147483647] */

#if   !defined( FALSE )
#define  FALSE 0
#endif

#if   !defined( TRUE )
#define  TRUE  1
#endif

#if   !defined( MAX )
#define  MAX(a,b) ((a)>(b)?(a):(b))
#endif

#if   !defined( MIN )
#define  MIN(a,b) ((a)<(b)?(a):(b))
#endif

#if   !defined( LOBYTE )
#define  LOBYTE(a) ((a) & 0xff)
#endif 

#if   !defined( HIBYTE )
#define  HIBYTE(a) (((a) >> 8) & 0xff)
#endif 

#if   !defined( max )
#define  max(a,b) ((a)>(b)?(a):(b))
#endif

#if   !defined( min )
#define  min(a,b) ((a)<(b)?(a):(b))
#endif
#else
#     error Platform is not defined
#endif

#if defined( __cplusplus )
extern "C"
{
#endif

#if defined( __cplusplus )
}
#endif

#endif	//	CMBS_PLATF_H
//*/


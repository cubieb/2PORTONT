/* 
 *	debug.h -- debug header
 *
 */

#ifndef INCLUDE_DEBUG_H
#define INCLUDE_DEBUG_H

extern void OutputMessage(const char* szFormat, ...);
extern void OutputDebugMessage(unsigned long mode, const char* szFormat, ...);

#define STA_NONE	0x0
#define STA_NOTAG	0x00000001
#define STA_INFO	0x00000002
#define STA_SCRIPT	0x00000004
#define STA_WARNING	0x00000008
#define STA_ERR		0x00000010

#ifdef _DEBUG_MESSAGE

	extern void DebugAssert(char* szExpress, char* szFile, int nLine);
	extern void DebugVerify(char* szExpress, char* szFile, int nLine);
	extern void DebugMode(unsigned long mode);

	// note: __DEBUG__ is defined in debug.c
	#ifndef __DEBUG__

	#ifndef TRACE
		#define TRACE			OutputDebugMessage
	#endif	// TRACE

	#ifndef ASSERT
		#define ASSERT(x)		((x) ? (void)0 : DebugAssert(#x, __FILE__, __LINE__))
		#define VERIFY(x)		((x) ? (void)0 : DebugVerify(#x, __FILE__, __LINE__))
	#endif	// ASSERT
	
	#endif	// __DEBUG__

	#ifndef DEBUGMODE
		#define DEBUGMODE(x)		DebugMode(x)
	#endif	// DEBUGMODE

#else	// _DEBUG_MESSAGE

	#ifndef TRACE
		#define TRACE			1 ? (void)0 : OutputDebugMessage
	#endif	// TRACE
	
	#ifndef ASSERT
		#define ASSERT(x)		((void)0)
		#define VERIFY(x)		((void)(x))
	#endif	// ASSERT

	#ifndef DEBUGMODE	
		#define DEBUGMODE(x)
	#endif	// DEBUGMODE
	
#endif	// _DEBUG_MESSAGE

#endif


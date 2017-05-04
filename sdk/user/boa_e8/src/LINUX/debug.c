/* 
 *	debug.c -- debug code
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define __DEBUG__
#include "debug.h"

static unsigned long s_bDBGMode = STA_ERR;

#ifdef _DEBUG_MESSAGE

void DebugAssert(char* szExpress, char* szFile, int nLine)
{
	OutputMessage("\nAssertion fails at:\nFile: %s, line %d\n", szFile, nLine);
	OutputMessage("\n\t%s", szExpress);
	
	OutputMessage("\n\nProgram terminated.\n");
	exit(0);
}

void DebugVerify(char* szExpress, char* szFile, int nLine)
{
	OutputMessage("\nVerification fails at:\nFile: %s, line %d\n", szFile, nLine);
	OutputMessage("\n\t%s", szExpress);
	
	OutputMessage("\n\nProgram terminated.\n");
	exit(0);
}

void DebugMode(unsigned long mode)
{
	s_bDBGMode = mode;
}

#endif	// _DEBUG_MESSAGE

void OutputMessage(const char* szFormat, ...)
{
	va_list args;
	
	va_start(args, szFormat);
	vprintf(szFormat, args);
	va_end(args);
	fflush(NULL);
}

void OutputDebugMessage(unsigned long tap, const char* szFormat, ...)
{
	char* szType = NULL;
			
	unsigned long mode = tap & s_bDBGMode;
	if (mode)
	{
		switch(mode)
		{
		case STA_INFO:
			szType = "INFO";
			break;
        
		case STA_SCRIPT:
			szType = "CMD";
			break;
        
		case STA_WARNING:
			szType = "WARNING";
			break;
        
		case STA_ERR:
			szType = "ERROR";
			break;
			
		default:
			break;
		}
	}
	
	if (szType == NULL)
		return;
	
	if (!(tap & STA_NOTAG))
		printf("%6s: ", szType);
	
	va_list args;
	va_start(args, szFormat);
	vprintf(szFormat, args);
	va_end(args);
	
	fflush(NULL);
}


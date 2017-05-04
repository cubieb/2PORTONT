#include "vxWorks.h"
#include "stdarg.h"

#define DISP_BUF_SZ	16384
static char buf[256];
static char disp_buf[DISP_BUF_SZ];
static char *disp_idx = NULL;
static int  disp_cnt;



int dbg_printf(const char * fmt, ...)
{
int len;
va_list args;

	va_start(args,fmt);
	len = vsprintf(buf, fmt, args);
	va_end(args);
	if(len<(DISP_BUF_SZ-disp_cnt))
	{	
		if(disp_idx==NULL)
			disp_idx = 	disp_buf;
		memcpy(disp_idx, buf, len);
		disp_idx += len;
		disp_cnt += len;
		return(len);
	}
	return 0;
}

/**********************************************************************
* FUNCTION: pppoeDebugGet
* ARGUMENTS:
* 	out_buf	-- output buffer pointer
*	len		-- max length of output buffer
* RETURNS: output length
* DESCRIPTION: Get pppoe debug message
***********************************************************************/
int pppoeDebugGet(char *out_buf, int len)
{
	if(len<0)
		return -1;
	if(len == 0)
		len = disp_cnt;
	if(len > disp_cnt)
		len = disp_cnt;
	memcpy(out_buf, disp_buf, len);
	disp_idx = disp_buf;
	disp_cnt = 0;
	return len;
}


/* debug function */
#if 1
/**********************************************************************
* FUNCTION: pppoeDebugGet
* ARGUMENTS:
* 	out_buf	-- output buffer pointer
*	len		-- max length of output buffer
* RETURNS: N/A
* DESCRIPTION: Flush pppoe debug message to standard output
***********************************************************************/
void pppoeDebugFlush(void)
{
char *disp_ch;
	disp_ch = disp_buf;
	while(disp_cnt)
	{
		putchar(*disp_ch);
		disp_ch++;
		disp_cnt--;
	}
	disp_idx = disp_buf;
}

#endif

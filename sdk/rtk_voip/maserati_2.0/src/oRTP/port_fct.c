
/*
  The oRTP library is an RTP (Realtime Transport Protocol - rfc1889) stack.
  Copyright (C) 2001  Simon MORLAT simon.morlat@linphone.org

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <unistd.h>
#include "rtpport.h"
#include "payloadtype.h"


static char * g_strdup_vprintf(const char *fmt, va_list ap)
{
	/* Guess we need no more than 100 bytes. */
	int n, size = 100;
	char *p;
	if ((p = g_malloc (size)) == NULL)
		return NULL;
	while (1)
	{
		/* Try to print in the allocated space. */
		n = vsnprintf (p, size, fmt, ap);
		/* If that worked, return the string. */
		if (n > -1 && n < size)
			return p;
		//printf("Reallocing space.\n");
		/* Else try again with more space. */
		if (n > -1)	/* glibc 2.1 */
			size = n + 1;	/* precisely what is needed */
		else		/* glibc 2.0 */
			size *= 2;	/* twice the old size */
		if ((p = g_realloc (p, size)) == NULL)
			return NULL;
	}
}	



extern void ortp_log(const gchar *log_domain,GLogLevelFlags log_level,
                                           const gchar *message,
                                           gpointer user_data);

static GLogFunc __log_func=ortp_log;
static gpointer __log_user_data=(gpointer)NULL;

void g_log(const gchar *log_domain,GLogLevelFlags log_level,const gchar *format,...){
	va_list args;
	va_start(args,format);
	g_logv(log_domain,log_level,format,args);
	va_end(args);
}

void g_logv(const gchar *log_domain,GLogLevelFlags log_level,const gchar *format,va_list args){
	gchar *msg;
	msg=g_strdup_vprintf(format,args);
	__log_func(log_domain,log_level,msg,__log_user_data);
	g_free(msg);
}

void g_log_set_handler(const gchar *log_domain,GLogLevelFlags log_levels, GLogFunc log_func, gpointer user_data){
	__log_func=log_func;
	__log_user_data=user_data;
}

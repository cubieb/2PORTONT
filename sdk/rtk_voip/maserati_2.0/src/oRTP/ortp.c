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

#include "ortp.h"
#include <stdlib.h>
#include "../config.h"
#include "payloadtype.h"

extern void av_profile_init( RtpProfile * profile ) ;
extern void av_profile_destroy( RtpProfile * profile ) ;

gboolean initialized = FALSE ;

void ortp_init()
{
	if (initialized) return;
	initialized = TRUE ;

	av_profile_init( &av_profile ) ;
}

void ortp_destroy()
{
	initialized = FALSE;
	av_profile_destroy( &av_profile ) ;
}

void ortp_log(const gchar *log_domain,GLogLevelFlags log_level,
                                           const gchar *message,
                                           gpointer user_data)
{
	gchar *lev;
	switch(log_level){
		case G_LOG_LEVEL_MESSAGE:
			lev="message";
			break;
		case G_LOG_LEVEL_WARNING:
			lev="warning";
			break;
		case G_LOG_LEVEL_ERROR:
			lev="error";
		default:
			lev="(unknown log type)";
	}
	if (user_data==NULL){
		user_data=stdout;
	}
	fprintf((FILE*)user_data,"%s-%s:%s\n",log_domain,lev,message);
}

/*
  The mediastreamer library aims at providing modular media processing and I/O
	for linphone, but also for any telephony application.
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


#ifndef MEDIASTREAM_H
#define MEDIASTREAM_H

#include "ms.h"
#include "mscodec.h"
#include "msfilter.h"
#include "../oRTP/payloadtype.h"
#include "../linphone/uglib.h"


#if 0
/* start a thread that does sampling->encoding->rtp_sending|rtp_receiving->decoding->playing */
gboolean audio_stream_start_with_dsp( guint32 chid , guint32 ssid , RtpProfile * prof , int locport , char * remip4 ,
									int remport , int profile , int jitt_comp ) ;

/* send a dtmf */
gint audio_stream_send_dtmf (gboolean stream, gchar dtmf);
#endif

#endif

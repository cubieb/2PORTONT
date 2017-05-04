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

#ifndef MSCODEC_H
#define MSCODEC_H

#include "msfilter.h"

struct _MSCodecInfo
{
	MSFilterInfo info;
	gint rate;		/*frequency */
	gint pt;			/* the payload type number associated with this codec*/
	gchar *description;		/* a rtpmap field to describe the codec */
};

typedef struct _MSCodecInfo MSCodecInfo;

MSCodecInfo * ms_audio_codec_info_get(gchar *name);

/* register all statically linked codecs */
void ms_codec_register_all(void);
void ms_codec_unregister_all(void);

#define MS_CODEC_INFO(codinfo)	((MSCodecInfo*)codinfo)

#endif

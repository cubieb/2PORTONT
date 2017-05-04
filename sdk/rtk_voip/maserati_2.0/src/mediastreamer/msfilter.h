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


#ifndef MSFILTER_H
#define MSFILTER_H

#undef VERSION
#undef PACKAGE
#include "../linphone/uglib.h"
#include <string.h>

enum _MSFilterType
{
	MS_FILTER_DISK_IO,
	MS_FILTER_AUDIO_CODEC,
	MS_FILTER_NET_IO,
	MS_FILTER_AUDIO_IO,
	MS_FILTER_OTHER
};

typedef enum _MSFilterType MSFilterType;


struct _MSFilterInfo
{
	gchar *name;
	MSFilterType type;
};

typedef struct _MSFilterInfo MSFilterInfo;

void ms_filter_register(MSFilterInfo *finfo);

extern GList *filter_list;
#define MS_FILTER_INFO(obj)	((MSFilterInfo*)obj)

#endif

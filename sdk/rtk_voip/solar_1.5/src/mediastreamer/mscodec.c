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


#include "mscodec.h"

MSCodecInfo ALAWinfo={
	{
		"ALAW codec" ,
		MS_FILTER_AUDIO_CODEC ,
	} ,
	8000 ,
	8 ,
	"PCMA" ,
} ;

MSCodecInfo MULAWinfo={
	{
		"MULAW codec" ,
		MS_FILTER_AUDIO_CODEC ,
	} ,
	8000 ,
	0 ,
	"PCMU" ,
} ;

MSCodecInfo G7231info={
	{
		"G7231 codec" ,
		MS_FILTER_AUDIO_CODEC ,
	} ,
	6300 ,
	4 ,
	"G723" ,
} ;

MSCodecInfo G729info={
	{
		"G729 codec" ,
		MS_FILTER_AUDIO_CODEC ,
	} ,
	8000 ,
	18 ,
	"G729" ,
} ;

MSCodecInfo G726_16info={
	{
		"G726-16 codec" ,
		MS_FILTER_AUDIO_CODEC ,
	} ,
	8000 ,
	23 ,
	"G726-16" ,
} ;

MSCodecInfo G726_24info={
	{
		"G726-24 codec" ,
		MS_FILTER_AUDIO_CODEC ,
	} ,
	8000 ,
	22 ,
	"G726-24" ,
} ;

MSCodecInfo G726_32info={
	{
		"G726-32 codec" ,
		MS_FILTER_AUDIO_CODEC ,
	} ,
	8000 ,
	2 ,
	"G726-32" ,
} ;

MSCodecInfo G726_40info={
	{
		"G726-40 codec" ,
		MS_FILTER_AUDIO_CODEC ,
	} ,
	8000 ,
	21 ,
	"G726-40" ,
} ;

/* register all statically linked codecs */
void ms_codec_register_all()
{
	ms_filter_register( MS_FILTER_INFO( &G7231info ) ) ;
	ms_filter_register( MS_FILTER_INFO( &G729info ) ) ;
	ms_filter_register(MS_FILTER_INFO(&MULAWinfo));
	ms_filter_register(MS_FILTER_INFO(&ALAWinfo));

	ms_filter_register(MS_FILTER_INFO(&G726_16info));
	ms_filter_register(MS_FILTER_INFO(&G726_24info));
	ms_filter_register(MS_FILTER_INFO(&G726_32info));
	ms_filter_register(MS_FILTER_INFO(&G726_40info));
}

void ms_codec_unregister_all()
{
	g_list_free(filter_list);
	filter_list = NULL;
}

MSCodecInfo * ms_audio_codec_info_get(gchar *name)
{
	GList *elem=filter_list;
	MSFilterInfo *info;
	while (elem!=NULL)
	{
		info=(MSFilterInfo *)elem->data;
		if ( (info->type==MS_FILTER_AUDIO_CODEC) ){
			MSCodecInfo *codinfo=(MSCodecInfo *)info;
			if (strcmp(codinfo->description,name)==0){
				return MS_CODEC_INFO(info);
			}
		}
		elem=g_list_next(elem);
	}
	return NULL;
}

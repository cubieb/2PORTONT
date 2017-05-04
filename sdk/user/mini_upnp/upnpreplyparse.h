/* $Id: upnpreplyparse.h,v 1.1 2007/05/15 14:34:17 yachang Exp $ */
/* miniupnp project
 * see http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2005-2006 Thomas Bernard
 * This software is subjects to the conditions detailed in the LICENCE
 * file provided with this distribution */
#ifndef __UPNPREPLYPARSE_H__
#define __UPNPREPLYPARSE_H__

#if defined(sun) || defined(__sun) || defined(WIN32)
#include "bsdqueue.h"
#else
#include <sys/queue.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct NameValue {
    LIST_ENTRY(NameValue) entries;
    char name[64];
    char *value;
};

struct NameValueParserData {
    LIST_HEAD(listhead, NameValue) head;
    char curelt[64];
};

void
ParseNameValue(const char * buffer, int bufsize,
               struct NameValueParserData * data);

void
ClearNameValueList(struct NameValueParserData * pdata);

char *
GetValueFromNameValueList(struct NameValueParserData * pdata,
                          const char * Name);

char *
GetValueFromNameValueListIgnoreNS(struct NameValueParserData * pdata,
                                  const char * Name);

void
DisplayNameValueList(char * buffer, int bufsize);

char *
mini_UPnPGetFirstElement(char *file_start, unsigned int len, 
		char *tag_name, unsigned int tag_name_len);

char *
mini_UPnPGetFirstElementAndReturnAddr(char *file_start, unsigned int len, char *tag_name, unsigned int tag_name_len, char *buf);

char *
mini_UPnP_UploadXML(char *file_path);

char *
get_token(char *data, char *token);

int 
get_value(char *data, char *value);
#ifdef __cplusplus
}
#endif

#endif


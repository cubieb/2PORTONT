/* $Id: localupnphttp.h,v 1.4 2008/05/26 12:14:06 adsmt Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef __UPNPHTTP_H__
#define __UPNPHTTP_H__

#include <netinet/in.h>
#include <sys/queue.h>

#include "config.h"

/* server: HTTP header returned in all HTTP responses : */
#define MINIUPNPD_SERVER_STRING	OS_VERSION " UPnP/1.0 miniupnpd/1.0"

/*
 states :
  0 - waiting for data to read
  1 - waiting for HTTP Post Content.
 ...
 >= 100 - to be deleted
*/
enum httpCommands {
	EUnknown = 0,
	EGet,
	EPost,
	ESubscribe,
	EUnSubscribe
};

struct upnphttp {
	int socket;
struct in_addr clientaddr;	/* client address */
	/* Data structure added by Sean --begin-- */
	struct _soapMethods *soapMethods;
	struct _sendDesc *sendDesc;
	//struct upnp_subscription_record *subscribe_list;
	/* Data structure added by Sean --end-- */
	int state;
	char HttpVer[16];
	/* request */
	char * req_buf;
	int req_buflen;
	int req_contentlen;
	int req_contentoff;     /* header length */
	enum httpCommands req_command;
	char * req_soapAction;
	int req_soapActionLen;
#ifdef ENABLE_EVENTS
	const char * req_Callback;	/* For SUBSCRIBE */
	int req_CallbackLen;
	int req_Timeout;
	const char * req_SID;		/* For UNSUBSCRIBE */
	int req_SIDLen;
#endif
	/* response */
	char * res_buf;
	int res_buflen;
	int res_buf_alloclen;
	/*int res_contentlen;*/
	/*int res_contentoff;*/		/* header length */
	LIST_ENTRY(upnphttp) entries;
};

//cathy
struct _soapMethods {
	const char * serviceName;//cathy
	char * methodName;
	void (*methodImpl)(struct upnphttp *h);
};

struct _sendDesc {
	char * DescName;
	union {
		char * (*sendDescImpl)(int *len);
		void (*sendDescImpl_h)(struct upnphttp *h);
	}Impl;
	unsigned char type;
};

struct upnphttp * New_upnphttp(int);

void CloseSocket_upnphttp(struct upnphttp *);

void Delete_upnphttp(struct upnphttp *);

void Process_upnphttp(struct upnphttp *);

/* Build the header for the HTTP Response
 * Also allocate the buffer for body data */
void
BuildHeader_upnphttp(struct upnphttp * h, int respcode,
                     const char * respmsg,
                     int bodylen);

/* BuildResp_upnphttp() fill the res_buf buffer with the complete
 * HTTP 200 OK response from the body passed as argument */
void BuildResp_upnphttp(struct upnphttp *, const char *, int);

/* BuildResp2_upnphttp()
 * same but with given response code/message */
void
BuildResp2_upnphttp(struct upnphttp * h, int respcode,
                    const char * respmsg,
                    const char * body, int bodylen);

void SendResp_upnphttp(struct upnphttp *);

extern int OpenAndConfHTTPSocket(unsigned short port);
#endif


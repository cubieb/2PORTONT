#ifndef _CWMP_HTTPSERVER_H_
#define _CWMP_HTTPSERVER_H_

#include "soapH.h"
#include <config/autoconf.h>

int cwmp_webserver_init( struct soap *web_soap, void *data );

#ifdef CONFIG_MIDDLEWARE
void *cwmp_webserver_loop( void *data );
#else
int cwmp_webserver_loop( struct soap *web_soap, void *data);
#endif	//end of CONFIG_MIDDLEWARE

#endif /*_CWMP_HTTPSERVER_H_*/

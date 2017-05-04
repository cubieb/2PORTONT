/*
  eXosip - This is the eXtended osip library.
  Copyright (C) 2002, 2003  Aymeric MOIZARD  - jack@atosc.org
  
  eXosip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  eXosip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "eXosip2.h"
#include "../linphone/uglib.h"

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
kkkkkkkkkkkkkkk
#include <windows.h>
#include <Iphlpapi.h>
#endif

#ifdef RTK_DNS
#include "dns_api.h"
#endif

extern eXosip_t eXosip;

/* Private functions */
static int dialog_fill_route_set(osip_dialog_t *dialog,
				 osip_message_t *request);

/* should use cryptographically random identifier is RECOMMENDED.... */
/* by now this should lead to identical call-id when application are
   started at the same time...   */
char *
osip_call_id_new_random()
{
  char *tmp = (char *)osip_malloc(33);
  unsigned int number = osip_build_random_number();
  sprintf(tmp,"%u",number);
  return tmp;
}

char *
osip_from_tag_new_random(void)
{
  return osip_call_id_new_random();
}

char *
osip_to_tag_new_random(void)
{
  return osip_call_id_new_random();
}

unsigned int
via_branch_new_random(void)
{
  return osip_build_random_number();
}

int remove_url_port(const char *orig_url, char *new_url)
{
	int i;
	char *new_from;
	osip_from_t *a_from;

	i = osip_from_init(&a_from);
	if (i != 0)
    	return -1;

	i = osip_from_parse(a_from, orig_url);
	if (i != 0)
	{
		osip_from_free(a_from);
		return -1;
	}

	if (a_from->url->port)
	{
		osip_free(a_from->url->port);
		a_from->url->port = NULL;
	}

	osip_from_to_str(a_from, &new_from);
	if (new_from == NULL)
	{
		osip_from_free(a_from);
		return -1;
	}

	strcpy(new_url, new_from);
	osip_free(new_from);
	osip_from_free(a_from);
	return 0;
}

/* prepare a minimal request (outside of a dialog) with required headers */
/* 
   method_name is the type of request. ("INVITE", "REGISTER"...)
   to is the remote target URI
   transport is either "TCP" or "UDP" (by now, only UDP is implemented!)
*/
#ifdef MALLOC_DEBUG
#define generating_request_out_of_dialog(n, d, m, to, t, from,  r, reg)		\
		__generating_request_out_of_dialog(n, d, m, to, t, from, r, reg, __FILE__, __LINE__)
static int
__generating_request_out_of_dialog(int nPort, osip_message_t **dest, char *method_name,
				 char *orig_to, char *transport, char *orig_from,
				 osip_list_t *routes,
				 char *reg_proxy, char *file, int line)
#else
static int
generating_request_out_of_dialog(int nPort, osip_message_t **dest, char *method_name,
				 char *orig_to, char *transport, char *orig_from,
				 osip_list_t *routes,
				 char *reg_proxy)
#endif
{
  /* Section 8.1:
     A valid request contains at a minimum "To, From, Call-iD, Cseq,
     Max-Forwards and Via
  */
  int i;
  osip_message_t *request;
#ifdef SM
  char *locip=NULL;
#else
aaaaaaaaaaaaaaaaaaaaa
  char locip[50];
#endif
  int doing_register;
  char *register_callid_number = NULL;
  osip_call_id_t *callid;
  char tmp[200] ;
  char from[200], to[200];
  int pos;
  osip_route_t *route;
  char *request_str, *tmproute;

  // remove port in "from" according table 1 of RFC3261
  if (remove_url_port(orig_from, from) != 0)
    return -1;

  if (orig_to && remove_url_port(orig_to, to) != 0)
    return -1;

  i = osip_message_init(&request);
  if (i!=0) return -1;

  /* prepare the request-line */
  osip_message_set_method(request, osip_strdup(method_name));
  osip_message_set_version(request, osip_strdup("SIP/2.0"));
  osip_message_set_status_code(request, 0);
  osip_message_set_reason_phrase(request, NULL);

  doing_register = 0==strcmp("REGISTER", method_name);

	if (doing_register)
	{
		osip_uri_init(&(request->req_uri));
		i = osip_uri_parse(request->req_uri, reg_proxy);
		if (i!=0)
			goto brood_error_1;

      	osip_message_set_to(request, from);
	}
	else
	{
		/* in any cases except REGISTER: */
		i = osip_message_set_to(request, orig_to);
		if (i!=0)
		{
			OSIP_TRACE (osip_trace(__FILE__, __LINE__, OSIP_ERROR, NULL,
				"ERROR: callee address does not seems to be a sipurl: %s\n", orig_to));
			goto brood_error_1;
		}

		i = osip_uri_clone(request->to->url, &(request->req_uri));
		if (i!=0) 
			goto brood_error_1;
    
		// remove port in "to" according table 1 of RFC3261
	    osip_to_free(request->to);
    	request->to = NULL;
	    osip_message_set_to(request, to);
	}

	// check route set
	pos = 0;
	route = NULL;
	while (!osip_list_eol(routes, pos))
	{
		/* equal to a pre-existing route set */
		osip_uri_param_t *lr_param;

		if (osip_routes_get(routes, pos, &route) != pos)
		{
			pos++;
			continue;
		}

		osip_uri_uparam_get_byname(route->url, "lr", &lr_param);
	  	if (lr_param)
		{
			OSIP_TRACE (osip_trace(__FILE__, __LINE__, OSIP_WARNING, NULL,
				"ERROR: pre-existing route shoud be strict route\n"));
			pos++;
			continue;
		}

		// first route found
		break;
	}

	if (route)
	{
		osip_uri_param_t *lr_param;

    	/* if the first URI of route set does not contain "lr", the req_uri
		is set to the first uri of route set */
		
		osip_uri_uparam_get_byname(request->req_uri, "lr", &lr_param);
		if (lr_param == NULL)
			osip_uri_uparam_add(request->req_uri, osip_strdup("lr"), NULL);

		osip_uri_to_str(request->req_uri, &request_str);
      	osip_uri_free(request->req_uri);
		request->req_uri = NULL;
		osip_uri_clone(route->url, &(request->req_uri));

	    /* "The UAC MUST add a route header field containing
		the remainder of the route set values in order. */
		pos++;
		for (;!osip_list_eol(routes, pos); pos++)
		{
			if (osip_routes_get(routes, pos, &route) != pos)
				continue;

			/* check if the lr parameter is set , if not add it */
			osip_uri_uparam_get_byname(route->url, "lr", &lr_param);
		  	if (lr_param == NULL)
				osip_uri_uparam_add(route->url, osip_strdup("lr"), NULL);

			if (osip_route_to_str(route, &tmproute) != 0)
				continue;

			osip_message_set_route(request, tmproute);
			osip_free(tmproute);
		}
		
		/* The UAC MUST then place the remote target URI into
		the route header field as the last value. */
		osip_message_set_route(request, request_str);
		osip_free(request_str);
	}

#if 0

  if (doing_register)
    {
      osip_uri_init(&(request->req_uri));
      i = osip_uri_parse(request->req_uri, reg_proxy);
      if (i!=0)
	{
	  goto brood_error_1;
	}
      osip_message_set_to(request, from);
    }
  else
    {
      /* in any cases except REGISTER: */
      i = osip_message_set_to(request, orig_to);
      if (i!=0)
	{
     OSIP_TRACE (osip_trace
		 (__FILE__, __LINE__, OSIP_ERROR, NULL,
	     "ERROR: callee address does not seems to be a sipurl: %s\n", to));
	  goto brood_error_1;
	}
      if (proxy!=NULL && proxy[0] != 0)
	{  /* equal to a pre-existing route set */
	   /* if the pre-existing route set contains a "lr" (compliance
	      with bis-08) then the req_uri should contains the remote target
	      URI */
	  osip_uri_param_t *lr_param;
	  osip_route_t *o_proxy;
	  osip_route_init(&o_proxy);
	  i = osip_route_parse(o_proxy, proxy);
	  if (i!=0) {
	    osip_route_free(o_proxy);
	    goto brood_error_1;
	  }

	  osip_uri_uparam_get_byname(o_proxy->url, "lr", &lr_param);
	  if (lr_param!=NULL) /* to is the remote target URI in this case! */
	    {
	      osip_uri_clone(request->to->url, &(request->req_uri));
	      /* "[request] MUST includes a Route header field containing
	       the route set values in order." */
	      osip_list_add(request->routes, o_proxy, 0);
	    }
	  else
	    /* if the first URI of route set does not contain "lr", the req_uri
	       is set to the first uri of route set */
	    {
	      request->req_uri = o_proxy->url;
	      o_proxy->url = NULL;
	      osip_route_free(o_proxy);
	      /* add the route set */
	      /* "The UAC MUST add a route header field containing
		 the remainder of the route set values in order.
		 The UAC MUST then place the remote target URI into
		 the route header field as the last value
	       */
	      osip_message_set_route(request, to);
	    }
	}
      else /* No route set (outbound proxy) is used */
	{
	  /* The UAC must put the remote target URI (to field) in the req_uri */
	    i = osip_uri_clone(request->to->url, &(request->req_uri));
	    if (i!=0) goto brood_error_1;
	}
#if 1  
    // remove port in "to" according table 1 of RFC3261
    osip_to_free(request->to);
    request->to = NULL;
    osip_message_set_to(request, to);
#endif	
    }

#endif

  /*guess the local ip since req uri is known */
#ifdef SM
  eXosip_get_localip_for(request->req_uri->host,&locip);
#else
bbbbbbbbbbbbbbbb
  eXosip_guess_ip_for_via(eXosip.ip_family, locip, 49);
#endif
  /* set To and From */
  osip_message_set_from(request, from);
  /* add a tag */
  osip_from_set_tag(request->from, osip_from_tag_new_random());
  /* set the cseq and call_id header */
    {
      osip_cseq_t *cseq;
      char *num;
      char  *cidrand;

      /* call-id is always the same for REGISTRATIONS */
      i = osip_call_id_init(&callid);
      if (i!=0) goto brood_error_1;
      cidrand = osip_call_id_new_random();
      osip_call_id_set_number(callid, cidrand);
      if (doing_register)
	register_callid_number = cidrand;
     //osip_call_id_set_host(callid, osip_strdup(locip));
      //request->call_id = callid;

      i = osip_cseq_init(&cseq);
      if (i!=0) goto brood_error_1;
      num = osip_strdup(doing_register ? "1" : "20" );
      osip_cseq_set_number(cseq, num);
      osip_cseq_set_method(cseq, osip_strdup(method_name));
      request->cseq = cseq;
    }

	/* always add the Max-Forward header */
	osip_message_set_max_forwards( request , "70" ) ;	/* a UA should start a request with 70 */

	/* should be useless with compliant UA */
	if (eXosip.j_firewall_ip[nPort][0] != '\0')
	{
		char * c_address = request->req_uri->host ;
		struct addrinfo * addrinfo ;
		struct __eXosip_sockaddr addr ;

		i = eXosip_get_addrinfo(&addrinfo, request->req_uri->host, 5060);
		if( i == 0 )
		{
			memcpy( &addr , addrinfo->ai_addr , addrinfo->ai_addrlen ) ;
			freeaddrinfo( addrinfo ) ;
			c_address = inet_ntoa(((struct sockaddr_in *)((void *)&addr))->sin_addr ) ;
			g_message( "eXosip: here is the resolved destination host=%s\n" , c_address ) ;
		}

		if( eXosip_is_public_address( c_address ) )
		{
			char tmp[200] ;
			memset(tmp, 0, sizeof(tmp));
			snprintf( tmp , 200 , "SIP/2.0/%s %s:%s;branch=z9hG4bK%u",
					transport , eXosip.j_firewall_ip[nPort] , eXosip.j_firewall_ports[nPort] , via_branch_new_random() ) ;
			osip_message_set_via( request , tmp ) ;
			/* Mandy add for stun support */
			#if 1
			osip_call_id_set_host(callid, osip_strdup(eXosip.j_firewall_ip[nPort]));
			request->call_id = callid;
			#endif
			/* End of Mandy add for stun support */
		}
		else
		{
			memset(tmp, 0, sizeof(tmp));
			if( eXosip.ip_family == AF_INET6 )
				snprintf( tmp , 200 , "SIP/2.0/%s [%s]:%s;branch=z9hG4bK%u" ,
						transport , locip , eXosip.localports[nPort] , via_branch_new_random() ) ;
			else
				snprintf( tmp , 200 , "SIP/2.0/%s %s:%s;rport;branch=z9hG4bK%u" ,
						transport , locip , eXosip.localports[nPort] , via_branch_new_random() ) ;
			osip_message_set_via( request , tmp ) ;
			/* Mandy add for stun support */
			#if 1
			osip_call_id_set_host(callid, osip_strdup(locip));
			request->call_id = callid;
			#endif
			/* End of Mandy add for stun support */
		}
	}
	else
	{
		memset(tmp, 0, sizeof(tmp));
		if( eXosip.ip_family == AF_INET6 )
			snprintf( tmp , 200 , "SIP/2.0/%s [%s]:%s;branch=z9hG4bK%u" ,
					transport , locip , eXosip.localports[nPort] , via_branch_new_random() ) ;
		else
			snprintf( tmp , 200 , "SIP/2.0/%s %s:%s;rport;branch=z9hG4bK%u" ,
				transport , locip , eXosip.localports[nPort] , via_branch_new_random() ) ;
		osip_message_set_via( request , tmp ) ;
			/* Mandy add for stun support */
			#if 1
			#ifdef MALLOC_DEBUG
			osip_call_id_set_host(callid, __osip_strdup(locip, file, line));
			#else
			osip_call_id_set_host(callid, osip_strdup(locip));
			#endif
			request->call_id = callid;
			#endif
	}

	/* add specific headers for each kind of request... */
	if( 0 == strcmp( "INVITE" , method_name ) || 0 == strcmp( "SUBSCRIBE" , method_name ) )
	{
		osip_from_t * a_from ;
		int i ;

		i = osip_from_init( &a_from ) ;
		if( i == 0 )
			i = osip_from_parse( a_from , from ) ;

		if (i == 0 && a_from != NULL && a_from->url != NULL)
		{
			char contact_addr[100];
			char contact[200];

			if (eXosip.j_firewall_ip[nPort][0] != '\0')
			{
				char * c_address = request->req_uri->host ;
				struct addrinfo * addrinfo ;
				struct __eXosip_sockaddr addr ;

				i = eXosip_get_addrinfo(&addrinfo, request->req_uri->host, 5060);
				if( i == 0 )
				{
					memcpy( &addr , addrinfo->ai_addr , addrinfo->ai_addrlen ) ;
					freeaddrinfo( addrinfo ) ;
					c_address = inet_ntoa(((struct sockaddr_in *)((void *)&addr))->sin_addr ) ;
					g_message( "eXosip: here is the resolved destination host=%s\n" , c_address ) ;
				}

				if( eXosip_is_public_address( c_address ) )
				{
				  	if( eXosip.j_firewall_ports[nPort] == NULL )
						sprintf(contact_addr, "%s", eXosip.j_firewall_ip[nPort]);
					else
						sprintf(contact_addr, "%s:%s", eXosip.j_firewall_ip[nPort], eXosip.j_firewall_ports[nPort]);
				}
				else
				{
					if( eXosip.localports[nPort] == NULL )
						sprintf(contact_addr, "%s", locip);
					else
						sprintf(contact_addr, "%s:%s", locip, eXosip.localports[nPort]);
				}
			}
			else
			{
				if( eXosip.localports[nPort] == NULL )
					sprintf(contact_addr, "%s", locip);
				else
					sprintf(contact_addr, "%s:%s", locip, eXosip.localports[nPort]);
			}

#ifdef CONFIG_RTK_VOIP_SIP_TLS
			if (a_from->url->username != NULL)
				sprintf(contact, "<sip:%s@%s;transport=%s>", a_from->url->username, contact_addr, transport);
			else
				sprintf(contact, "<sip:%s;transport=%s>", contact_addr,transport);
#else
			if (a_from->url->username != NULL)
				sprintf(contact, "<sip:%s@%s>", a_from->url->username, contact_addr);
			else
				sprintf(contact, "<sip:%s>", contact_addr);
#endif

			osip_message_set_contact( request , contact ) ;
		}
		osip_from_free( a_from ) ;

		/* This is probably useless for other messages */
		osip_message_set_allow( request , "INVITE" ) ;
		osip_message_set_allow( request , "ACK" ) ;
		osip_message_set_allow( request , "CANCEL" ) ;
		osip_message_set_allow( request , "BYE" ) ;
		osip_message_set_allow( request , "OPTIONS" ) ;
		osip_message_set_allow( request , "REFER" ) ;
		osip_message_set_allow( request , "SUBSCRIBE" ) ;
		osip_message_set_allow( request , "NOTIFY" ) ;
		osip_message_set_allow( request , "MESSAGE" ) ;
		osip_message_set_allow( request , "INFO" ) ;
	}

#ifdef ATTENDED_TRANSFER
	if ( 0 == strcmp( "INVITE" , method_name ) ) {
		osip_message_set_supported( request, "replaces" ) ;
	}
#endif

	if ( 0 == strcmp( "REGISTER" , method_name ) ) {
		osip_message_set_allow( request , "INVITE" ) ;
		osip_message_set_allow( request , "ACK" ) ;
		osip_message_set_allow( request , "CANCEL" ) ;
		osip_message_set_allow( request , "BYE" ) ;
		osip_message_set_allow( request , "OPTIONS" ) ;
		osip_message_set_allow( request , "REFER" ) ;
		osip_message_set_allow( request , "SUBSCRIBE" ) ;
		osip_message_set_allow( request , "NOTIFY" ) ;
		osip_message_set_allow( request , "MESSAGE" ) ;
		osip_message_set_allow( request , "INFO" ) ;
	}

	if( 0 == strcmp( "SUBSCRIBE" , method_name ) )
	{
		osip_message_set_header( request , "Event" , "presence" ) ;
		osip_message_set_accept( request , "application/pidf+xml" ) ;
	}
	else if( 0 == strcmp( "OPTIONS" , method_name ) )
	{
		osip_message_set_accept( request , "application/sdp" ) ;
	}

	osip_message_set_user_agent( request , eXosip.user_agent ) ;
	/*  else if ... */
	*dest = request ;
	osip_free( locip ) ;
	return 0 ;

brood_error_1 :
	osip_message_free( request ) ;
	*dest = NULL ;
	if( locip != NULL )
		osip_free( locip ) ;
	return -1 ;
}

int
generating_register(int nPort, osip_message_t **reg, char *from,
		    osip_list_t *routes, char *proxy, char *contact, int expires)
{
  osip_from_t *a_from;
  int i;
#ifdef SM
  char *locip=NULL;
#else
ddddddddddddddddddd
  char locip[50];
#endif

#ifdef CONFIG_RTK_VOIP_SIP_TLS
	int bTLS=0;
	osip_uri_t *url;
	/*find out the tls sip proxy*/
	osip_uri_init(&url);
	osip_uri_parse(url, proxy);
	if(eXosip_check_tls_proxy(nPort, url->host)){
		i = generating_request_out_of_dialog(nPort, reg, "REGISTER", NULL, "TLS",
				       from, routes, proxy);
		bTLS=1;
	}else{
  i = generating_request_out_of_dialog(nPort, reg, "REGISTER", NULL, "UDP",
				       from, routes, proxy);
	}
	osip_uri_free(url);
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  i = generating_request_out_of_dialog(nPort, reg, "REGISTER", NULL, "UDP",
				       from, routes, proxy);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

  if (i!=0) return -1;

#ifdef SM
  eXosip_get_localip_for((*reg)->req_uri->host,&locip);
#else
eeeeeeeeeeeeeeeeee
  eXosip_guess_ip_for_via(eXosip.ip_family, locip, 49);
#endif

  if (contact==NULL)
    {
      i = osip_from_init(&a_from);
      if (i==0)
	i = osip_from_parse(a_from, from);

      if (i==0 && a_from!=NULL
	  && a_from->url!=NULL && a_from->url->username!=NULL )
	{
	char contact_addr[100];

	  contact = (char *) osip_malloc(50+strlen(a_from->url->username));
	  if (eXosip.j_firewall_ip[nPort][0]!='\0')
	    {
	      char *c_address = (*reg)->req_uri->host;

		  struct addrinfo *addrinfo;
		  struct __eXosip_sockaddr addr;
		  i = eXosip_get_addrinfo(&addrinfo, (*reg)->req_uri->host, 5060);
		  if (i==0)
			{
			  memcpy (&addr, addrinfo->ai_addr, addrinfo->ai_addrlen);
			  freeaddrinfo (addrinfo);
			  c_address = inet_ntoa (((struct sockaddr_in *)((void *)&addr))->sin_addr);
			  OSIP_TRACE (osip_trace
				  (__FILE__, __LINE__, OSIP_INFO1, NULL,
				  "eXosip: here is the resolved destination host=%s\n", c_address));
			}

	      if (eXosip_is_public_address(c_address))
		{
			
		  if (eXosip.j_firewall_ports[nPort]==NULL)
		    sprintf(contact_addr, "%s", eXosip.j_firewall_ip[nPort]);
		  else
		    sprintf(contact_addr, "%s:%s", eXosip.j_firewall_ip[nPort], eXosip.j_firewall_ports[nPort]);
		}
	      else
		{
		  if (eXosip.localports[nPort]==NULL)
		    sprintf(contact_addr, "%s", locip);
		  else
		    sprintf(contact_addr, "%s:%s", locip, eXosip.localports[nPort]);
		}
	    }
	  else
	    {
	      if (eXosip.localports[nPort]==NULL)
		sprintf(contact_addr, "%s", locip);
	      else
		sprintf(contact_addr, "%s:%s", locip, eXosip.localports[nPort]);
	    }
	  
		if (a_from->url->username != NULL)
		{
#ifdef CONFIG_RTK_VOIP_SIP_TLS
			/*add TLS or TCP transport for contact, if not set, default is UDP*/
			if(bTLS)
				sprintf(contact, "<sip:%s@%s;transport=TLS>", a_from->url->username, contact_addr);
			else
			sprintf(contact, "<sip:%s@%s>", a_from->url->username, contact_addr);
#else
			sprintf(contact, "<sip:%s@%s>", a_from->url->username, contact_addr);
#endif
		}
		else
		{
			g_warning("username is null in register!?\n");
			sprintf(contact, "<sip:%s>", contact_addr);
		}

	  osip_message_set_contact(*reg, contact);
	  osip_free(contact);
	}
      osip_from_free(a_from);
    }
  else
    {
      osip_message_set_contact(*reg, contact);
    }

  {
    char exp[10]; /* MUST never be ouside 1 and 3600 */
    snprintf(exp, 9, "%i", expires);
    osip_message_set_expires(*reg, exp);
  }

  osip_message_set_content_length(*reg, "0");
  
#ifdef FIX_MEMORY_LEAK
#ifdef SM
  if (locip) osip_free(locip);
#endif
#endif

  return 0;
}

/* this method can't be called unless the previous
   INVITE transaction is over. */
int eXosip_build_initial_invite(int nPort, osip_message_t **invite, char *to, char *from,
				osip_list_t *routes, char *subject)
{
  int i;

  if (to!=NULL && *to=='\0')
    return -1;

  osip_clrspace(to);
  osip_clrspace(subject);
  osip_clrspace(from);
#if 0  
  osip_clrspace(route);
  if (route!=NULL && *route=='\0')
    route=NULL;
#endif	
  if (subject!=NULL && *subject=='\0')
    subject=NULL;

#ifdef CONFIG_RTK_VOIP_SIP_TLS
	osip_from_t *fromTemp=NULL;
	osip_to_t *toTemp=NULL;
	osip_uri_t *uri=NULL, *toUri=NULL;
	char *inviteHost=NULL, *toHost=NULL;
	int toPort=0;
	proxy_info proxyInfo;

	osip_from_init(&fromTemp);
	if(osip_from_parse(fromTemp, from) != -1){
		uri=osip_from_get_url(fromTemp);
		inviteHost=osip_uri_get_host(uri);
	}

	/*check for direct ip call*/
	osip_to_init(&toTemp);
	if(osip_to_parse(toTemp, to) != -1){
		char *tmpPort=NULL;
		
		toUri=osip_to_get_url(toTemp);
		toHost=osip_uri_get_host(toUri);
		tmpPort=osip_uri_get_port(toUri);
		if(tmpPort != NULL)
			toPort=atoi(osip_uri_get_port(toUri));
	}
	proxyInfo=exosip_get_proxy_info(nPort, toHost, toPort);
	  /*find out the tls sip proxy*/
	if(eXosip_check_tls_proxy(nPort, inviteHost) && proxyInfo.proxyIndex != -1)
		i = generating_request_out_of_dialog(nPort, invite, "INVITE", to, "TLS", from,
				       routes, NULL);
	else
  i = generating_request_out_of_dialog(nPort, invite, "INVITE", to, "UDP", from,
				       routes, NULL);
	osip_from_free(fromTemp);
	osip_to_free(toTemp);
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  i = generating_request_out_of_dialog(nPort, invite, "INVITE", to, "UDP", from,
				       routes, NULL);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

  if (i!=0) return -1;
  
  if (subject!=NULL)
	  osip_message_set_subject(*invite, subject);

  /* after this delay, we should send a CANCEL */
  osip_message_set_expires(*invite, "120");

  /* osip_message_set_organization(*invite, "Jack's Org"); */
  return 0;
}

/* this method can't be called unless the previous
   INVITE transaction is over. */
int eXosip_build_initial_options(int nPort, osip_message_t **options, char *to, char *from,
				 osip_list_t *routes)
{
  int i;

  if (to!=NULL && *to=='\0')
    return -1;

  osip_clrspace(to);
  osip_clrspace(from);
#if 0  
  osip_clrspace(route);
  if (route!=NULL && *route=='\0')
    route=NULL;
#endif

#ifdef CONFIG_RTK_VOIP_SIP_TLS
osip_from_t *fromTemp=NULL;
osip_to_t *toTemp=NULL;
osip_uri_t *uri=NULL, *toUri=NULL;
char *inviteHost=NULL, *toHost=NULL;
int toPort=0;
proxy_info proxyInfo;

osip_from_init(&fromTemp);
if(osip_from_parse(fromTemp, from) != -1){
	uri=osip_from_get_url(fromTemp);
	inviteHost=osip_uri_get_host(uri);
}

	/*check for direct ip call*/
	osip_to_init(&toTemp);
	if(osip_to_parse(toTemp, to) != -1){
		char *tmpPort=NULL;
		
		toUri=osip_to_get_url(toTemp);
		toHost=osip_uri_get_host(toUri);
		tmpPort=osip_uri_get_port(toUri);
		if(tmpPort != NULL)
			toPort=atoi(osip_uri_get_port(toUri));
	}
	proxyInfo=exosip_get_proxy_info(nPort, toHost, toPort);
	  /*find out the tls sip proxy*/
	if(eXosip_check_tls_proxy(nPort, inviteHost) && proxyInfo.proxyIndex != -1)
		i = generating_request_out_of_dialog(nPort, options, "OPTIONS", to, "TLS", from,
				       routes, NULL);
	else
		i = generating_request_out_of_dialog(nPort, options, "OPTIONS", to, "UDP", from,
				       routes, NULL);
	osip_from_free(fromTemp);
	osip_to_free(toTemp);
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  i = generating_request_out_of_dialog(nPort, options, "OPTIONS", to, "UDP", from,
				       routes, NULL);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

  if (i!=0) return -1;

  /* after this delay, we should send a CANCEL */
  osip_message_set_expires(*options, "120");

  /* osip_message_set_organization(*invite, "Jack's Org"); */
  return 0;
}

/* this method can't be called unless the previous
   INVITE transaction is over. */
int generating_initial_subscribe(int nPort, osip_message_t **subscribe, char *to,
				 char *from, osip_list_t *routes)
{
  int i;

  if (to!=NULL && *to=='\0')
    return -1;

  osip_clrspace(to);
  osip_clrspace(from);
#if 0  
  osip_clrspace(route);
  if (route!=NULL && *route=='\0')
    route=NULL;
#endif

#ifdef CONFIG_RTK_VOIP_SIP_TLS
	  osip_from_t *fromTemp=NULL;
	  osip_to_t *toTemp=NULL;
	  osip_uri_t *uri=NULL, *toUri=NULL;
	  char *inviteHost=NULL, *toHost=NULL;
	  int toPort=0;
	  proxy_info proxyInfo;
  
	  osip_from_init(&fromTemp);
	  if(osip_from_parse(fromTemp, from) != -1){
		  uri=osip_from_get_url(fromTemp);
		  inviteHost=osip_uri_get_host(uri);
	  }
  
	  /*check for direct ip call*/
	  osip_to_init(&toTemp);
	  if(osip_to_parse(toTemp, to) != -1){
		  char *tmpPort=NULL;
		  
		  toUri=osip_to_get_url(toTemp);
		  toHost=osip_uri_get_host(toUri);
		  tmpPort=osip_uri_get_port(toUri);
		  if(tmpPort != NULL)
			  toPort=atoi(osip_uri_get_port(toUri));
	  }
	  proxyInfo=exosip_get_proxy_info(nPort, toHost, toPort);
		/*find out the tls sip proxy*/
	  if(eXosip_check_tls_proxy(nPort, inviteHost) && proxyInfo.proxyIndex != -1)
		  i = generating_request_out_of_dialog(nPort, subscribe, "SUBSCRIBE", to, "TLS", from,
						 routes, NULL);
	  else
  i = generating_request_out_of_dialog(nPort, subscribe, "SUBSCRIBE", to, "UDP", from,
				       routes, NULL);
	  osip_from_free(fromTemp);
	  osip_to_free(toTemp);
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  i = generating_request_out_of_dialog(nPort, subscribe, "SUBSCRIBE", to, "UDP", from,
				       routes, NULL);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

  if (i!=0) return -1;
  
#ifdef LOW_EXPIRE
ffffffffffffffffff
  osip_message_set_expires(*subscribe, "120");
#else
  osip_message_set_expires(*subscribe, "3600");
#endif

  /* osip_message_set_organization(*subscribe, "Jack's Org"); */
  return 0;
}

/* this method can't be called unless the previous
   INVITE transaction is over. */
int generating_message(int nPort, osip_message_t **message, char *to, char *from,
		       osip_list_t *routes, char *buff)
{
  int i;

  if (to!=NULL && *to=='\0')
    return -1;

  osip_clrspace(to);
  /*  osip_clrspace(buff); */
  osip_clrspace(from);
#if 0  
  osip_clrspace(route);
  if (route!=NULL && *route=='\0')
    route=NULL;
#endif	

#ifdef CONFIG_RTK_VOIP_SIP_TLS
	  osip_from_t *fromTemp=NULL;
	  osip_to_t *toTemp=NULL;
	  osip_uri_t *uri=NULL, *toUri=NULL;
	  char *inviteHost=NULL, *toHost=NULL;
	  int toPort=0;
	  proxy_info proxyInfo;
  
	  osip_from_init(&fromTemp);
	  if(osip_from_parse(fromTemp, from) != -1){
		  uri=osip_from_get_url(fromTemp);
		  inviteHost=osip_uri_get_host(uri);
	  }
  
	  /*check for direct ip call*/
	  osip_to_init(&toTemp);
	  if(osip_to_parse(toTemp, to) != -1){
		  char *tmpPort=NULL;
		  
		  toUri=osip_to_get_url(toTemp);
		  toHost=osip_uri_get_host(toUri);
		  tmpPort=osip_uri_get_port(toUri);
		  if(tmpPort != NULL)
			  toPort=atoi(osip_uri_get_port(toUri));
	  }
	  proxyInfo=exosip_get_proxy_info(nPort, toHost, toPort);
		/*find out the tls sip proxy*/
	  if(eXosip_check_tls_proxy(nPort, inviteHost) && proxyInfo.proxyIndex != -1){
		if (buff!=NULL && *buff=='\0')
		    return -1; /* at least, the message must be of length >= 1 */
		i = generating_request_out_of_dialog(nPort, message, "MESSAGE", to, "TLS", from,
						 routes, NULL);
	  }else{
	  	if (buff!=NULL && *buff=='\0')
		    return -1; /* at least, the message must be of length >= 1 */
		  i = generating_request_out_of_dialog(nPort, message, "MESSAGE", to, "UDP", from,
						 routes, NULL);
	  }
	  osip_from_free(fromTemp);
	  osip_to_free(toTemp);
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  if (buff!=NULL && *buff=='\0')
    return -1; /* at least, the message must be of length >= 1 */
  
  i = generating_request_out_of_dialog(nPort, message, "MESSAGE", to, "UDP", from,
				       routes, NULL);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

  if (i!=0) return -1;
  
  osip_message_set_expires(*message, "120");
  osip_message_set_body(*message, buff, strlen(buff));
  osip_message_set_content_type(*message, "text/plain");

  /* osip_message_set_organization(*message, "Jack's Org"); */


  return 0;
}

/* this method can't be called unless the previous
   INVITE transaction is over. */
int
generating_publish(int nPort, osip_message_t **message, char *to, char *from,
		   osip_list_t *routes)
{
  int i;

  if (to!=NULL && *to=='\0')
    return -1;

  osip_clrspace(to);
  osip_clrspace(from);
#if 0  
  osip_clrspace(route);
  if (route!=NULL && *route=='\0')
    route=NULL;
#endif
#ifdef CONFIG_RTK_VOIP_SIP_TLS
		osip_from_t *fromTemp=NULL;
		osip_to_t *toTemp=NULL;
		osip_uri_t *uri=NULL, *toUri=NULL;
		char *inviteHost=NULL, *toHost=NULL;
		int toPort=0;
		proxy_info proxyInfo;
	
		osip_from_init(&fromTemp);
		if(osip_from_parse(fromTemp, from) != -1){
			uri=osip_from_get_url(fromTemp);
			inviteHost=osip_uri_get_host(uri);
		}
	
		/*check for direct ip call*/
		osip_to_init(&toTemp);
		if(osip_to_parse(toTemp, to) != -1){
			char *tmpPort=NULL;
			
			toUri=osip_to_get_url(toTemp);
			toHost=osip_uri_get_host(toUri);
			tmpPort=osip_uri_get_port(toUri);
			if(tmpPort != NULL)
				toPort=atoi(osip_uri_get_port(toUri));
		}
		proxyInfo=exosip_get_proxy_info(nPort, toHost, toPort);
		  /*find out the tls sip proxy*/
		if(eXosip_check_tls_proxy(nPort, inviteHost) && proxyInfo.proxyIndex != -1)
			i = generating_request_out_of_dialog(nPort, message, "PUBLISH", to, "TLS", from,
						   routes, NULL);
		else
  i = generating_request_out_of_dialog(nPort, message, "PUBLISH", to, "UDP", from,
				       routes, NULL);
		osip_from_free(fromTemp);
		osip_to_free(toTemp);
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
	  i = generating_request_out_of_dialog(nPort, message, "PUBLISH", to, "UDP", from,
						   routes, NULL);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

  if (i!=0) return -1;
  
  /* osip_message_set_organization(*message, "Jack's Org"); */

  return 0;
}


int
generating_options(int nPort, osip_message_t **options, char *from, char *to, osip_list_t *routes)
{
  int i;
#ifdef CONFIG_RTK_VOIP_SIP_TLS
		osip_from_t *fromTemp=NULL;
		osip_to_t *toTemp=NULL;
		osip_uri_t *uri=NULL, *toUri=NULL;
		char *inviteHost=NULL, *toHost=NULL;
		int toPort=0;
		proxy_info proxyInfo;
	
		osip_from_init(&fromTemp);
		if(osip_from_parse(fromTemp, from) != -1){
			uri=osip_from_get_url(fromTemp);
			inviteHost=osip_uri_get_host(uri);
		}
	
		/*check for direct ip call*/
		osip_to_init(&toTemp);
		if(osip_to_parse(toTemp, to) != -1){
			char *tmpPort=NULL;
			
			toUri=osip_to_get_url(toTemp);
			toHost=osip_uri_get_host(toUri);
			tmpPort=osip_uri_get_port(toUri);
			if(tmpPort != NULL)
				toPort=atoi(osip_uri_get_port(toUri));
		}
		proxyInfo=exosip_get_proxy_info(nPort, toHost, toPort);
		  /*find out the tls sip proxy*/
		if(eXosip_check_tls_proxy(nPort, inviteHost) && proxyInfo.proxyIndex != -1)
			i = generating_request_out_of_dialog(nPort, options, "OPTIONS", to, "TLS", from,
						   routes, NULL);
		else
			i = generating_request_out_of_dialog(nPort, options, "OPTIONS", to, "UDP", from,
						   routes, NULL);
		osip_from_free(fromTemp);
		osip_to_free(toTemp);
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  i = generating_request_out_of_dialog(nPort, options, "OPTIONS", to, "UDP",
				       from, routes, NULL);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
  if (i!=0) return -1;

#if 0
  if (sdp!=NULL)
    {
      osip_message_set_content_type(*options, "application/sdp");
      osip_message_set_body(*options, sdp);
    }
#endif

  return 0;
}

int
generating_info(int nPort, osip_message_t **info, char *from, char *to, osip_list_t *routes)
{
  int i;
#ifdef CONFIG_RTK_VOIP_SIP_TLS
		osip_from_t *fromTemp=NULL;
		osip_to_t *toTemp=NULL;
		osip_uri_t *uri=NULL, *toUri=NULL;
		char *inviteHost=NULL, *toHost=NULL;
		int toPort=0;
		proxy_info proxyInfo;
	
		osip_from_init(&fromTemp);
		if(osip_from_parse(fromTemp, from) != -1){
			uri=osip_from_get_url(fromTemp);
			inviteHost=osip_uri_get_host(uri);
		}
	
		/*check for direct ip call*/
		osip_to_init(&toTemp);
		if(osip_to_parse(toTemp, to) != -1){
			char *tmpPort=NULL;
			
			toUri=osip_to_get_url(toTemp);
			toHost=osip_uri_get_host(toUri);
			tmpPort=osip_uri_get_port(toUri);
			if(tmpPort != NULL)
				toPort=atoi(osip_uri_get_port(toUri));
		}
		proxyInfo=exosip_get_proxy_info(nPort, toHost, toPort);
		  /*find out the tls sip proxy*/
		if(eXosip_check_tls_proxy(nPort, inviteHost) && proxyInfo.proxyIndex != -1)
			i = generating_request_out_of_dialog(nPort, info, "INFO", to, "TLS", from,
						   routes, NULL);
		else
			i = generating_request_out_of_dialog(nPort, info, "INFO", to, "UDP", from,
						   routes, NULL);
		osip_from_free(fromTemp);
		osip_to_free(toTemp);
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  i = generating_request_out_of_dialog(nPort, info, "INFO", to, "UDP",
				       from, routes, NULL);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
  if (i!=0) return -1;
  return 0;
}


#ifdef SIP_PING

int
generating_ping(int nPort, osip_message_t **ping, char *from, char *to, osip_list_t *routes)
{
  int i;

#ifdef CONFIG_RTK_VOIP_SIP_TLS
		  osip_from_t *fromTemp=NULL;
		  osip_to_t *toTemp=NULL;
		  osip_uri_t *uri=NULL, *toUri=NULL;
		  char *inviteHost=NULL, *toHost=NULL;
		  int toPort=0;
		  proxy_info proxyInfo;
	  
		  osip_from_init(&fromTemp);
		  if(osip_from_parse(fromTemp, from) != -1){
			  uri=osip_from_get_url(fromTemp);
			  inviteHost=osip_uri_get_host(uri);
		  }
	  
		  /*check for direct ip call*/
		  osip_to_init(&toTemp);
		  if(osip_to_parse(toTemp, to) != -1){
			  char *tmpPort=NULL;
			  
			  toUri=osip_to_get_url(toTemp);
			  toHost=osip_uri_get_host(toUri);
			  tmpPort=osip_uri_get_port(toUri);
			  if(tmpPort != NULL)
				  toPort=atoi(osip_uri_get_port(toUri));
		  }
		  proxyInfo=exosip_get_proxy_info(nPort, toHost, toPort);
			/*find out the tls sip proxy*/
		  if(eXosip_check_tls_proxy(nPort, inviteHost) && proxyInfo.proxyIndex != -1)
			  i = generating_request_out_of_dialog(nPort, ping, "PING", to, "TCP", from,
							 routes, NULL);
		  else
			  i = generating_request_out_of_dialog(nPort, ping, "PING", to, "UDP", from,
							 routes, NULL);
		  osip_from_free(fromTemp);
		  osip_to_free(toTemp);
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  i = generating_request_out_of_dialog(nPort, ping, "PING", to, "UDP",
				       from, routes, NULL);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
  if (i!=0) return -1;

  osip_message_set_contact(*ping, from);
  return 0;
}

#endif

static int
dialog_fill_route_set(osip_dialog_t *dialog, osip_message_t *request)
{
  /* if the pre-existing route set contains a "lr" (compliance
     with bis-08) then the req_uri should contains the remote target
     URI */
  int i;
  int pos=0;
  osip_uri_param_t *lr_param;
  osip_route_t *route;
  char *last_route;
  /* AMD bug: fixed 17/06/2002 */

  if (dialog->type==CALLER)
    {
      pos = osip_list_size(dialog->route_set)-1;
      route = (osip_route_t*)osip_list_get(dialog->route_set, pos);
    }
  else
    route = (osip_route_t*)osip_list_get(dialog->route_set, 0);

	/* check if there are any "loose route" in the route header */    
  osip_uri_uparam_get_byname(route->url, "lr", &lr_param);
	
  if (lr_param!=NULL) /* the remote target URI is the req_uri!For loose route*/
    {
      i = osip_uri_clone(dialog->remote_contact_uri->url,
		    &(request->req_uri));
      if (i!=0) return -1;
      /* "[request] MUST includes a Route header field containing
	 the route set values in order." */
      /* AMD bug: fixed 17/06/2002 */
      pos=0; /* first element is at index 0 */
      while (!osip_list_eol(dialog->route_set, pos))
	{
	  osip_route_t *route2;
	  route = (osip_route_t*)osip_list_get(dialog->route_set, pos);
	  i = osip_route_clone(route, &route2);
	  if (i!=0) return -1;
	  if (dialog->type==CALLER)
	    osip_list_add(request->routes, route2, 0);
	  else
	    osip_list_add(request->routes, route2, -1);
	  pos++;
	}
      return 0;
    }

  /* if the first URI of route set does not contain "lr", the req_uri
     is set to the first uri of route set */

  i = osip_uri_clone(route->url, &(request->req_uri));
  if (i!=0) return -1;
  /* add the route set */
  /* "The UAC MUST add a route header field containing
     the remainder of the route set values in order. */
  
#ifdef RTK_FIX_ROUTE
  pos=0; /* yes it is */
#else
  if (dialog->type==CALLER)
    pos=0; /* yes it is */
  else
    pos=1;
#endif

  while (!osip_list_eol(dialog->route_set, pos)) /* not the first one in the list */
    {
      osip_route_t *route2;
      route = (osip_route_t*)osip_list_get(dialog->route_set, pos);
      i = osip_route_clone(route, &route2);
      if (i!=0) return -1;
#ifdef RTK_FIX_ROUTE
		// rock: add strict rouet set info for later process
		//	1. osip use route set to select destination,
		//  2. osip remove first route if it is strict route
		if (dialog->type==CALLER)
			osip_list_add(request->routes, route2, 0);
		else
			osip_list_add(request->routes, route2, -1);
#else
      if (dialog->type==CALLER)
	{
	  if (pos!=osip_list_size(dialog->route_set)-1)
	    osip_list_add(request->routes, route2, 0);
	  else
	    osip_route_free(route2);
	}
      else
	{
	  if (!osip_list_eol(dialog->route_set, pos))
	    osip_list_add(request->routes, route2, -1);
	  else
	    osip_route_free(route2);
	}
#endif
      pos++;
    }

  /* The UAC MUST then place the remote target URI into
     the route header field as the last value */
  i = osip_uri_to_str(dialog->remote_contact_uri->url, &last_route);
  if (i!=0) return -1;
  i = osip_message_set_route(request, last_route);
  osip_free(last_route);
  if (i!=0) { return -1; }
  
  /* route header and req_uri set */
  return 0;
}

int
_eXosip_build_request_within_dialog(int nPort, osip_message_t **dest, char *method_name,
				   osip_dialog_t *dialog, char *transport)
{
  int i;
  osip_message_t *request;
#ifdef SM
  char *locip=NULL;
#else
ggggggggggggggggggggg
  char locip[50];
#endif
  
  i = osip_message_init(&request);
  if (i!=0) return -1;

  if (dialog->remote_contact_uri==NULL)
    {
      /* this dialog is probably not established! or the remote UA
	 is not compliant with the latest RFC
      */
      osip_message_free(request);
      return -1;
    }
#ifdef SM
  eXosip_get_localip_for(dialog->remote_contact_uri->url->host,&locip);
#else
hhhhhhhhhhhhhhhhhh
  eXosip_guess_ip_for_via(eXosip.ip_family, locip, 49);
#endif
  /* prepare the request-line */
  request->sip_method  = osip_strdup(method_name);
  request->sip_version = osip_strdup("SIP/2.0");
  request->status_code   = 0;
  request->reason_phrase = NULL;

  /* and the request uri???? */
  if (osip_list_eol(dialog->route_set, 0))
    {
      /* The UAC must put the remote target URI (to field) in the req_uri */
      i = osip_uri_clone(dialog->remote_contact_uri->url, &(request->req_uri));
      if (i!=0) goto grwd_error_1;
    }
  else
    {
      /* fill the request-uri, and the route headers. */
      dialog_fill_route_set(dialog, request);
    }
  
  /* To and From already contains the proper tag! */
  i = osip_to_clone(dialog->remote_uri, &(request->to));
  if (i!=0) goto grwd_error_1;
  i = osip_from_clone(dialog->local_uri, &(request->from));
  if (i!=0) goto grwd_error_1;

  /* set the cseq and call_id header */
  osip_message_set_call_id(request, dialog->call_id);

  if (0==strcmp("ACK", method_name))
    {
      osip_cseq_t *cseq;
      char *tmp;
      i = osip_cseq_init(&cseq);
      if (i!=0) goto grwd_error_1;
      tmp = osip_malloc(20);
      sprintf(tmp,"%i", dialog->local_cseq);
      osip_cseq_set_number(cseq, tmp);
      osip_cseq_set_method(cseq, osip_strdup(method_name));
      request->cseq = cseq;
    }
  else
    {
      osip_cseq_t *cseq;
      char *tmp;
      i = osip_cseq_init(&cseq);
      if (i!=0) goto grwd_error_1;
      dialog->local_cseq++; /* we should we do that?? */
      tmp = osip_malloc(20);
      sprintf(tmp,"%i", dialog->local_cseq);
      osip_cseq_set_number(cseq, tmp);
      osip_cseq_set_method(cseq, osip_strdup(method_name));
      request->cseq = cseq;
    }
  
  /* always add the Max-Forward header */
  osip_message_set_max_forwards(request, "70"); /* a UA should start a request with 70 */


  /* even for ACK for 2xx (ACK within a dialog), the branch ID MUST
     be a new ONE! */
  /* should be useless with compliant UA */
  if (eXosip.j_firewall_ip[nPort][0]!='\0')
  {
	  char *c_address = request->req_uri->host;

	  struct addrinfo *addrinfo;
	  struct __eXosip_sockaddr addr;
	  i = eXosip_get_addrinfo(&addrinfo, request->req_uri->host, 5060);
	  if (i==0)
		{
		  memcpy (&addr, addrinfo->ai_addr, addrinfo->ai_addrlen);
		  freeaddrinfo (addrinfo);
		  c_address = inet_ntoa (((struct sockaddr_in *)((void *)&addr))->sin_addr);
		  OSIP_TRACE (osip_trace
			  (__FILE__, __LINE__, OSIP_INFO1, NULL,
			  "eXosip: here is the resolved destination host=%s\n", c_address));
		}

	  if (eXosip_is_public_address(c_address))
	  {
		    char tmp[200];
		    sprintf(tmp, "SIP/2.0/%s %s:%s;rport;branch=z9hG4bK%u", transport,
			    eXosip.j_firewall_ip[nPort],
			    eXosip.j_firewall_ports[nPort],
			    via_branch_new_random() );
		    osip_message_set_via(request, tmp);
	  }
	  else
	  {
	    char tmp[200];
	    if (eXosip.ip_family==AF_INET6)
	      snprintf(tmp, 200, "SIP/2.0/%s [%s]:%s;branch=z9hG4bK%u", transport,
		      locip,
		      eXosip.localports[nPort],
		      via_branch_new_random() );
	    else
	      snprintf(tmp, 200, "SIP/2.0/%s %s:%s;rport;branch=z9hG4bK%u", transport,
		      locip,
		      eXosip.localports[nPort],
		      via_branch_new_random() );
	      
	    osip_message_set_via(request, tmp);
	  }
  }
  else
  {
    char tmp[200];
    if (eXosip.ip_family==AF_INET6)
      snprintf(tmp, 200, "SIP/2.0/%s [%s]:%s;branch=z9hG4bK%u", transport,
	      locip,
	      eXosip.localports[nPort],
	      via_branch_new_random() );
    else
      snprintf(tmp, 200, "SIP/2.0/%s %s:%s;rport;branch=z9hG4bK%u", transport,
	      locip,
	      eXosip.localports[nPort],
	      via_branch_new_random() );

    osip_message_set_via(request, tmp);
  }

  /* add specific headers for each kind of request... */

#if 0
  if (0==strcmp("INVITE", method_name) || 0==strcmp("SUBSCRIBE", method_name))
#endif
    {
      char contact[200];
      char contact_addr[100];
	if (eXosip.j_firewall_ip[nPort][0]!='\0')
	{
	  char *c_address = request->req_uri->host;

	  struct addrinfo *addrinfo;
	  struct __eXosip_sockaddr addr;
	  i = eXosip_get_addrinfo(&addrinfo, request->req_uri->host, 5060);
	  if (i==0)
		{
		  memcpy (&addr, addrinfo->ai_addr, addrinfo->ai_addrlen);
		  freeaddrinfo (addrinfo);
		  c_address = inet_ntoa (((struct sockaddr_in *)((void *)&addr))->sin_addr);
		  OSIP_TRACE (osip_trace
			  (__FILE__, __LINE__, OSIP_INFO1, NULL,
			  "eXosip: here is the resolved destination host=%s\n", c_address));
		}

	  if (eXosip_is_public_address(c_address))
	    {
	      sprintf(contact_addr, "%s:%s", eXosip.j_firewall_ip[nPort], eXosip.j_firewall_ports[nPort]);
	    }
	  else
	    {
	      sprintf(contact_addr, "%s:%s", locip, eXosip.localports[nPort]);
	    }
	}
      else
	{
	      sprintf(contact_addr, "%s:%s", locip, eXosip.localports[nPort]);
	}
      if (dialog->local_uri->url->username)
#ifdef CONFIG_RTK_VOIP_SIP_TLS
		sprintf(contact, "<sip:%s@%s;transport=%s>", dialog->local_uri->url->username, contact_addr, transport);
	else
		sprintf(contact, "<sip:%s;transport=%s>", contact_addr, transport);
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
          sprintf(contact, "<sip:%s@%s>", dialog->local_uri->url->username, contact_addr);
      else
          sprintf(contact, "<sip:%s>", contact_addr);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
      osip_message_set_contact(request, contact);
      /* Here we'll add the supported header if it's needed! */
      /* the require header must be added by the upper layer if needed */
    }

  if (0==strcmp("SUBSCRIBE", method_name))
    {
      osip_message_set_header(request, "Event", "presence");
#ifdef SUPPORT_MSN
iiiiiiiiiiiiiiiiiii
      osip_message_set_accept(request, "application/xpidf+xml");
#else
      osip_message_set_accept(request, "application/pidf+xml");
#endif
    }
  else if (0==strcmp("NOTIFY", method_name))
    {
    }
  else if (0==strcmp("INFO", method_name))
    {

    }
  else if (0==strcmp("OPTIONS", method_name))
    {
      osip_message_set_accept(request, "application/sdp");
    }
  else if (0==strcmp("ACK", method_name))
    {
      /* The ACK MUST contains the same credential than the INVITE!! */
      /* TODO... */
    }

  osip_message_set_user_agent(request, eXosip.user_agent);
  /*  else if ... */
  *dest = request;

#ifdef FIX_MEMORY_LEAK
#ifdef SM
  if (locip) osip_free(locip);
#endif
#endif

  return 0;

  /* grwd_error_2: */
  dialog->local_cseq--;
 grwd_error_1:
  osip_message_free(request);
  *dest = NULL;

#ifdef FIX_MEMORY_LEAK
#ifdef SM
  if (locip) osip_free(locip);
#endif
#endif

  return -1;
}

/* this request is only build within a dialog!! */
int
generating_bye(int nPort, osip_message_t **bye, osip_dialog_t *dialog)
{
  int i;
#ifdef CONFIG_RTK_VOIP_SIP_TLS
  osip_uri_t *uri=NULL;
  char *host, *remoteHost;
  proxy_info proxyInfo;
  int remotePort;

  uri=osip_to_get_url(dialog->remote_uri);
  host=osip_uri_get_host(uri);
  remotePort=osip_uri_get_port(uri);
  proxyInfo=exosip_get_proxy_info(nPort, host, remotePort);;

  uri=osip_from_get_url(dialog->local_uri);
  host=osip_uri_get_host(uri);
  
  if(eXosip_check_tls_proxy(nPort, host) && proxyInfo.proxyIndex != -1)
  	i = _eXosip_build_request_within_dialog(nPort, bye, "BYE", dialog, "TLS");
  else
  i = _eXosip_build_request_within_dialog(nPort, bye, "BYE", dialog, "UDP");
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  i = _eXosip_build_request_within_dialog(nPort, bye, "BYE", dialog, "UDP");
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
  if (i!=0) return -1;

  return 0;
}

/* this request is only build within a dialog! (but should not!) */
int
generating_refer_outside_dialog(int nPort, osip_message_t **refer, char *refer_to, char *from, char *to, osip_list_t *routes)
{
  int i;
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	osip_from_t *fromTemp=NULL;
	osip_to_t *toTemp=NULL;
	osip_uri_t *uri=NULL, *toUri=NULL;
	char *inviteHost=NULL, *toHost=NULL;
	int toPort=0;
	proxy_info proxyInfo;
	  
	osip_from_init(&fromTemp);
	if(osip_from_parse(fromTemp, from) != -1){
		uri=osip_from_get_url(fromTemp);
		inviteHost=osip_uri_get_host(uri);
	}
	  
	/*check for direct ip call*/
	osip_to_init(&toTemp);
	if(osip_to_parse(toTemp, to) != -1){
		char *tmpPort=NULL;
			  
		toUri=osip_to_get_url(toTemp);
		toHost=osip_uri_get_host(toUri);
		tmpPort=osip_uri_get_port(toUri);
		if(tmpPort != NULL)
			toPort=atoi(osip_uri_get_port(toUri));
		}
		proxyInfo=exosip_get_proxy_info(nPort, toHost, toPort);
		/*find out the tls sip proxy*/
		if(eXosip_check_tls_proxy(nPort, inviteHost) && proxyInfo.proxyIndex != -1)
			i = generating_request_out_of_dialog(nPort, refer, "REFER", to, "TLS", from,
							 routes, NULL);
		  else
			i = generating_request_out_of_dialog(nPort, refer, "REFER", to, "UDP", from,
							 routes, NULL);
		osip_from_free(fromTemp);
		osip_to_free(toTemp);
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  i = generating_request_out_of_dialog(nPort, refer, "REFER", to, "UDP",
				       from, routes, NULL);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
  if (i!=0) return -1;

  osip_message_set_header(*refer, "Refer-to", refer_to);
  return 0;
}

/* this request is only build within a dialog! (but should not!) */
int
generating_refer(int nPort, osip_message_t **refer, osip_dialog_t *dialog, const char *refer_to)
{
  int i;
#ifdef CONFIG_RTK_VOIP_SIP_TLS
  osip_uri_t *uri=NULL;
  char *host;

  uri=osip_from_get_url(dialog->local_uri);
  host=osip_uri_get_host(uri);
  if(eXosip_check_tls_proxy(nPort, host))
  	i = _eXosip_build_request_within_dialog(nPort, refer, "REFER", dialog, "TLS");
  else
  	i = _eXosip_build_request_within_dialog(nPort, refer, "REFER", dialog, "UDP");
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  i = _eXosip_build_request_within_dialog(nPort, refer, "REFER", dialog, "UDP");
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
  if (i!=0) return -1;

#ifdef ATTENDED_TRANSFER
  osip_message_set_header(*refer, "Refer-To", refer_to);
#else
  osip_message_set_header(*refer, "Refer-to", refer_to);
#endif

  return 0;
}

/* this request can be inside or outside a dialog */
int
generating_options_within_dialog(int nPort, osip_message_t **options, osip_dialog_t *dialog)
{
  int i;
#ifdef CONFIG_RTK_VOIP_SIP_TLS
  osip_uri_t *uri=NULL;
  char *host;

  uri=osip_from_get_url(dialog->local_uri);
  host=osip_uri_get_host(uri);
  if(eXosip_check_tls_proxy(nPort, host))
  	i = _eXosip_build_request_within_dialog(nPort, options, "OPTIONS", dialog, "TLS");
  else
  	i = _eXosip_build_request_within_dialog(nPort, options, "OPTIONS", dialog, "UDP");
#else /*CONFIG_RTK_VOIP_SIP_TLS*/  	 
  i = _eXosip_build_request_within_dialog(nPort, options, "OPTIONS", dialog, "UDP");
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
  if (i!=0) return -1;

#if 0
  if (sdp!=NULL)
    {
      osip_message_set_content_type(*options, "application/sdp");
      osip_message_set_body(*options, sdp);
    }
#endif

  return 0;
}

int
generating_info_within_dialog(int nPort, osip_message_t **info, osip_dialog_t *dialog)
{
  int i;
#ifdef CONFIG_RTK_VOIP_SIP_TLS
  osip_uri_t *uri=NULL;
  char *host;

  uri=osip_from_get_url(dialog->local_uri);
  host=osip_uri_get_host(uri);
  if(eXosip_check_tls_proxy(nPort, host))
  	i = _eXosip_build_request_within_dialog(nPort, info, "INFO", dialog, "TLS");
  else
  i = _eXosip_build_request_within_dialog(nPort, info, "INFO", dialog, "UDP");
#else  	
  i = _eXosip_build_request_within_dialog(nPort, info, "INFO", dialog, "UDP");
#endif
  if (i!=0) return -1;
  return 0;
}

/* It is RECOMMENDED to only cancel INVITE request */
int
generating_cancel(int nPort, osip_message_t **dest, osip_message_t *request_cancelled)
{
  int i;
  osip_message_t *request;
  
  i = osip_message_init(&request);
  if (i!=0) return -1;
  
  /* prepare the request-line */
  osip_message_set_method(request, osip_strdup("CANCEL"));
  osip_message_set_version(request, osip_strdup("SIP/2.0"));
  osip_message_set_status_code(request, 0);
  osip_message_set_reason_phrase(request, NULL);

  i = osip_uri_clone(request_cancelled->req_uri, &(request->req_uri));
  if (i!=0) goto gc_error_1;
  
  i = osip_to_clone(request_cancelled->to, &(request->to));
  if (i!=0) goto gc_error_1;
  i = osip_from_clone(request_cancelled->from, &(request->from));
  if (i!=0) goto gc_error_1;
  
  /* set the cseq and call_id header */
  i = osip_call_id_clone(request_cancelled->call_id, &(request->call_id));
  if (i!=0) goto gc_error_1;
  i = osip_cseq_clone(request_cancelled->cseq, &(request->cseq));
  if (i!=0) goto gc_error_1;
  osip_free(request->cseq->method);
  request->cseq->method = osip_strdup("CANCEL");
  
  /* copy ONLY the top most Via Field (this method is also used by proxy) */
  {
    osip_via_t *via;
    osip_via_t *via2;
    i = osip_message_get_via(request_cancelled, 0, &via);
    if (i!=0) goto gc_error_1;
    i = osip_via_clone(via, &via2);
    if (i!=0) goto gc_error_1;
    osip_list_add(request->vias, via2, -1);
  }

  /* add the same route-set than in the previous request */
  {
    int pos=0;
    osip_route_t *route;
    osip_route_t *route2;
    while (!osip_list_eol(request_cancelled->routes, pos))
      {
	route = (osip_route_t*) osip_list_get(request_cancelled->routes, pos);
	i = osip_route_clone(route, &route2);
	if (i!=0) goto gc_error_1;
	osip_list_add(request->routes, route2, -1);
	pos++;
      }
  }

  osip_message_set_max_forwards(request, "70"); /* a UA should start a request with 70 */
  osip_message_set_user_agent(request, eXosip.user_agent);

  *dest = request;
  return 0;

 gc_error_1:
  osip_message_free(request);
  *dest = NULL;
  return -1;
}


int
generating_ack_for_2xx(int nPort, osip_message_t **ack, osip_dialog_t *dialog)
{
  int i;
#ifdef CONFIG_RTK_VOIP_SIP_TLS
  osip_uri_t *uri=NULL;
  char *host;

  uri=osip_from_get_url(dialog->local_uri);
  host=osip_uri_get_host(uri);
  if(eXosip_check_tls_proxy(nPort, host))
  	i = _eXosip_build_request_within_dialog(nPort, ack, "ACK", dialog, "TLS");
  else
  	i = _eXosip_build_request_within_dialog(nPort, ack, "ACK", dialog, "UDP");
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  i = _eXosip_build_request_within_dialog(nPort, ack, "ACK", dialog, "UDP");
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
  if (i!=0) return -1;

  return 0;
}

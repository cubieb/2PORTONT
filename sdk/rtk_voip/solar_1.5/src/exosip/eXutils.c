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


#include "../osip/osip_port.h"
#include "eXosip2.h"

extern eXosip_t eXosip;

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <sys/param.h>
#include <stdio.h>

#ifdef RTK_DNS
#include "dns_api.h"
#endif

static int ppl_dns_default_gateway_ipv4 (char *address, int size);
static int ppl_dns_default_gateway_ipv6 (char *address, int size);

int
eXosip_guess_ip_for_via (int familiy, char *address, int size)
{
  if (familiy==AF_INET6)
    {
      return ppl_dns_default_gateway_ipv6 (address, size);
    }
  else
    {
      return ppl_dns_default_gateway_ipv4 (address, size);
    }
}

/* This is a portable way to find the default gateway.
 * The ip of the default interface is returned.
 */
static int
ppl_dns_default_gateway_ipv4 (char *address, int size)
{
  unsigned int len;
  int sock_rt, on=1;
  struct sockaddr_in iface_out;
  struct sockaddr_in remote;
  
  memset(&remote, 0, sizeof(struct sockaddr_in));

  remote.sin_family = AF_INET;
  remote.sin_addr.s_addr = inet_addr("217.12.3.11");
  remote.sin_port = htons(11111);
  
  memset(&iface_out, 0, sizeof(iface_out));
  sock_rt = socket(AF_INET, SOCK_DGRAM, 0 );
  
  if (setsockopt(sock_rt, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on))
      == -1) {
    g_warning("[get_output_if] setsockopt(SOL_SOCKET, SO_BROADCAST): %s\n", strerror(errno));
    close(sock_rt);
    return -1;
  }
  
  if (connect(sock_rt, (struct sockaddr*)&remote, sizeof(struct sockaddr_in))
      == -1 ) {
    g_warning("[get_output_if] connect: %s\n", strerror(errno));
    close(sock_rt);
    return -1;
  }
  
  len = sizeof(iface_out);
  if (getsockname(sock_rt, (struct sockaddr *)&iface_out, &len) == -1 ) {
    g_warning("[get_output_if] getsockname: %s\n", strerror(errno));
    close(sock_rt);
    return -1;
  }

  close(sock_rt);
  if (iface_out.sin_addr.s_addr == 0)
    { /* what is this case?? */
      return -1;
    }
  osip_strncpy(address, inet_ntoa(iface_out.sin_addr), size-1);
  return 0;
}


/* This is a portable way to find the default gateway.
 * The ip of the default interface is returned.
 */
static int
ppl_dns_default_gateway_ipv6 (char *address, int size)
{
  unsigned int len;
  int sock_rt, on=1;
  struct sockaddr_in6 iface_out;
  struct sockaddr_in6 remote;
  
  memset(&remote, 0, sizeof(struct sockaddr_in6));

  remote.sin6_family = AF_INET6;
  inet_pton(AF_INET6, "2001:638:500:101:2e0:81ff:fe24:37c6",
	    &remote.sin6_addr);
  remote.sin6_port = htons(11111);
  
  memset(&iface_out, 0, sizeof(iface_out));
  sock_rt = socket(AF_INET6, SOCK_DGRAM, 0 );
  
  if (setsockopt(sock_rt, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on))
      == -1) {
    g_warning("[get_output_if] setsockopt(SOL_SOCKET, SO_BROADCAST: %s\n", strerror(errno));
    close(sock_rt);
    return -1;
  }
  
  if (connect(sock_rt, (struct sockaddr*)&remote, sizeof(struct sockaddr_in6))
      == -1 ) {
    g_warning("[get_output_if] connect: %s\n", strerror(errno));
    close(sock_rt);
    return -1;
  }
  
  len = sizeof(iface_out);
  if (getsockname(sock_rt, (struct sockaddr *)&iface_out, &len) == -1 ) {
    g_warning("DEBUG: [get_output_if] getsockname: %s\n", strerror(errno));
    close(sock_rt);
    return -1;
  }
  close(sock_rt);

  if (iface_out.sin6_addr.s6_addr == 0)
    { /* what is this case?? */
      return -1;
    }
  inet_ntop(AF_INET6, (const void*) &iface_out.sin6_addr, address, size-1);
  return 0;
}

int eXosip_get_localip_for(char *address_to_reach,char **loc){
	int err,tmp;
	struct addrinfo hints;
	struct addrinfo *res=NULL;
	struct sockaddr_storage addr;
	int sock;
	socklen_t s;
	
	if (eXosip.forced_localip){
		*loc=osip_strdup(eXosip.localip);
		return 0;
	}
	
	*loc=osip_malloc(MAXHOSTNAMELEN);
	if (eXosip.ip_family==AF_INET)
		strcpy(*loc,"127.0.0.1");  /* always fallback to local loopback */
	else strcpy(*loc,"::1"); 

	memset(&hints,0,sizeof(hints));
	hints.ai_family=(eXosip.ip_family==AF_INET) ? PF_INET:PF_INET6;
	hints.ai_socktype=SOCK_DGRAM;
	/*hints.ai_flags=AI_NUMERICHOST|AI_CANONNAME;*/
	err=getaddrinfo(address_to_reach,"5060",&hints,&res);
	if (err!=0){
		eXosip_trace(OSIP_ERROR,("Error in getaddrinfo for %s: %s\n",address_to_reach,gai_strerror(err)));
		if (err == EAI_AGAIN)
			return 1;
		else
			return -1;
	}
	if (res==NULL){
		eXosip_trace(OSIP_ERROR,("getaddrinfo reported nothing !"));
 		freeaddrinfo(res);
		return -1;
	}

	sock=socket(res->ai_family,SOCK_DGRAM,0);
	tmp=1;
	err=setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&tmp,sizeof(int));
	if (err<0){
		eXosip_trace(OSIP_ERROR,("Error in setsockopt: %s\n",strerror(errno)));
 		close(sock);
 		freeaddrinfo(res);
		return -1;
	}
	err=connect(sock,res->ai_addr,res->ai_addrlen);
	if (err<0) {
		eXosip_trace(OSIP_ERROR,("Error in connect: %s\n",strerror(errno)));
 		freeaddrinfo(res);
 		close(sock);
		return -1;
	}
	freeaddrinfo(res);
	res=NULL;

	s=sizeof(addr);
	err=getsockname(sock,(struct sockaddr*)&addr,&s);
	if (err!=0) {
		eXosip_trace(OSIP_ERROR,("Error in getsockname: %s\n",strerror(errno)));
		close(sock);
		return -1;
	}
	
	err=getnameinfo((struct sockaddr *)&addr,s,*loc,MAXHOSTNAMELEN,NULL,0,NI_NUMERICHOST);
	if (err!=0){
		eXosip_trace(OSIP_ERROR,("getnameinfo error:%s",strerror(errno)));
 		close(sock);
		return -1;
	}
	close(sock);
	eXosip_trace(OSIP_INFO1,("Outgoing interface to reach %s is %s.\n",address_to_reach,*loc));
	return 0;
}

#ifdef SM

void eXosip_get_localip_from_via(osip_message_t *mesg,char **locip){
	osip_via_t *via=NULL;
	char *host;
	via=(osip_via_t*)osip_list_get(mesg->vias,0);
	if (via==NULL) {
		host="15.128.128.93";
		eXosip_trace(OSIP_ERROR,("Could not get via:%s"));
	}else host=via->host;
	eXosip_get_localip_for(host,locip);
	
}
#endif

#ifdef MALLOC_DEBUG
char *__strdup_printf(char *file, int line, const char *fmt, ...)
#else
char *strdup_printf(const char *fmt, ...)
#endif
{
	/* Guess we need no more than 100 bytes. */
	int n, size = 100;
	char *p;
	va_list ap;
#ifdef MALLOC_DEBUG
	if ((p = __osip_malloc (size, file, line)) == NULL)
#else
	if ((p = osip_malloc (size)) == NULL)
#endif
		return NULL;
	while (1)
	{
		/* Try to print in the allocated space. */
		va_start (ap, fmt);
		n = vsnprintf (p, size, fmt, ap);
		va_end (ap);
		/* If that worked, return the string. */
		if (n > -1 && n < size)
			return p;
		/* Else try again with more space. */
		if (n > -1)	/* glibc 2.1 */
			size = n + 1;	/* precisely what is needed */
		else		/* glibc 2.0 */
			size *= 2;	/* twice the old size */
		if ((p = osip_realloc (p, size)) == NULL)
			return NULL;
	}
}

int
eXosip_get_addrinfo(struct addrinfo **addrinfo, char *hostname, int service)
{
#ifndef WIN32
  struct in_addr addr;
  struct in6_addr addrv6;
#else
bbbbbbbbbbbbbbb
  unsigned long int one_inet_addr;
#endif
  struct addrinfo hints;
  int error;
  char portbuf[10];
  char hbuf[NI_MAXHOST];

  if (service!=0)
    snprintf(portbuf, sizeof(portbuf), "%i", service);

  memset (&hints, 0, sizeof (hints));
#ifndef WIN32
 if (inet_pton(AF_INET, hostname, &addr)>0)
 {
   /* ipv4 address detected */
   hints.ai_flags = AI_NUMERICHOST;
   hints.ai_family = PF_INET;
   OSIP_TRACE (osip_trace
	       (__FILE__, __LINE__, OSIP_INFO2, NULL,
		"IPv4 address detected: %s\n", hostname));
 }
 else if (inet_pton(AF_INET6, hostname, &addrv6)>0)
 {
   /* ipv6 address detected */
   /* Do the resolution anyway */
   hints.ai_flags = AI_CANONNAME;
   hints.ai_family = PF_INET6;
   OSIP_TRACE (osip_trace
	       (__FILE__, __LINE__, OSIP_INFO2, NULL,
		"IPv6 address detected: %s\n", hostname));
 }
 else
 {
   /* hostname must be resolved */
//   hints.ai_flags = AI_CANONNAME;
//   hints.ai_flags = AI_NUMERICHOST;
   hints.ai_family = (eXosip.ip_family==AF_INET) ? PF_INET:PF_INET6;
   OSIP_TRACE (osip_trace
	       (__FILE__, __LINE__, OSIP_INFO2, NULL,
		"Not an IPv4 or IPv6 address: %s\n", hostname));
 }
#else
cccccccccccccccc
  if ((int)(one_inet_addr = inet_addr(hostname)) == -1)
    hints.ai_flags = AI_CANONNAME;
  else
    hints.ai_flags = AI_NUMERICHOST;

#ifdef IPV6_SUPPORT
aaaaaaaaaaaaaaaaa
  hints.ai_family = PF_UNSPEC; /* ipv6 support */
#else
  hints.ai_family = PF_INET;   /* ipv4 only support */
#endif

#endif

  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;
  if (service==0)
    {
      error = getaddrinfo (hostname, "sip", &hints, addrinfo);
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_INFO2, NULL,
		   "SRV resolution with udp-sip-%s\n", hostname));
    }
  else
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_INFO2, NULL,
		   "DNS resolution start: hostname=%s, port=%s\n", hostname, portbuf));
      error = getaddrinfo (hostname, portbuf, &hints, addrinfo);
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_INFO2, NULL,
		   "DNS resolution with %s:%i\n", hostname, service));
    }
  if (error || *addrinfo == NULL)
    { 
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_INFO2, NULL,
		   "getaddrinfo failure. %s:%s (%s)\n", hostname, portbuf, gai_strerror(error)));

	  if (error == EAI_AGAIN)
		  return 1;
	  else
		  return -1;
    }

  error = getnameinfo((*addrinfo)->ai_addr, (*addrinfo)->ai_addrlen, hbuf, sizeof(hbuf), NULL, 0, NI_NUMERICHOST);
  if (error == 0)
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_INFO2, NULL, "resolve ip is %s\n", hbuf));

  return 0;
}

int
eXosip_get_ip_by_name(unsigned long *ip, const char *hostname, int service)
{
  struct addrinfo *addrinfo;
  int ret;
  struct sockaddr_in addr;	/* in order to eliminate compiler warning */
  
  if( ( ret = eXosip_get_addrinfo(&addrinfo, ( char * )hostname, service) ) )
    return ret; /* not get an IP */
  
  if( sizeof( struct sockaddr_in ) != addrinfo ->ai_addrlen ) {
    ret = 1;
    g_warning( "sockaddr_in size not match\n" );
    goto label_free_and_end;
  }
  
  memcpy( &addr, addrinfo ->ai_addr, sizeof( struct sockaddr_in ) );
  
  *ip = addr.sin_addr.s_addr;

label_free_and_end:  
  freeaddrinfo(addrinfo);
  
  return ret;
}


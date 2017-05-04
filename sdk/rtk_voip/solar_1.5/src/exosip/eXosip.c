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

#include <sys/ioctl.h>
#include <net/if.h>  

#include "eXosip2.h"
#include "eXosip.h"
#include "eXosip_cfg.h"
//#include "../linphone/uglib.h"
#include "../linphone/linphonecore.h"

#include "../osip/osip_mt.h"
#include "../osip/osip_condv.h"

#include "voip_manager.h"
#include "stun_api.h"

#ifndef  WIN32
#include <memory.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#ifdef RTK_DNS
#include "dns_api.h"
#endif

#ifdef CONFIG_RTK_VOIP_SIP_TLS
#include	<netinet/tcp.h>
#include "eXosip_tls.h"
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

typedef enum { IP_ADDR, SUBNET_MASK, DEFAULT_GATEWAY, HW_ADDR } ADDR_T;

/* Private functions */
static int eXosip_create_transaction(eXosip_call_t *jc, eXosip_dialog_t *jd,
				     osip_message_t *request);
static int eXosip_create_cancel_transaction(eXosip_call_t *jc,
					    eXosip_dialog_t *jd,
					    osip_message_t *request);
static int eXosip_add_authentication_information(osip_message_t *req,
						 osip_message_t *last_response);
#ifndef DISABLE_THREAD
static void *eXosip_thread(void *arg);
static int eXosip_execute(void);
#else
int eXosip_execute(void);
#endif
static osip_message_t *eXosip_prepare_request_for_auth(int nPort, osip_message_t *msg);

static int eXosip_update_top_via(int nPort, osip_message_t *sip);


eXosip_t eXosip;
#ifdef CONFIG_RTK_VOIP_SIP_TLS
tlsObject_t sipTLSObj[MAX_VOIP_PORTS][MAX_PROXY];
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

void eXosip_set_firewallip(const char *firewall_address, int nPort)
{
	if (firewall_address==NULL) return;
	snprintf(eXosip.j_firewall_ip[nPort],50, "%s", firewall_address);

	if(eXosip.j_firewall_ip[nPort][0]!='\0')
		stun_message("eXosip.j_firewall_ip: %s\n", eXosip.j_firewall_ip[nPort]);
	else
		stun_message("eXosip.j_firewall_ip is not valid!\n");
}

/* Mandy add for stun support */
void eXosip_set_firewallport(const int nPort, const char *firewall_port)
{
	if (firewall_port==NULL) return;
	snprintf(eXosip.j_firewall_ports[nPort], 9, "%s", firewall_port);
}

void eXosip_set_firewall_media_port(const int nPort, const int firewall_media_port)
{
	if (firewall_media_port==0)
		eXosip.j_firewall_media_ports[nPort][0] = 0;
	else
		snprintf(eXosip.j_firewall_media_ports[nPort], 9, "%d", firewall_media_port);
}

/*++added by Jack Chan 05/03/07 for T.38++*/
void eXosip_set_faxport(const int nPort, const char *fax_port)
{
	if (fax_port==NULL) return;
	snprintf(eXosip.fax_ports[nPort], 9, "%s", fax_port);
}

void eXosip_set_firewall_faxport(const int nPort, const int firewall_faxport)
{
	if (firewall_faxport==0)
		eXosip.j_firewall_fax_ports[nPort][0] = 0;
	else
		snprintf(eXosip.j_firewall_fax_ports[nPort], 9, "%d", firewall_faxport);
}
/*--end--*/

void eXosip_set_nattype(const char *nat_type)
{
    osip_strncpy(eXosip.nat_type, (nat_type ? nat_type : ""), sizeof(eXosip.nat_type)-1);
}

int eXosip_guess_localip(int family, char *address, int size)
{
#if 1
  return eXosip_guess_ip_for_via(family, address, size);
#else
struct in_addr curIpAddr;
char *BRIDGE_IF = "br0";


	/* get current used IP */
	if ( !get_InAddr(BRIDGE_IF, IP_ADDR, (void *)&curIpAddr) ) {
		printf("We can't get current used ip\n");
		return FALSE;
	}

  osip_strncpy(address, inet_ntoa(curIpAddr), size-1);
  return TRUE;
#endif
}

void eXosip_get_localip(char *ip)
{
  OSIP_TRACE (osip_trace
	      (__FILE__, __LINE__, OSIP_ERROR, NULL,
	       "eXosip_get_localip IS DEPRECATED. Use eXosip_guess_localip!\n"));
  eXosip_guess_ip_for_via(AF_INET, ip, 15);
}

#if 0
int get_InAddr( char *interface_name, ADDR_T type, void *pAddr )
{
    struct ifreq ifr;
    int skfd, found=0;
    struct sockaddr_in *addr;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

    strcpy(ifr.ifr_name, interface_name);
    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
	return (0);

    if (type == HW_ADDR) {
    	if (ioctl(skfd, SIOCGIFHWADDR, &ifr) >= 0) {
		memcpy(pAddr, &ifr.ifr_hwaddr, sizeof(struct sockaddr));
		found = 1;
	}
    }
    else if (type == IP_ADDR) {
	if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0) {
		addr = (struct sockaddr_in *) ((void *) &ifr.ifr_addr);
		*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);
		found = 1;
	}
    }
    else if (type == SUBNET_MASK) {
	if (ioctl(skfd, SIOCGIFNETMASK, &ifr) >= 0) {
		addr = (struct sockaddr_in *) ((void*) &ifr.ifr_addr);
		*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);
		found = 1;
	}
    }
    close( skfd );
    return found;

}
#endif

int 
eXosip_is_public_address(const char *c_address)
{
#if 0
    return (0!=strncmp(c_address, "192.168",7)
	    && 0!=strncmp(c_address, "10.",3)
	    && 0!=strncmp(c_address, "172.16.",7)
	    && 0!=strncmp(c_address, "172.17.",7)
	    && 0!=strncmp(c_address, "172.18.",7)
	    && 0!=strncmp(c_address, "172.19.",7)
	    && 0!=strncmp(c_address, "172.20.",7)
//	    && 0!=strncmp(c_address, "172.21.",7)
	    && 0!=strncmp(c_address, "172.22.",7)
	    && 0!=strncmp(c_address, "172.23.",7)
	    && 0!=strncmp(c_address, "172.24.",7)
	    && 0!=strncmp(c_address, "172.25.",7)
	    && 0!=strncmp(c_address, "172.26.",7)
	    && 0!=strncmp(c_address, "172.27.",7)
	    && 0!=strncmp(c_address, "172.28.",7)
	    && 0!=strncmp(c_address, "172.29.",7)
	    && 0!=strncmp(c_address, "172.30.",7)
	    && 0!=strncmp(c_address, "172.31.",7)
	    && 0!=strncmp(c_address, "169.254",7));
#endif
#if 0
 /* Mandy add for stun support */
unsigned char ipAddr[4];
struct in_addr curIpAddr, curSubnet;
unsigned long v1, v2, v3;
char Netinterface[32];


	inet_aton(c_address, (struct in_addr *) ((void *)&ipAddr));

	/* get the device */
	if(!eXosip_get_DeviceName(c_address, Netinterface)){
		g_warning("cannot fine the device name associated with the ip: %s",c_address);
		return FALSE;
	}
	else {
		g_warning("find the device name = %s, associated with the ip: %s", Netinterface, c_address);
	}
	
	/* get current used IP */
	if ( !get_InAddr(Netinterface, IP_ADDR, (void *)&curIpAddr) ) {
		g_warning("We can't get current used ip\n");
		return FALSE;
	}
	

	/* get current used netmask */
	if ( !get_InAddr(Netinterface, SUBNET_MASK, (void *)&curSubnet )) {
		g_warning("We can't get current used netmask\n");
		return FALSE;
	}

	v1 = *((unsigned long *)((void *)ipAddr));
	v2 = *((unsigned long *)&curIpAddr);
	v3 = *((unsigned long *)&curSubnet);

	if ( (v1 & v3) != (v2 & v3) ) {
		g_message("Public IP address! It is not within the same subnet.");
		return TRUE;
	}

	return FALSE;
#endif
#if 1
	struct ifconf ifc;
	struct ifreq *ifr;
	char *ptr;
	int n, e;
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	int len = 100 * sizeof(struct ifreq);
	char buf[len];
	unsigned long lai;

	// get remote ip 
	if (inet_aton(c_address, (struct in_addr *) &lai) == 0)
	{
		fprintf(stderr, "inet_aton failed: input address is %s\n", c_address);
		close(s);
		return 0; // assume private address if error
	}

	ifc.ifc_len = len;
	ifc.ifc_buf = buf;
	e = ioctl(s, SIOCGIFCONF, &ifc);
	if (e == -1)
	{
		g_warning("get SIOCGIFCONF failed: %s\n", strerror(errno));
		close(s);
		return 0; // assume private address if error
	}

	ifr = ifc.ifc_req;
	for (n = 0; n < ifc.ifc_len; n += sizeof(struct ifreq))
	{
		unsigned long ai, mi;
		
//		printf("Get interface %s\n", ifr->ifr_name);

		// get flags
		e = ioctl(s, SIOCGIFFLAGS, ifr);
		if (e == -1)
		{
			g_warning("get SIOCGIFFLAGS failed: %s\n", strerror(errno));
			close(s);
			return 0; // assume private address if error
		}

//		printf("%s: flags is %x\n", ifr->ifr_name, ifr->ifr_flags);
		if ((ifr->ifr_flags & IFF_UP) == 0)
		{
//			printf("%s: NOT UP Link\n", ifr->ifr_name);
			continue;
		}

		// get local ip 
		e = ioctl(s, SIOCGIFADDR, ifr);
		if (e == -1)
		{
			g_warning("get SIOCGIFADDR failed: %s\n", strerror(errno));
			close(s);
			return 0; // assume private address if error
		}
      	ai = ntohl(((struct sockaddr_in*)&ifr->ifr_addr)->sin_addr.s_addr);
		
		// get local mask
		e = ioctl(s, SIOCGIFNETMASK, ifr);
		if (e == -1)
		{
			g_warning("get SIOCGIFNETMASK failed: %s\n", strerror(errno));
			close(s);
			return 0; // assume private address if error
		}

      	mi = ntohl(((struct sockaddr_in*)&ifr->ifr_addr)->sin_addr.s_addr);

		// check the same subnet	
		if ( (lai & mi) == (ai & mi) ) 
		{
			// found some interface use the same subnet!
//			printf("interface %s use the same subnet with address %s\n", name, c_address);
			close(s);
			return 0;
		}
		ifr++;
	}

	// assume public ip if all interface are not the same subnet
	close(s);
	return 1;
#endif
}


void __eXosip_wakeup(void)
{
  jpipe_write(eXosip.j_socketctl, "w", 1);
}

void __eXosip_wakeup_event(void)
{
  // rock: As no reader in linphone, it cause blocekd problem!
  // jpipe_write(eXosip.j_socketctl_event, "w", 1);
}

#ifndef DISABLE_THREAD

#ifdef MUTEX_DEBUG

int
__eXosip_lock(char *file, int line)
{
  return __osip_mutex_lock((struct osip_mutex*)eXosip.j_mutexlock, file, line);
}

int
__eXosip_unlock(char *file, int line)
{
  return __osip_mutex_unlock((struct osip_mutex*)eXosip.j_mutexlock, file, line);
}

#else

int
eXosip_lock(void)
{
  return osip_mutex_lock((struct osip_mutex*)eXosip.j_mutexlock);
}

int
eXosip_unlock(void)
{
  return osip_mutex_unlock((struct osip_mutex*)eXosip.j_mutexlock);
}

#endif

#endif // DISABLE_THREAD

jfriend_t *jfriend_get(void)
{
  return eXosip.j_friends;
}

void jfriend_remove(jfriend_t *fr)
{
  REMOVE_ELEMENT(eXosip.j_friends, fr);
}

jsubscriber_t *jsubscriber_get(void)
{
  return eXosip.j_subscribers;
}

jidentity_t *jidentity_get(void)
{
  return eXosip.j_identitys;
}

void
eXosip_kill_transaction (osip_list_t * transactions)
{
  osip_transaction_t *transaction;

  if (!osip_list_eol (transactions, 0))
    {
      /* some transaction are still used by osip,
         transaction should be released by modules! */
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
		   "module sfp: _osip_kill_transaction transaction should be released by modules!\n"));
    }

  while (!osip_list_eol (transactions, 0))
    {
      transaction = osip_list_get (transactions, 0);

      __eXosip_delete_jinfo(transaction);
      osip_transaction_free (transaction);
    }
}

void eXosip_quit(void)
{
  jauthinfo_t   *jauthinfo;
  eXosip_call_t *jc;
  eXosip_notify_t *jn;
  eXosip_subscribe_t *js;
  eXosip_reg_t  *jreg;
  eXosip_pub_t  *jpub;
#if !defined(DISABLE_THREAD) || defined(NEW_STUN)
  int i;
#endif
  int nPort;

  eXosip.j_stop_ua = 1; /* ask to quit the application */
  __eXosip_wakeup();
  __eXosip_wakeup_event();

#ifndef DISABLE_THREAD
  i = osip_thread_join((struct osip_thread*)eXosip.j_thread);
  if (i!=0)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: can't terminate thread!"));
    }
  osip_free((struct osip_thread*)eXosip.j_thread);
#endif

  jpipe_close(eXosip.j_socketctl);
  jpipe_close(eXosip.j_socketctl_event);

  osip_free(eXosip.localip);
  for (nPort=0; nPort<g_MaxVoIPPorts; nPort++)
  {
    osip_free(eXosip.localports[nPort]);
    osip_free(eXosip.media_ports[nPort]);
  }
  osip_free(eXosip.user_agent);

  eXosip.j_input = 0;
  eXosip.j_output = 0;

  for (jc = eXosip.j_calls; jc!=NULL;jc = eXosip.j_calls)
    {
      REMOVE_ELEMENT(eXosip.j_calls, jc);
      eXosip_call_free(jc);
    }

  for (js = eXosip.j_subscribes; js!=NULL;js = eXosip.j_subscribes)
    {
      REMOVE_ELEMENT(eXosip.j_subscribes, js);
      eXosip_subscribe_free(js);
    }

  for (jn = eXosip.j_notifies; jn!=NULL;jn = eXosip.j_notifies)
    {
      REMOVE_ELEMENT(eXosip.j_notifies, jn);
      eXosip_notify_free(jn);
    }
  
#ifndef DISABLE_THREAD
  osip_mutex_destroy((struct osip_mutex*)eXosip.j_mutexlock);
  osip_cond_destroy((struct osip_cond*)eXosip.j_cond);
#endif

#ifdef DYNAMIC_PAYLOAD
#else
  eXosip_sdp_negotiation_free(eXosip.osip_negotiation);
#endif // DYNAMIC_PAYLOAD

  if (eXosip.j_input)
    fclose(eXosip.j_input);
  if (eXosip.j_output)
    osip_free(eXosip.j_output);

  for (nPort=0; nPort<g_MaxVoIPPorts; nPort++)
  {
    if (eXosip.j_sockets[nPort])
      close(eXosip.j_sockets[nPort]);

#ifdef CONFIG_RTK_VOIP_SIP_TLS
	for(i = 0; i < MAX_PROXY; i++)
		if (eXosip.j_tls_sockets[nPort][i] != -1){
			close(eXosip.j_tls_sockets[nPort][i]);
			tls_free(sipTLSObj[nPort][i]);
		}
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

#ifdef NEW_STUN
    for (i=0; i<MAX_SS; i++)
    {
      if (eXosip.j_media_sockets[nPort * 2 + i] != -1)
      {
        close(eXosip.j_media_sockets[nPort * 2 + i]);
        eXosip.j_media_sockets[nPort * 2 + i] = -1;
      }
    }

    if (eXosip.j_t38_sockets[nPort] != -1)
    {
      close(eXosip.j_t38_sockets[nPort]);
      eXosip.j_t38_sockets[nPort] = -1;
    }
#endif
  }

  for (jreg = eXosip.j_reg; jreg!=NULL; jreg = eXosip.j_reg)
    {
      REMOVE_ELEMENT(eXosip.j_reg, jreg);
      eXosip_reg_free(jreg);
    }

  for (jpub = eXosip.j_pub; jpub!=NULL; jpub = eXosip.j_pub)
    {
      REMOVE_ELEMENT(eXosip.j_pub, jpub);
      _eXosip_pub_free(jpub);
    }

  /* should be moved to method with an argument */
  jfriend_unload();
  jidentity_unload();
  jsubscriber_unload();

  /*
  for (jid = eXosip.j_identitys; jid!=NULL; jid = eXosip.j_identitys)
    {
      REMOVE_ELEMENT(eXosip.j_identitys, jid);
      eXosip_friend_free(jid);
    }

  for (jfr = eXosip.j_friends; jfr!=NULL; jfr = eXosip.j_friends)
    {
      REMOVE_ELEMENT(eXosip.j_friends, jfr);
      eXosip_reg_free(jfr);
    }
  */

  while (!osip_list_eol(eXosip.j_transactions, 0))
    {
      osip_transaction_t *tr = (osip_transaction_t*) osip_list_get(eXosip.j_transactions, 0);
      if (tr->state==IST_TERMINATED || tr->state==ICT_TERMINATED
	  || tr->state== NICT_TERMINATED || tr->state==NIST_TERMINATED)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,
		      "Release a terminated transaction\n"));
	  osip_list_remove(eXosip.j_transactions, 0);
	  __eXosip_delete_jinfo(tr);
	  osip_transaction_free(tr);
	}
      else
	{
	  osip_list_remove(eXosip.j_transactions, 0);
	  __eXosip_delete_jinfo(tr);
	  osip_transaction_free(tr);
	}
    }

  osip_free(eXosip.j_transactions);

  eXosip_kill_transaction (eXosip.j_osip->osip_ict_transactions);
  eXosip_kill_transaction (eXosip.j_osip->osip_nict_transactions);
  eXosip_kill_transaction (eXosip.j_osip->osip_ist_transactions);
  eXosip_kill_transaction (eXosip.j_osip->osip_nist_transactions);
  osip_release (eXosip.j_osip);

  if (eXosip.j_events!=NULL)
    {
      eXosip_event_t *ev;
      for(ev=osip_fifo_tryget(eXosip.j_events);ev!=NULL;
	  ev=osip_fifo_tryget(eXosip.j_events))
	eXosip_event_free(ev);
    }

  osip_fifo_free(eXosip.j_events);

  for (jauthinfo = eXosip.authinfos; jauthinfo!=NULL;
       jauthinfo = eXosip.authinfos)
    {
      REMOVE_ELEMENT(eXosip.authinfos, jauthinfo);
      osip_free(jauthinfo);
    }

  return ;
}

#ifdef DISABLE_THREAD

#ifdef NEW_STUN
#ifdef CONFIG_RTK_VOIP_SIP_TLS
int eXosip_read_message(LinphoneCore *lc, char **sip_buf, int *sip_len, int socket_type, int proxyIndex)
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
int eXosip_read_message(LinphoneCore *lc, char **sip_buf, int *sip_len)
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

#else
int eXosip_read_message(LinphoneCore *lc)
#endif
{
	struct sockaddr_storage sa;
	socklen_t slen;
	char *buf;
	int err;

	buf = (char *)osip_malloc(SIP_MESSAGE_MAX_LENGTH*sizeof(char)+1);
	if (buf == NULL)
		g_error("malloc SIP_MESSAGE_MAX_LENGTH failed\n");

	slen = sizeof(struct sockaddr_storage);
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	if(socket_type == TRANSPORT_TLS){
		/*read the data in tls transport*/
		err=tls_read_message(sipTLSObj[lc->chid][proxyIndex].ssl_conn, buf, SIP_MESSAGE_MAX_LENGTH);
		if(err > SIP_MESSAGE_MAX_LENGTH){
			g_error("buffer over flow!\n");
			exit(1);
		}
		if(-1 != err)
			if(0 > getpeername(eXosip.j_tls_sockets[lc->chid][proxyIndex], (struct sockaddr *) &sa, &slen))
				g_warning("getpeername Error! %s\n", strerror(errno));
	}else{
		err = recvfrom (eXosip.j_sockets[lc->chid], buf, SIP_MESSAGE_MAX_LENGTH, 0,
			(struct sockaddr *) &sa, &slen);
	}
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
	err = recvfrom (eXosip.j_sockets[lc->chid], buf, SIP_MESSAGE_MAX_LENGTH, 0,
			(struct sockaddr *) &sa, &slen);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

	if (err > 5)
	{
		osip_transaction_t *transaction = NULL;
		osip_event_t *sipevent;
          
		osip_strncpy(buf + err,"\0",1);
          
		OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,
			"Received message: \n%s\n", buf));

		sipevent = osip_parse(buf, err);
		transaction = NULL;
		if (sipevent!=NULL && sipevent->sip!=NULL)
		{
			char out_addr[INET6_ADDRSTRLEN];

			osip_message_fix_last_via_header(
				sipevent->sip, 
				inet_ntop(
					AF_INET,	
					(void *)&(((struct sockaddr_in *)&sa)->sin_addr).s_addr, 
					out_addr, 
					INET_ADDRSTRLEN
				),
				ntohs(((struct sockaddr_in *)&sa)->sin_port)
			);

			err = osip_find_transaction_and_add_event(eXosip.j_osip, sipevent);
			if (err != 0)
			{
				OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,
					"This is a request\n", buf));
				eXosip_lock();
				if (MSG_IS_REQUEST(sipevent->sip))
				{
					eXosip_process_newrequest(lc->chid, sipevent);
				}
				else if (MSG_IS_RESPONSE(sipevent->sip))
				{
					eXosip_process_response_out_of_transaction(sipevent);
				}
				eXosip_unlock();
			}
			else
			{
				/* handled by oSIP !*/
				/* Mandy add for stun support */
				#if 1
	   			if (MSG_IS_RESPONSE_FOR(sipevent->sip, "INVITE") && MSG_IS_STATUS_2XX(sipevent->sip)) {
				    //printf( "\n MSG_IS_RESPONSE_FOR INVITE" );
                    linphone_core_set_use_info_for_stun(lc, -1, STUN_UPDATE_SIP);
				}	
				#endif
                /* End of Mandy add for stun support */
			}

			osip_free(buf);
			return 1;
		}
		else
		{
			OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL, "Could not parse SIP message\n"));
			osip_event_free(sipevent);
		}
	}
	else if (err<0)
	{
		OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL, "Could not read socket\n"));
	}
	else
	{
		OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL, "Dummy SIP message received\n"));
	}

#ifdef NEW_STUN
	*sip_buf = buf;
	*sip_len = err;
#else		
	osip_free(buf);
#endif
	return 0;
}

int eXosip_process_icmp_message( unsigned long ip, unsigned short port )
{
	eXosip_lock();

	osip_icmp_ict_execute(eXosip.j_osip, eXosip_get_ip_by_name, ip, port);
	osip_icmp_nict_execute(eXosip.j_osip, eXosip_get_ip_by_name, ip, port);

	eXosip_unlock();

	return 0;
}

int eXosip_execute(void)
{
#ifdef MEASURE_TIME
	struct timeval start, end;
	int waste_time;
	clock_t c_start, c_end;

	fprintf(stderr, "\n *** eXosip_execute start *** \n");
 	osip_gettimeofday (&start, NULL);
	c_start = clock();
#endif

	eXosip_lock();

	osip_timers_ict_execute(eXosip.j_osip);
	osip_timers_nict_execute(eXosip.j_osip);
	osip_timers_ist_execute(eXosip.j_osip);
	osip_timers_nist_execute(eXosip.j_osip);
  
	osip_ict_execute(eXosip.j_osip);
	osip_nict_execute(eXosip.j_osip);
	osip_ist_execute(eXosip.j_osip);
	osip_nist_execute(eXosip.j_osip);
  
	osip_retransmissions_execute(eXosip.j_osip);

	/* free all Calls that are in the TERMINATED STATE? */
	eXosip_release_terminated_calls();

	eXosip_unlock();

#ifdef MEASURE_TIME
	osip_gettimeofday (&end, NULL);
	c_end = clock();

	if (end.tv_usec >= start.tv_usec)
		waste_time = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
	else
		waste_time = (end.tv_sec - start.tv_sec - 1) * 1000 + (1000000 + end.tv_usec - start.tv_usec) / 1000;

	if (waste_time)
	{
		fprintf(stderr, "\n *** eXosip_execute waste %d ms (%ld) *** \n", waste_time,
			(c_end - c_start) / (CLOCKS_PER_SEC / 1000));
	}
#endif

	return 0;
}

#else

static int eXosip_execute ( void )
{
  struct timeval lower_tv;
  int i;

  osip_timers_gettimeout(eXosip.j_osip, &lower_tv);
  if (lower_tv.tv_sec>15)
    {
      lower_tv.tv_sec = 15;
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_INFO1, NULL,
		   "eXosip: Reseting timer to 15s before waking up!\n"));
    }
  else
    {
        /*  add a small amount of time on windows to avoid
            waking up too early. (probably a bad time precision) */
        if (lower_tv.tv_usec<900000)
            lower_tv.tv_usec = 100000; /* add 10ms */
        else 
        {
            lower_tv.tv_usec = 10000; /* add 10ms */
            lower_tv.tv_sec++;
        }
#ifdef WATCH_DOG
#else
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_INFO1, NULL,
		   "eXosip: timer sec:%i usec:%i!\n",
		   lower_tv.tv_sec, lower_tv.tv_usec));
#endif
    }
  i = eXosip_read_message(1, lower_tv.tv_sec, lower_tv.tv_usec);

  if (i==-2)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_WARNING, NULL,
		   "eXosip: eXosip_read_message failed.\n"));
      return -2;
    }

  eXosip_lock();
  osip_timers_ict_execute(eXosip.j_osip);
  osip_timers_nict_execute(eXosip.j_osip);
  osip_timers_ist_execute(eXosip.j_osip);
  osip_timers_nist_execute(eXosip.j_osip);
  
  osip_ict_execute(eXosip.j_osip);
  osip_nict_execute(eXosip.j_osip);
  osip_ist_execute(eXosip.j_osip);
  osip_nist_execute(eXosip.j_osip);
  
#if 1
  osip_retransmissions_execute(eXosip.j_osip);
#endif

  /* free all Calls that are in the TERMINATED STATE? */
  eXosip_release_terminated_calls();

  eXosip_unlock();

  return 0;
}

void *eXosip_thread        ( void *arg )
{
  int i;
  while (eXosip.j_stop_ua==0)
    {
      i = eXosip_execute();
      if (i==-2)
	osip_thread_exit();
    }
  osip_thread_exit();
  return NULL;
}

#endif // DISABLE_THREAD

static int ipv6_enable = 0;

void eXosip_enable_ipv6(int _ipv6_enable)
{
  ipv6_enable = _ipv6_enable;
}

#ifdef DYNAMIC_PAYLOAD
#else
typedef struct {
	int number;
	char pt[5];
	char string[20];
} support_media_t;

static support_media_t g711u =
{
	number: 0, 
	pt: "0", 
	string: "0 PCMU/8000"
};

static support_media_t g711a =
{
	number: 8,
	pt: "8",
	string: "8 PCMA/8000"
};

static support_media_t g723 =
{
	number: 4,
	pt: "4",
	string: "4 G723/8000"
};

static support_media_t g729 =
{
	number: 18,
	pt: "18",
	string: "18 G729/8000"
};

static support_media_t g726_16 =
{
	number: 23,
	pt: "23",
	string: "23 G726-16/8000"
};

static support_media_t g726_24 =
{
	number: 22,
	pt: "22",
	string: "22 G726-24/8000"
};

static support_media_t g726_32 =
{
	number: 2,
	pt: "2",
	string: "2 G726-32/8000"
};

static support_media_t g726_40 =
{
	number: 21,
	pt: "21",
	string: "21 G726-40/8000"
};

static support_media_t gsm_fr =
{
	number: 3,
	pt: "3",
	string: "3 GSM/8000"
};

static support_media_t *support_media_list[_CODEC_MAX] = 
{
	&g711u, 
	&g711a, 
	&g723,
	&g729,
	&g726_16, 
	&g726_24, 
	&g726_32, 
	&g726_40,
	&gsm_fr,
};
#endif // DYNAMIC_PAYLOAD

int eXosip_bind_socket(int *pfd, int port)
{
	int i, fd;
	struct sockaddr_in raddr;
#ifdef SUPPORT_DSCP
	int priority;
#endif

	if (port == 0)
	{
		fd = -1;
		return TRUE;
	}

	fd = (int)socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd == -1)
	{
		return FALSE;
	}

	memset(&raddr, 0, sizeof(raddr));     
	raddr.sin_addr.s_addr = htonl(INADDR_ANY);
	raddr.sin_port = htons((short) port);
	raddr.sin_family = AF_INET;
	i = bind(fd, (struct sockaddr *) &raddr, sizeof(raddr));
	if (i < 0)
	{
		OSIP_TRACE (osip_trace
			(__FILE__, __LINE__, OSIP_ERROR, NULL,
			"eXosip: Cannot bind on port %d: %i!\n", port, i));
		close(fd);
		return FALSE;
	}

#ifdef SUPPORT_DSCP
	// add voip priority (for vlan tag in the future)
	priority = 7;
	setsockopt(fd, SOL_SOCKET, SO_PRIORITY, &priority, sizeof(priority));
	// add voip tos
	priority = qos2tos(g_pVoIPCfg->sipDscp);
	setsockopt(fd, IPPROTO_IP, IP_TOS, &priority, sizeof(priority)); 
#endif

	*pfd = fd;
	return TRUE;
}

#ifdef CONFIG_RTK_VOIP_SIP_TLS
int eXosip_create_TCP_socket(int *pfd, int port, int nPort, int proxyIndex)
{
	int i, fd;
	struct sockaddr_in raddr;
	struct addrinfo *proxy_addrinfo;
	int addrReuse=1;
	char *proxyAddr=NULL;
	int proxyPort=0;

	if (port == 0)
	{
		fd = -1;
		*pfd = fd;
		return TRUE;
	}

	fd = (int)socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd == -1)
	{
		g_warning("open socket error: %s\n", strerror(errno));
		*pfd = fd;
		return FALSE;
	}

	/*avoid TCP reconnect and bind error*/
	if(-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &addrReuse, sizeof(addrReuse))){
		g_warning("set SO_REUSEADDR error: %s\n", strerror(errno));
		fd=-1;
		*pfd = fd;
		return FALSE;
	}
	
	memset(&raddr, 0, sizeof(raddr));     
	raddr.sin_addr.s_addr = htonl(INADDR_ANY);
	raddr.sin_port = htons((short) port);
	raddr.sin_family = AF_INET;
	i = bind(fd, (struct sockaddr *) &raddr, sizeof(raddr));
	if (i < 0)
	{
		g_warning("bind socket error: %s\n", strerror(errno));
		OSIP_TRACE (osip_trace
			(__FILE__, __LINE__, OSIP_ERROR, NULL,
			"eXosip: Cannot bind on port %d: %i!\n", port, i));
		close(fd);
		fd=-1;
		*pfd = fd;
		return FALSE;
	}
	/*connect to the proxy server*/
	if(g_pVoIPCfg->ports[nPort].proxies[proxyIndex].outbound_enable){
		proxyAddr=g_pVoIPCfg->ports[nPort].proxies[proxyIndex].outbound_addr;
		proxyPort=g_pVoIPCfg->ports[nPort].proxies[proxyIndex].outbound_port;
	}else if(g_pVoIPCfg->ports[nPort].proxies[proxyIndex].enable){
		proxyAddr=g_pVoIPCfg->ports[nPort].proxies[proxyIndex].addr;
		proxyPort=g_pVoIPCfg->ports[nPort].proxies[proxyIndex].port;
	}

	/*do DNS convert*/
	while(1){
		i = eXosip_get_addrinfo(&proxy_addrinfo, proxyAddr, proxyPort);
		if(i == 1){ //retry
			freeaddrinfo(proxy_addrinfo);
			continue;
		}else if(i == -1){ //fail
			close(fd);
			fd=-1;
			*pfd = fd;
			g_error("convert proxy address fail!!\n");
			freeaddrinfo(proxy_addrinfo);
			return FALSE;
		}else{
			break;
		}
	}
	if(proxyAddr != NULL && proxyPort != 0 && g_pVoIPCfg->ports[nPort].proxies[proxyIndex].siptls_enable){		
		if((connect(fd, proxy_addrinfo->ai_addr, proxy_addrinfo->ai_addrlen))==-1){
			g_warning("connect to proxy server error (%s)!!!\n", strerror(errno));
			close(fd);
			freeaddrinfo(proxy_addrinfo);
			fd=-1;
			*pfd = fd;
			return FALSE;
		}
	}
	freeaddrinfo(proxy_addrinfo);
	*pfd = fd;
	return TRUE;
}

int eXosip_check_tls_proxy(int nPort, char *proxy){
	int j;
	
	/*find out the tls sip proxy*/
	for(j = 0; j < MAX_PROXY; j++){
		if(g_pVoIPCfg->ports[nPort].proxies[j].siptls_enable && !strcmp(proxy, g_pVoIPCfg->ports[nPort].proxies[j].addr) && eXosip.j_tls_sockets[nPort][j] != -1){
			return TRUE;
		}
	}
	
	return FALSE;
}

TRANSPORT_TYPE eXosip_get_transport_type(osip_message_t *sip){
	osip_via_t *via=NULL;

	osip_message_get_via(sip, 0, &via);
	if(via != NULL){
		if(!osip_strcasecmp(via->protocol,"UDP"))
			return TRANSPORT_UDP;
		else if(!osip_strcasecmp(via->protocol,"TCP"))
			return TRANSPORT_TCP;
		else if(!osip_strcasecmp(via->protocol,"TLS"))
			return TRANSPORT_TLS;
	}

	return TRANSPORT_ERROR;
}

int eXosip_get_TCP_status(int fd){
	struct tcp_info tcp_info;
	int tcp_info_length;

	tcp_info_length = sizeof(tcp_info);
	if (getsockopt(fd, SOL_TCP, TCP_INFO, (void *)&tcp_info, (socklen_t *)&tcp_info_length ) == 0 ) {
		return tcp_info.tcpi_state;
	}else{
		return -1;
	}
}

int eXosip_find_TCP_socket_form_proxy(int nPort, char *proxy){
	int proxyIndex;

	for(proxyIndex = 0; proxyIndex < MAX_PROXY; proxyIndex++){
		//if(strlen(g_pVoIPCfg->ports[nPort].proxies[proxyIndex].domain_name) != 0){
			/*compare with domain proxy*/
			/*if(g_pVoIPCfg->ports[nPort].proxies[proxyIndex].siptls_enable && 
				!osip_strcasecmp(g_pVoIPCfg->ports[nPort].proxies[proxyIndex].domain_name, proxy))
				return proxyIndex;
		}else{*/
			/*compare with proxy*/
			if(g_pVoIPCfg->ports[nPort].proxies[proxyIndex].siptls_enable && 
				!osip_strcasecmp(g_pVoIPCfg->ports[nPort].proxies[proxyIndex].addr, proxy) &&
				eXosip.j_tls_sockets[nPort][proxyIndex] != -1)	
				return proxyIndex;
		//}
	}

	return -1; //not found
}

proxy_info exosip_get_proxy_info(int nPort, char *proxy, int port){
	int i;
	proxy_info proxyInfo;
	
	for(i = 0; i < MAX_PROXY; i++){
			if(port){
				if(!strcmp(proxy, g_pVoIPCfg->ports[nPort].proxies[i].addr) &&
				port == g_pVoIPCfg->ports[nPort].proxies[i].port){
					proxyInfo.proxyIndex=i;
					if(g_pVoIPCfg->ports[nPort].proxies[i].siptls_enable &&
					eXosip.j_tls_sockets[nPort][i] != -1)	
						proxyInfo.transportType=TRANSPORT_TLS;
					else
						proxyInfo.transportType=TRANSPORT_UDP;
					return proxyInfo;
				}
			}else{
				if(!strcmp(proxy, g_pVoIPCfg->ports[nPort].proxies[i].addr)){
					proxyInfo.proxyIndex=i;
					if(g_pVoIPCfg->ports[nPort].proxies[i].siptls_enable &&
					eXosip.j_tls_sockets[nPort][i] != -1)	
						proxyInfo.transportType=TRANSPORT_TLS;
					else
						proxyInfo.transportType=TRANSPORT_UDP;
					return proxyInfo;
				}
			}
	}

	proxyInfo.proxyIndex=-1;
	return proxyInfo;
}
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

#ifdef FIX_RESPONSE_TIME

eXosip_callback_t *g_registered_cb = NULL;

void eXosip_register_callback(eXosip_callback_t *cb)
{
	g_registered_cb = cb;
}

int eXosip_init(void *user, FILE *input, FILE *output, int ports[], int media_ports[], int t38_ports[])
#else
int eXosip_init(FILE *input, FILE *output, int ports[], int media_ports[], int t38_ports[])
#endif
{
  osip_t *osip;
  int i, nPort;

  for (nPort=0; nPort<g_MaxVoIPPorts; nPort++)
  {
    if (ports[nPort]<=0)
    {
      OSIP_TRACE (osip_trace
        (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: port must be higher than 0!\n"));
      return -1;
    }
  }
 
  memset(&eXosip, 0, sizeof(eXosip));
  eXosip.localip = (char *) osip_malloc(50);
  memset(eXosip.localip, '\0', 50);

#ifdef FIX_RESPONSE_TIME
  eXosip.user = user;
#endif

  if (ipv6_enable == 0)
    eXosip.ip_family = AF_INET;
  else
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_INFO2, NULL,
		   "IPv6 is enabled. Pls report bugs\n"));
      eXosip.ip_family = AF_INET6;
    }

  eXosip_guess_localip(eXosip.ip_family, eXosip.localip, 49);
  if (eXosip.localip[0]=='\0')
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: No ethernet interface found!\n"));
      OSIP_TRACE (osip_trace
        (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: using 127.0.0.1 (debug mode)!\n"));
      strcpy(eXosip.localip, "127.0.0.1");
		/* we should always fallback on something. The linphone user will surely
		start linphone BEFORE setting its dial up connection.*/
    }
  eXosip.user_agent = osip_strdup("eXosip/" EXOSIP_VERSION);


#ifdef FIX_RESPONSE_TIME
  if (g_registered_cb == NULL)
    g_error("Install eXosip callback functions first!\n");
  eXosip.j_call_callbacks = g_registered_cb;
#endif

  eXosip_set_mode(EVENT_MODE);
  eXosip.j_input = input;
  eXosip.j_output = output;
  eXosip.j_calls = NULL;
  eXosip.j_stop_ua = 0;
#ifndef DISABLE_THREAD
  eXosip.j_thread = NULL;
#endif
  eXosip.j_transactions = (osip_list_t*) osip_malloc(sizeof(osip_list_t));
  osip_list_init(eXosip.j_transactions);
  eXosip.j_reg = NULL;

#ifndef DISABLE_THREAD
  eXosip.j_cond      = (struct osip_cond*)osip_cond_init();
  eXosip.j_mutexlock = (struct osip_mutex*)osip_mutex_init();
#endif

  if (-1==osip_init(&osip))
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: Cannot initialize osip!\n"));
      return -1;
    }

#ifdef DYNAMIC_PAYLOAD
#else
  eXosip_sdp_negotiation_init(&(eXosip.osip_negotiation));

{
	precedence_config_t CodecPredCfg;
	RtpPayloadType codec_seq[_CODEC_MAX];
	int32 FramePerPacket[_CODEC_MAX];
	int idx1, idx2;

	CodecPredCfg.chid = 0;
	CodecPredCfg.uCodecMax = _CODEC_MAX;
	CodecPredCfg.pCodecPred = codec_seq;
	CodecPredCfg.pFramePerPacket = FramePerPacket;
	GetCodecPrecedence(0, &CodecPredCfg);

	for (idx1=0; idx1<g_nMaxCodec; idx1++)
	{
		for (idx2=0; idx2<_CODEC_MAX; idx2++)
		{
			if (codec_seq[idx1] == support_media_list[idx2]->number)
			{
				eXosip_sdp_negotiation_add_codec(
					osip_strdup(support_media_list[idx2]->pt),
					NULL,
					osip_strdup("RTP/AVP"),
					NULL, NULL, NULL,
					NULL, NULL,
					osip_strdup(support_media_list[idx2]->string));
				break;
			}
		}
	}
  	
#if 0
	eXosip_sdp_negotiation_add_codec(
		osip_strdup("101"),
		NULL,
		osip_strdup("RTP/AVP"),
		NULL, NULL, NULL,
		NULL, NULL,
		osip_strdup("101 telephone-event/8000"));
#endif
}

#endif // DYNAMIC_PAYLOAD

  osip_set_application_context(osip, &eXosip);
  
  eXosip_set_callbacks(osip);
  
  eXosip.j_osip = osip;

  /* open a TCP socket to wake up the application when needed. */
  eXosip.j_socketctl = jpipe();
  if (eXosip.j_socketctl==NULL)
    return -1;

  eXosip.j_socketctl_event = jpipe();
  if (eXosip.j_socketctl_event==NULL)
    return -1;
  if (ipv6_enable == 0)
    {
		for (nPort=0; nPort<g_MaxVoIPPorts; nPort++)
		{
			if (!eXosip_bind_socket(&eXosip.j_sockets[nPort], ports[nPort]))
				return -1;

#ifdef CONFIG_RTK_VOIP_SIP_TLS
			for(i = 0; i < MAX_PROXY; i++){
				eXosip.j_tls_sockets[nPort][i]=-1;
				if(g_pVoIPCfg->ports[nPort].proxies[i].siptls_enable){
					if(!eXosip_create_TCP_socket(&eXosip.j_tls_sockets[nPort][i], ports[nPort], nPort, i)){
						g_warning("create TCP socket for fxs:%d,proxy:%d fail\n",nPort, i);
					}else{
						/*Do ssl initial*/
						sipTLSObj[nPort][i].ssl_ctx=initialize_client_ctx(nPort, i);
						if(sipTLSObj[nPort][i].ssl_ctx == NULL){
							close(eXosip.j_tls_sockets[nPort][i]);
							continue;
						}else{
							sipTLSObj[nPort][i].ssl_conn=initialize_client_ssl(sipTLSObj[nPort][i].ssl_ctx,eXosip.j_tls_sockets[nPort][i]);
						}
						/*do ssl handshake to sip proxy*/
						if(sipTLSObj[nPort][i].ssl_conn == NULL){
							close(eXosip.j_tls_sockets[nPort][i]);
							tls_free(sipTLSObj[nPort][i]);
							continue;
						}else{
							if(SSL_Handshake(sipTLSObj[nPort][i].ssl_conn) == 0){
								close(eXosip.j_tls_sockets[nPort][i]);
								g_warning("ssl handshake fail!\n");
							}
						}
					}
				}
			}
#endif

#ifdef NEW_STUN
			for (i=0; i<MAX_SS; i++)
			{
				eXosip.j_media_sockets[nPort * 2 + i] = -1;
			}

			eXosip.j_t38_sockets[nPort] = -1;
#endif
		}
    }
  else
    {
		struct sockaddr_in6 raddr;

		for (nPort=0; nPort<g_MaxVoIPPorts; nPort++)
		{

			eXosip.j_sockets[nPort] = (int)socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);
			if (eXosip.j_sockets[nPort] == -1)
				return -1;
      
			memset(&raddr, 0, sizeof(raddr));
			raddr.sin6_port = htons((short)ports[nPort]);
			raddr.sin6_family = AF_INET6;
			i = bind(eXosip.j_sockets[nPort], (struct sockaddr *)&raddr, sizeof(raddr));
			if (i < 0)
			{
				OSIP_TRACE (osip_trace
					(__FILE__, __LINE__, OSIP_ERROR, NULL,
					"eXosip: Cannot bind on port[%d]: %i!\n", nPort, i));
				return -1;
			}
		}
    }

  for (nPort=0; nPort<g_MaxVoIPPorts; nPort++)
  {
    eXosip.localports[nPort] = (char*)osip_malloc(10);
    sprintf(eXosip.localports[nPort], "%i", ports[nPort]);
    eXosip.media_ports[nPort] = (char*)osip_malloc(10);
    sprintf(eXosip.media_ports[nPort], "%i", media_ports[nPort]);
  }

#ifdef FIX_THREAD_PROBLEM
  /* To be changed in osip! */
  eXosip.j_events = (osip_fifo_t*) osip_malloc(sizeof(osip_fifo_t));
  osip_fifo_init(eXosip.j_events);
#endif

#ifndef DISABLE_THREAD
  eXosip.j_thread = (void*) osip_thread_create(20000,eXosip_thread, NULL);
  if (eXosip.j_thread==NULL)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: Cannot start thread!\n"));
      return -1;
    }
#endif

#ifndef FIX_THREAD_PROBLEM
  /* To be changed in osip! */
  eXosip.j_events = (osip_fifo_t*) osip_malloc(sizeof(osip_fifo_t));
  osip_fifo_init(eXosip.j_events);
#endif

  jfriend_load();
  jidentity_load();
  jsubscriber_load();
  return 0;
}

int eXosip_force_localip(const char *localip){
	if (localip!=NULL){
		strcpy(eXosip.localip,localip);
		eXosip.forced_localip=1;
	}else {
		eXosip_guess_ip_for_via(eXosip.ip_family, eXosip.localip, 49);
		eXosip.forced_localip=0;
	}
	return 0;
}

void
eXosip_set_mode(int mode)
{
  eXosip.j_runtime_mode = mode;
}

void
eXosip_set_user_agent(const char *user_agent)
{
  osip_free(eXosip.user_agent);
  eXosip.user_agent = osip_strdup(user_agent);
}

void
eXosip_automatic_refresh()
{
#if 0
  eXosip_call_t      *jc;
  eXosip_notify_t    *jn;
#endif
  eXosip_subscribe_t *js;
  eXosip_dialog_t    *jd;
  eXosip_reg_t       *jr;
  int now;
  
  now = time(NULL);
  
#if 0
  for (jc=eXosip.j_calls; jc!=NULL; jc=jc->next)
    {
      if (jc->c_id<1)
	{
	}
      for (jd=jc->c_dialogs; jd!=NULL; jd=jd->next)
	{
	  if (jd->d_dialog!=NULL) /* finished call */
	    {
	      /* sending update for keep-alive mechanism could be done here! */
	    }
	}
    }
#endif
  
  for (js=eXosip.j_subscribes; js!=NULL; js=js->next)
    {
      for (jd=js->s_dialogs; jd!=NULL; jd=jd->next)
	{
	  if (jd->d_dialog!=NULL) /* finished call */
	    {
	      if (jd->d_id>=1)
		{
		  if (eXosip_subscribe_need_refresh(js, now)==0)
		    {
		      int i;
		      i = eXosip_subscribe_send_subscribe(js, jd, "3600");
		    }
		}
	    }
	}
    }

    for (jr = eXosip.j_reg; jr != NULL; jr = jr->next)
      {
	if (jr->r_id >=1 || jr->r_last_tr!=NULL)
	  {
	    if (jr->r_reg_period==0)
	      {
		/* skip refresh! */
	      }
	    else if (now-jr->r_last_tr->birth_time>300)
	      {
		/* automatic refresh */
		eXosip_register(jr->r_id, jr->r_reg_period);
	      }
	    else if (now-jr->r_last_tr->birth_time>jr->r_reg_period-60)
	      {
		/* automatic refresh */
		eXosip_register(jr->r_id, jr->r_reg_period);
	      }
	    else if (now-jr->r_last_tr->birth_time>120 &&
		     (jr->r_last_tr->last_response==NULL
		      || (!MSG_IS_STATUS_2XX(jr->r_last_tr->last_response))))
	      {
		/* automatic refresh */
		eXosip_register(jr->r_id, jr->r_reg_period);
	      }
	  }
      }
}

void
eXosip_update()
{
	static int static_id  = 1 ;
	eXosip_call_t * jc ;
	eXosip_subscribe_t * js ;
	eXosip_notify_t * jn ;
	eXosip_dialog_t * jd ;
	int now;

	if( static_id > 100000 )
		static_id = 1 ;	/* loop */

	now = time( NULL ) ;
	for( jc = eXosip.j_calls ; jc != NULL ; jc = jc->next )
	{
		if( jc->c_id < 1 )
		{
			jc->c_id = ++static_id ;
		}

		for( jd = jc->c_dialogs ; jd != NULL ; jd = jd->next )
		{
			if( jd->d_dialog != NULL )	/* finished call */
			{
				if( jd->d_id < 1 )
				{
					jd->d_id = ++static_id ;
				}
			}
			else
				jd->d_id = -1 ;
		}
	}

	for( js = eXosip.j_subscribes ; js != NULL ; js = js->next )
	{
		if( js->s_id < 1 )
		{
			js->s_id = ++static_id ;
		}

		for( jd = js->s_dialogs ; jd != NULL ; jd = jd->next )
		{
			if( jd->d_dialog != NULL )	/* finished call */
			{
				if( jd->d_id < 1 )
				{
					jd->d_id = ++static_id ;
				}
			}
			else
				jd->d_id = -1 ;
		}
	}

	for( jn = eXosip.j_notifies ; jn != NULL ; jn = jn->next )
	{
		if( jn->n_id < 1 )
		{
			jn->n_id = ++static_id ;
		}

		for( jd = jn->n_dialogs ; jd != NULL ; jd = jd->next )
		{
			if( jd->d_dialog != NULL )	/* finished call */
			{
				if( jd->d_id < 1 )
				{
					jd->d_id = ++static_id ;
				}
			}
			else
				jd->d_id = -1 ;
		}
	}
}

int eXosip_message(int nPort, char *to, char *from, osip_list_t *routes, char *buff)
{
  /* eXosip_call_t *jc;
     osip_header_t *subject; */
  osip_message_t *message;
  osip_transaction_t *transaction;
  osip_event_t *sipevent;
  int i;

  i = generating_message(nPort, &message, to, from, routes, buff);
  if (i!=0)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: cannot send message (cannot build MESSAGE)! "));
      return -1;
    }

  i = osip_transaction_init(&transaction,
		       NICT,
		       eXosip.j_osip,
		       message);
  if (i!=0)
    {
      /* TODO: release the j_call.. */

      osip_message_free(message);
      return -1;
    }
  
  #ifdef CONFIG_RTK_VOIP_SIP_TLS
  if(eXosip_get_transport_type(message)==TRANSPORT_TLS){
	osip_uri_t *uri=NULL;
	char *host;
	int proxyIndex;
	  
	uri=osip_from_get_url(message->from);
	host=osip_uri_get_host(uri);
	proxyIndex=eXosip_find_TCP_socket_form_proxy(nPort, host);
	if(proxyIndex != -1)
	  osip_transaction_set_out_socket(transaction, eXosip.j_tls_sockets[nPort][proxyIndex]);
  }else{
  osip_transaction_set_out_socket(transaction, eXosip.j_sockets[nPort]);
  }
  #else /*CONFIG_RTK_VOIP_SIP_TLS*/
  osip_transaction_set_out_socket(transaction, eXosip.j_sockets[nPort]);
  #endif /*CONFIG_RTK_VOIP_SIP_TLS*/

  osip_list_add(eXosip.j_transactions, transaction, 0);
  
  sipevent = osip_new_outgoing_sipmessage(message);
  sipevent->transactionid =  transaction->transactionid;
  
  osip_transaction_set_your_instance(transaction, __eXosip_new_jinfo(nPort, NULL, NULL, NULL, NULL));
  osip_transaction_add_event(transaction, sipevent);
  __eXosip_wakeup();
  return 0;
}

#ifdef SIP_PING

int eXosip_ping(int nPort, char *from, char *to, osip_list_t *routes, osip_transaction_t **tr)
{
	osip_message_t *message;
	osip_transaction_t *transaction;
	osip_event_t *sipevent;
	int i;

	*tr = NULL;
	i = generating_ping(nPort, &message, from, to, routes);
	if (i!=0)
	{
		OSIP_TRACE (osip_trace
			(__FILE__, __LINE__, OSIP_ERROR, NULL,
			"eXosip: cannot send message (cannot build MESSAGE)! "));
		return -1;
	}

	i = osip_transaction_init(&transaction,
		NICT,
		eXosip.j_osip,
		message);

	if (i!=0)
	{
		osip_message_free(message);
		return -1;
	}
	*tr = transaction;
#ifdef CONFIG_RTK_VOIP_SIP_TLS
		if(eXosip_get_transport_type(message)==TRANSPORT_TLS){
			osip_uri_t *uri=NULL;
			char *host;
			int proxyIndex;

			uri=osip_from_get_url(message->from);
			host=osip_uri_get_host(uri);
			proxyIndex=eXosip_find_TCP_socket_form_proxy(nPort, host);
			if(proxyIndex != -1)
				osip_transaction_set_out_socket(transaction, eXosip.j_tls_sockets[nPort][proxyIndex]);
		}else{
	osip_transaction_set_out_socket(transaction, eXosip.j_sockets[nPort]);
		}
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
	osip_transaction_set_out_socket(transaction, eXosip.j_sockets[nPort]);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
	osip_list_add(eXosip.j_transactions, transaction, 0);
	sipevent = osip_new_outgoing_sipmessage(message);
	sipevent->transactionid =  transaction->transactionid;
	osip_transaction_set_your_instance(transaction, __eXosip_new_jinfo(nPort, NULL, NULL, NULL, NULL));
	osip_transaction_add_event(transaction, sipevent);
	__eXosip_wakeup();
	return 0;
}

#endif

int eXosip_info_call(int jid, char *content_type, char *body)
{
  osip_transaction_t *transaction;
  osip_event_t *sipevent;
  osip_message_t *info;
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  int i;
  
  if (jid>0)
    {
      eXosip_call_dialog_find(jid, &jc, &jd);
    }
  if (jd==NULL)
    {
       OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: No call here?\n"));
      return -1;
    }
  if (jd==NULL || jd->d_dialog==NULL)
    {
       OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: No established dialog!"));
      return -1;
    }

  transaction = eXosip_find_last_options(jc, jd);
  if (transaction!=NULL)
    {
      if (transaction->state!=NICT_TERMINATED &&
	  transaction->state!=NIST_TERMINATED)
	return -1;
      transaction=NULL;
    }
 
  i = generating_info_within_dialog(jc->nPort, &info, jd->d_dialog);
  if (i!=0)
    {
       OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: cannot send info message! "));
      return -2;
    }
  
  osip_message_set_content_type(info, content_type);
  osip_message_set_body(info, body, strlen(body));
  
  i = osip_transaction_init(&transaction,
			    NICT,
			    eXosip.j_osip,
			    info);
  if (i!=0)
    {
      osip_message_free(info);
      return -1;
    }

#ifdef CONFIG_RTK_VOIP_SIP_TLS
		  if(eXosip_get_transport_type(info)==TRANSPORT_TLS){
		  	osip_uri_t *uri=NULL;
			char *host;
			int proxyIndex;
	  
			uri=osip_from_get_url(info->from);
			host=osip_uri_get_host(uri);
		  	proxyIndex=eXosip_find_TCP_socket_form_proxy(jc->nPort, host);
			if(proxyIndex!=-1)
			  	osip_transaction_set_out_socket(transaction, eXosip.j_tls_sockets[jc->nPort][proxyIndex]);
		  }else{
  osip_transaction_set_out_socket(transaction, eXosip.j_sockets[jc->nPort]);
		  }
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  osip_transaction_set_out_socket(transaction, eXosip.j_sockets[jc->nPort]);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

  osip_list_add(jd->d_out_trs, transaction, 0);
  
  sipevent = osip_new_outgoing_sipmessage(info);
  sipevent->transactionid =  transaction->transactionid;
  
  osip_transaction_set_your_instance(transaction, __eXosip_new_jinfo(jc->nPort, jc, jd, NULL, NULL));
  osip_transaction_add_event(transaction, sipevent);
  __eXosip_wakeup();
  return 0;
}

int eXosip_initiate_call_with_body(int nPort, osip_message_t *invite, const char *bodytype, 
	const char *body, void *reference, char *caller_id)
{
	eXosip_call_t *jc;
  osip_header_t *subject;
  osip_transaction_t *transaction;
  osip_event_t *sipevent;
  int i;
  char *size;
  
  if (body!=NULL){
    size= (char *)osip_malloc(7*sizeof(char));
	sprintf(size,"%i",strlen(body));
	osip_message_set_content_length(invite, size);
	osip_free(size);
	osip_message_set_body(invite, body, strlen(body));
	osip_message_set_content_type(invite,bodytype);
  }
  else osip_message_set_content_length(invite, "0");

  eXosip_call_init(nPort, &jc);
  i = osip_message_get_subject(invite, 0, &subject);
  if (subject!=NULL && subject->hvalue!=NULL && subject->hvalue[0]!='\0')
    snprintf(jc->c_subject, 99, "%s", subject->hvalue);

  jc->c_ack_sdp = 0;
  i = osip_transaction_init(&transaction,
		       ICT,
		       eXosip.j_osip,
		       invite);
  if (i!=0)
    {
      eXosip_call_free(jc);
      osip_message_free(invite);
      return -1;
    }
  
  #ifdef CONFIG_RTK_VOIP_SIP_TLS
	if(eXosip_get_transport_type(invite)==TRANSPORT_TLS){
		osip_uri_t *uri=NULL;
		char *host;
		int proxyIndex;
	  
		uri=osip_from_get_url(invite->from);
		host=osip_uri_get_host(uri);
		proxyIndex=eXosip_find_TCP_socket_form_proxy(jc->nPort, host);
		if(proxyIndex != -1)
			osip_transaction_set_out_socket(transaction, eXosip.j_tls_sockets[jc->nPort][proxyIndex]);
	}else{
		osip_transaction_set_out_socket(transaction, eXosip.j_sockets[jc->nPort]);
	}
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  osip_transaction_set_out_socket(transaction, eXosip.j_sockets[jc->nPort]);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/


  jc->c_out_tr = transaction;
  
  sipevent = osip_new_outgoing_sipmessage(invite);
  sipevent->transactionid =  transaction->transactionid;

  osip_transaction_set_your_instance(transaction, __eXosip_new_jinfo(nPort, jc, NULL, NULL, NULL));
  osip_transaction_add_event(transaction, sipevent);

  jc->external_reference = reference;
  ADD_ELEMENT(eXosip.j_calls, jc);

  eXosip_update() ; /* fixed? */
  __eXosip_wakeup();
  return jc->c_id;
}

osip_message_t *eXosip_prepare_request_for_auth(int nPort, osip_message_t *msg)
{
#ifdef SM
  char *locip;
#else
aaaaaaaaaaaaaaa
  char locip[50];
#endif
  osip_message_t *newmsg;
  int  cseq;
  char tmp[90];
  osip_via_t *via;
  
  osip_message_clone(msg,&newmsg);
  if (newmsg==NULL){
    eXosip_trace(OSIP_INFO1,("eXosip_prepare_request_for_auth: could not clone msg."));
    return NULL;
  }
  via = (osip_via_t *) osip_list_get (newmsg->vias, 0);
  if (via==NULL || newmsg->cseq==NULL || newmsg->cseq->number==NULL) {
    osip_message_free(newmsg);
    eXosip_trace(OSIP_INFO1,("eXosip_prepare_request_for_auth: Bad headers in previous request."));
    return NULL;
  }
  /* increment cseq */
  cseq=atoi(newmsg->cseq->number);
  osip_free(newmsg->cseq->number);
  newmsg->cseq->number=strdup_printf("%i",cseq+1);

  osip_list_remove(newmsg->vias, 0);
  osip_via_free(via);
#ifdef SM
  eXosip_get_localip_for(newmsg->req_uri->host,&locip);
#else
aaaaaaaaaaaaaaa
  eXosip_guess_ip_for_via(eXosip.ip_family, locip, 49);
#endif
  if (eXosip.ip_family==AF_INET6)
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	if(eXosip_get_transport_type(msg)==TRANSPORT_TLS)
		sprintf(tmp, "SIP/2.0/TLS [%s]:%s;branch=z9hG4bK%u",
			locip,
			eXosip.localports[nPort],
			via_branch_new_random());
	else
    sprintf(tmp, "SIP/2.0/UDP [%s]:%s;branch=z9hG4bK%u",
	    locip,
	    eXosip.localports[nPort],
	    via_branch_new_random());
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
	sprintf(tmp, "SIP/2.0/UDP [%s]:%s;branch=z9hG4bK%u",
	    locip,
	    eXosip.localports[nPort],
	    via_branch_new_random());
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
  else
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	if(eXosip_get_transport_type(msg)==TRANSPORT_TLS)
		sprintf(tmp, "SIP/2.0/TLS %s:%s;rport;branch=z9hG4bK%u",
			locip,
			eXosip.localports[nPort],
			via_branch_new_random());
  else
    sprintf(tmp, "SIP/2.0/UDP %s:%s;rport;branch=z9hG4bK%u",
	    locip,
	    eXosip.localports[nPort],
	    via_branch_new_random());
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
    sprintf(tmp, "SIP/2.0/UDP %s:%s;rport;branch=z9hG4bK%u",
	    locip,
	    eXosip.localports[nPort],
	    via_branch_new_random());
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
#ifdef SM
  osip_free(locip);
#endif
  osip_via_init(&via);
  osip_via_parse(via, tmp);
  osip_list_add(newmsg->vias, via, 0);

  return newmsg;
}

#if 0
int eXosip_get_DeviceName(char *c_address, char *devName)
{
	int index=0, sockfd, len;
	struct sockaddr_in cliaddr, servaddr;
	struct ifreq ifr;
	char deviceIP[32], localip[32];

	if(NULL == c_address || 0==strlen(c_address))
		return 0;

	/* find out the local ip*/
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5060);
	inet_pton(AF_INET, c_address, &servaddr.sin_addr);
	connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
	len = sizeof(cliaddr);
	getsockname(sockfd, (struct sockaddr *) &cliaddr, &len);
	getnameinfo((struct sockaddr *)&cliaddr,sizeof(struct sockaddr),localip,256,NULL,0,NI_NUMERICHOST);

	/* find the device */
	for( index =1; index <= 256; index++){
		ifr.ifr_ifindex = index;
		if(ioctl(sockfd, SIOCGIFNAME, &ifr) == 0){
			memset(devName, 0, 32);	// reset the device name
			memcpy(devName,&ifr.ifr_name,strlen(ifr.ifr_name));
			if(ioctl(sockfd, SIOCGIFADDR, &ifr)==0){ // find the ip address allocate to the device
				getnameinfo(&ifr.ifr_addr, sizeof(struct sockaddr),deviceIP,256,NULL,0,NI_NUMERICHOST);
				if(0 == strcmp( localip, deviceIP))
					close(sockfd);
					return 1;
			}
		}
	}

	close(sockfd);
	return 0;
}
#endif

#if 0

int eXosip_retry_call(int cid)
{
  eXosip_call_t *jc=NULL;
  osip_transaction_t *tr,*newtr=NULL;
  osip_message_t *inv=NULL;
  int i;
  osip_event_t *sipevent;
  eXosip_call_find(cid,&jc);
  if (jc==NULL) {
    eXosip_trace(OSIP_INFO1,("eXosip_retry_last_invite: No such call."));
    return -1;
  }
  tr=eXosip_find_last_out_invite(jc,NULL);
  if (tr==NULL){
    eXosip_trace(OSIP_INFO1,("eXosip_retry_last_invite: No such transaction."));
    return -1;
  }
  if (tr->last_response==NULL){
    eXosip_trace(OSIP_INFO1,("eXosip_retry_last_invite: transaction has not been answered."));
    return -1;
  }
  inv=eXosip_prepare_request_for_auth(jc->nPort, tr->orig_request);
  if (inv==NULL) return -1;
  eXosip_add_authentication_information(inv,tr->last_response);
  if (-1 == eXosip_update_top_via(jc->nPort, inv))
    {
      osip_message_free(inv);
      return -1;
    }


  i = osip_transaction_init(&newtr,
		       ICT,
		       eXosip.j_osip,
		       inv);
  if (i!=0)
    {
      osip_message_free(inv);
      return -1;
    }

#ifdef FIX_MEMORY_LEAK
  __eXosip_delete_jinfo(tr);
  osip_transaction_free(tr);
#endif

  osip_transaction_set_out_socket(newtr, eXosip.j_sockets[jc->nPort]);

  jc->c_out_tr = newtr;
  
  sipevent = osip_new_outgoing_sipmessage(inv);
  
  osip_transaction_set_your_instance(newtr, __eXosip_new_jinfo(jc->nPort, jc, NULL, NULL, NULL));
  osip_transaction_add_event(newtr, sipevent);

  eXosip_update(); /* fixed? */
  __eXosip_wakeup();
  return jc->c_id;
}

#else

int eXosip_retry_call(int cid, int did)
{
  eXosip_call_t *jc = NULL;
  eXosip_dialog_t *jd;
  osip_transaction_t *tr, *newtr = NULL;
  osip_message_t *sip = NULL;
  int i;
  osip_event_t *sipevent;

  eXosip_call_dialog_find(did, &jc, &jd);
  if (jd == NULL)
    eXosip_call_find(cid, &jc);

  if (jc == NULL) {
    eXosip_trace(OSIP_INFO1,("eXosip_retry_last: No such call."));
    return -1;
  }

  tr = eXosip_find_last_out_tr(jc, jd);
  if (tr == NULL){
    eXosip_trace(OSIP_INFO1,("eXosip_retry_last: No such transaction."));
    return -1;
  }

  if (tr->last_response == NULL){
    eXosip_trace(OSIP_INFO1,("eXosip_retry_last: transaction has not been answered."));
    return -1;
  }

  sip = eXosip_prepare_request_for_auth(jc->nPort, tr->orig_request);
  if (sip == NULL) return -1;

  eXosip_add_authentication_information(sip, tr->last_response);
  if (-1 == eXosip_update_top_via(jc->nPort, sip))
    {
      osip_message_free(sip);
      return -1;
    }

  // rock 2007.9.12: 
  // have to update local seq after eXosip_prepare_request_for_auth
  if (jd && jd->d_dialog)
    jd->d_dialog->local_cseq++;

  // BYE auth
  if (tr->ctx_type == NICT)
  {
    i = eXosip_create_transaction(jc, jd, sip);
    if (i != 0)
    {
      OSIP_TRACE (osip_trace
        (__FILE__, __LINE__, OSIP_ERROR, NULL,
        "eXosip: cannot initiate SIP transaction! "));
      // recover local seq
      if (jd && jd->d_dialog)
        jd->d_dialog->local_cseq--;
      return -1;
    }

    return jc->c_id;
  }

  // INVITE auth
  i = osip_transaction_init(&newtr,
		       ICT,
		       eXosip.j_osip,
		       sip);
  if (i!=0)
    {
      osip_message_free(sip);
      // recover local seq
      if (jd && jd->d_dialog)
        jd->d_dialog->local_cseq--;
      return -1;
    }

#ifdef CONFIG_RTK_VOIP_SIP_TLS
	if(eXosip_get_transport_type(sip)==TRANSPORT_TLS){
		osip_uri_t *uri=NULL;
		char *host;
		int proxyIndex;
		  
		uri=osip_from_get_url(sip->from);
		host=osip_uri_get_host(uri);
		proxyIndex=eXosip_find_TCP_socket_form_proxy(jc->nPort, host);
		if(proxyIndex != -1)
			osip_transaction_set_out_socket(newtr, eXosip.j_tls_sockets[jc->nPort][proxyIndex]);
	}else{
  osip_transaction_set_out_socket(newtr, eXosip.j_sockets[jc->nPort]);
	}
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  osip_transaction_set_out_socket(newtr, eXosip.j_sockets[jc->nPort]);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

  if (jd)
  {
    osip_list_add(jd->d_out_trs, newtr, 0);
  }
  else
  {
#ifdef FIX_MEMORY_LEAK
	__eXosip_delete_jinfo(tr);
	osip_transaction_free(tr);
#endif
    jc->c_out_tr = newtr;
  }
  
  sipevent = osip_new_outgoing_sipmessage(sip);
  sipevent->transactionid = newtr->transactionid;
  osip_transaction_set_your_instance(newtr, __eXosip_new_jinfo(jc->nPort, jc, jd, NULL, NULL));
  osip_transaction_add_event(newtr, sipevent);
  eXosip_update(); /* fixed? */
  __eXosip_wakeup();

  return jc->c_id;
}

#endif

#ifdef DYNAMIC_PAYLOAD
#else
extern osip_list_t *supported_codec;
#endif // DYNAMIC_PAYLOAD

#if 0

int eXosip_initiate_call(int nPort, osip_message_t *invite, void *reference, 
	void *sdp_context_reference, char *local_sdp_port)
{
  eXosip_call_t *jc;
  osip_header_t *subject;
  osip_transaction_t *transaction;
  osip_event_t *sipevent;
  int i;
  sdp_message_t *sdp;
  char *body;
  char *size;
  
  if (invite==NULL || invite->req_uri==NULL || invite->req_uri->host==NULL  ) return -1;
  
  if (local_sdp_port!=NULL)
    {
      osip_negotiation_sdp_build_offer(eXosip.osip_negotiation, NULL, &sdp, local_sdp_port, NULL);
      
      /*
	if speex codec is supported, add bandwith attribute:
	b=AS:110 20
	b=AS:111 20
      */
      if (sdp!=NULL)
	{
	  int pos=0;
	  while (!sdp_message_endof_media (sdp, pos))
	    {
	      int k = 0;
	      char *tmp = sdp_message_m_media_get (sdp, pos);
	      if (0 == osip_strncasecmp (tmp, "audio", 5))
		{
		  char *payload = NULL;
		  do {
		    payload = sdp_message_m_payload_get (sdp, pos, k);
		    if (payload == NULL)
		      {
		      }
		    else if (0==strcmp("110",payload))
		      {
			sdp_message_a_attribute_add (sdp,
						     pos,
						     osip_strdup ("AS"),
						     osip_strdup ("110 20"));
		      }
		    else if (0==strcmp("111",payload))
		      {
			sdp_message_a_attribute_add (sdp,
						     pos,
						     osip_strdup ("AS"),
						     osip_strdup ("111 20"));
		      }
		    k++;
		  } while (payload != NULL);
		}
	      pos++;
	    }
	}

	if(eXosip.j_firewall_ip[0]!='\0')
	  {
		  char *c_address = invite->req_uri->host;
		  int pos=0;
		  struct addrinfo *addrinfo;
		  struct __eXosip_sockaddr addr;
		  i = eXosip_get_addrinfo(nPort, &addrinfo, invite->req_uri->host, 5060);
		  if (i==0)
			{
			  memcpy (&addr, addrinfo->ai_addr, addrinfo->ai_addrlen);
			  freeaddrinfo (addrinfo);
			  c_address = inet_ntoa (((struct sockaddr_in *) &addr)->sin_addr);
			  OSIP_TRACE (osip_trace
				  (__FILE__, __LINE__, OSIP_INFO1, NULL,
				  "eXosip: here is the resolved destination host=%s\n", c_address));
			}

		/* If remote message contains a Public IP, we have to replace the SDP
		   connection address */
		if (eXosip_is_public_address(c_address))
		{
			  /* replace the IP with our firewall ip */
			  sdp_connection_t *conn = sdp_message_connection_get(sdp, -1, 0);
			  if (conn!=NULL && conn->c_addr!=NULL )
			  {
				  osip_free(conn->c_addr);
				  conn->c_addr = osip_strdup(eXosip.j_firewall_ip);
			  }
			  pos=0;
			  conn = sdp_message_connection_get(sdp, pos, 0);
			  while (conn!=NULL)
			  {
				  if (conn!=NULL && conn->c_addr!=NULL )
				  {
					  osip_free(conn->c_addr);
					  conn->c_addr = osip_strdup(eXosip.j_firewall_ip);
				  }
				  pos++;
				  conn = sdp_message_connection_get(sdp, pos, 0);
			  }
		  }
	  }

      i = sdp_message_to_str(sdp, &body);
      if (body!=NULL)
	{
	  size= (char *)osip_malloc(7*sizeof(char));
	  sprintf(size,"%i",strlen(body));
	  osip_message_set_content_length(invite, size);
	  osip_free(size);
	  
	  osip_message_set_body(invite, body, strlen(body));
	  osip_free(body);
	  osip_message_set_content_type(invite, "application/sdp");
	}
      else
	osip_message_set_content_length(invite, "0");
    }

  eXosip_call_init(nPort, &jc);
  if (local_sdp_port!=NULL)
    snprintf(jc->c_sdp_port,9, "%s", local_sdp_port);
  i = osip_message_get_subject(invite, 0, &subject);
  if (subject!=NULL && subject->hvalue!=NULL && subject->hvalue[0]!='\0')
	  snprintf(jc->c_subject, 99, "%s", subject->hvalue);
  
  if (sdp_context_reference==NULL)
    osip_negotiation_ctx_set_mycontext(jc->c_ctx, jc);
  else
    osip_negotiation_ctx_set_mycontext(jc->c_ctx, sdp_context_reference);

  if (local_sdp_port!=NULL)
    {
      osip_negotiation_ctx_set_local_sdp(jc->c_ctx, sdp);
      jc->c_ack_sdp = 0;
    }
  else
    jc->c_ack_sdp = 1;

  i = osip_transaction_init(&transaction,
		       ICT,
		       eXosip.j_osip,
		       invite);
  if (i!=0)
    {
      eXosip_call_free(jc);
      osip_message_free(invite);
      return -1;
    }
  
  osip_transaction_set_out_socket(transaction, eXosip.j_sockets[jc->nPort]);

  jc->c_out_tr = transaction;
  
  sipevent = osip_new_outgoing_sipmessage(invite);
  sipevent->transactionid =  transaction->transactionid;
  
  osip_transaction_set_your_instance(transaction, __eXosip_new_jinfo(jc, NULL, NULL, NULL));
  osip_transaction_add_event(transaction, sipevent);

  jc->external_reference = reference;
  ADD_ELEMENT(eXosip.j_calls, jc);

  eXosip_update(); /* fixed? */
  __eXosip_wakeup();
  return jc->c_id;
}

#endif

int eXosip2_answer(int jid, int status, osip_message_t **answer){
  int i = -1;
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  if (jid>0)
    {
      eXosip_call_dialog_find(jid, &jc, &jd);
    }
  if (jd==NULL)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: No call here?\n"));
      return -1;
    }
  if (status>100 && status<200)
    {
      i = _eXosip2_answer_invite_1xx(jc, jd, status, answer);
    }
  else if (status>199 && status<300)
    {
      i = _eXosip2_answer_invite_2xx(jc, jd, status, answer);
    }
  else if (status>300 && status<699)
    {
      i = _eXosip2_answer_invite_3456xx(jc, jd, status, answer);
    }
  else
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: wrong status code (101<status<699)\n"));
      return -1;
    }
  if (i!=0)
    return -1;
  return 0;
}

int eXosip2_answer_send(int jid, osip_message_t *answer){
  int i = -1;
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  osip_transaction_t *tr;
  osip_event_t *evt_answer;
  if (jid>0)
    {
      eXosip_call_dialog_find(jid, &jc, &jd);
    }
  if (jd==NULL)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: No call here?\n"));
      return -1;
    }

  tr = eXosip_find_last_inc_invite(jc, jd);
  if (tr==NULL)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: cannot find transaction to answer"));
      return -1;
    }
  /* is the transaction already answered? */
  if (tr->state==IST_COMPLETED
      || tr->state==IST_CONFIRMED
      || tr->state==IST_TERMINATED)
    {
       OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: transaction already answered\n"));
      return -1;
    }

  if (MSG_IS_STATUS_1XX(answer))
    {
      if (jd==NULL)
	{
	  i = eXosip_dialog_init_as_uas(&jd, tr->orig_request, answer);
	  if (i!=0)
	    {
         OSIP_TRACE (osip_trace
		     (__FILE__, __LINE__, OSIP_ERROR, NULL,
	         "eXosip: cannot create dialog!\n"));
	      i = 0;
	    }
	  else
	    {
	      ADD_ELEMENT(jc->c_dialogs, jd);
	    }
	}
    }
  else if (MSG_IS_STATUS_2XX(answer))
    {
      if (jd==NULL)
	{
	  i = eXosip_dialog_init_as_uas(&jd, tr->orig_request, answer);
	  if (i!=0)
	    {
         OSIP_TRACE (osip_trace
           (__FILE__, __LINE__, OSIP_ERROR, NULL,
	         "eXosip: cannot create dialog!\n"));
	      return -1;
	    }
	  ADD_ELEMENT(jc->c_dialogs, jd);
	}
      else i = 0;
      
      eXosip_dialog_set_200ok(jd, answer);
      osip_dialog_set_state(jd->d_dialog, DIALOG_CONFIRMED);
    }
  else if (answer->status_code>=300 && answer->status_code<=699)
    {
      i = 0;
    }
  else
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: wrong status code (101<status<699)\n"));
      return -1;
    }
  if (i!=0)
    return -1;

  evt_answer = osip_new_outgoing_sipmessage(answer);
  evt_answer->transactionid = tr->transactionid;
  
  osip_transaction_add_event(tr, evt_answer);
  __eXosip_wakeup();
  return 0;
}

int eXosip_answer_call_with_body(int jid, int status, const char *bodytype, const char *body){
  int i = -1;
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  if (jid>0)
    {
      eXosip_call_dialog_find(jid, &jc, &jd);
    }
  if (jd==NULL)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: No call here?\n"));
      return -1;
    }
  if (status>100 && status<200)
    {
      i = eXosip_answer_invite_1xx(jc, jd, status);
    }
  else if (status>199 && status<300)
    {
      i = eXosip_answer_invite_2xx_with_body(jc, jd, status,bodytype,body);
    }
  else if (status>300 && status<699)
    {
      i = eXosip_answer_invite_3456xx(jc, jd, status);
    }
  else
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: wrong status code (101<status<699)\n"));
      return -1;
    }
  if (i!=0)
    return -1;
  return 0;
}

int eXosip_set_redirection_address (int jid, const char *contact)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  if (jid>0)
    {
      eXosip_call_dialog_find(jid, &jc, &jd);
    }
  if (jd==NULL)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: No call here?\n"));
      return -1;
    }
  if (contact==NULL)
    memset(jc->c_redirection, '\0', 1024);
  else
    snprintf(jc->c_redirection, 1024, "%s", contact);
  return 0;
}

int eXosip_answer_call   (int jid, int status, char *local_sdp_port)
{
  int i = -1;
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  if (jid>0)
    {
      eXosip_call_dialog_find(jid, &jc, &jd);
    }
  if (jd==NULL)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: No call here?\n"));
      return -1;
    }
  if (status>100 && status<200)
    {
      i = eXosip_answer_invite_1xx(jc, jd, status);
    }
#ifdef DYNAMIC_PAYLOAD
#else
  else if (status>199 && status<300)
    {
#if 0 /* this seems to be useless?? */
      if (jc->c_ctx!=NULL)
	osip_negotiation_ctx_set_mycontext(jc->c_ctx, jc);
      else
	osip_negotiation_ctx_set_mycontext(jc->c_ctx, sdp_context_reference);
#endif
      if (local_sdp_port!=NULL)
	{
	  osip_negotiation_ctx_set_mycontext(jc->c_ctx, jc);
	  snprintf(jc->c_sdp_port,9, "%s", local_sdp_port);
	}

      i = eXosip_answer_invite_2xx(jc, jd, status, local_sdp_port);
    }
#endif // DYNAMIC_PAYLOAD
  else if (status>300 && status<699)
    {
      i = eXosip_answer_invite_3456xx(jc, jd, status);
    }
  else
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: wrong status code (101<status<699)\n"));
      return -1;
    }
  if (i!=0)
    return -1;
  return 0;
}

#ifdef DYNAMIC_PAYLOAD
#else
int
eXosip_retrieve_negotiated_payload(int jid, int *payload, char *payload_name, int pnsize)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  int pl;

  if (jid>0)
    {
      eXosip_call_dialog_find(jid, &jc, &jd);
    }
  if (jd==NULL)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: No call here?\n"));
      return -1;
    }

  pl = eXosip_retrieve_sdp_negotiation_result(jc->c_ctx, payload_name, pnsize);

  if (pl >= 0)
    {
    *payload = pl;
    return 0;
    }

  return -1;
}

int eXosip_options_call  (int jid)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;

  osip_transaction_t *transaction;
  osip_event_t *sipevent;
  osip_message_t *options;
  int i;
#ifdef CONFIG_RTK_VOIP_SIP_TLS
  osip_uri_t *uri=NULL;
  char *host;
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

  if (jid>0)
    {
      eXosip_call_dialog_find(jid, &jc, &jd);
    }
  if (jd==NULL)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: No call here?\n"));
      return -1;
    }

  transaction = eXosip_find_last_options(jc, jd);
  if (transaction!=NULL)
    {
      if (transaction->state!=NICT_TERMINATED &&
	  transaction->state!=NIST_TERMINATED)
	return -1;
      transaction=NULL;
    }
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	uri=osip_from_get_url(jd->local_uri);
	host=osip_uri_get_host(uri);
	if(eXosip_check_tls_proxy(jc->nPort, host))
		i = _eXosip_build_request_within_dialog(jc->nPort, &options, "OPTIONS", jd->d_dialog, "TLS");
	else
		i = _eXosip_build_request_within_dialog(jc->nPort, &options, "OPTIONS", jd->d_dialog, "UDP");		
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  i = _eXosip_build_request_within_dialog(jc->nPort, &options, "OPTIONS", jd->d_dialog, "UDP");
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
  if (i!=0)
    return -2;
  i = osip_transaction_init(&transaction,
		       NICT,
		       eXosip.j_osip,
		       options);
  if (i!=0)
    {
      /* TODO: release the j_call.. */
      osip_message_free(options);
      return -2;
    }
  #ifdef CONFIG_RTK_VOIP_SIP_TLS
	if(eXosip_get_transport_type(options)==TRANSPORT_TLS){
		osip_uri_t *uri=NULL;
		char *host;
		int proxyIndex;
  
		uri=osip_from_get_url(options->from);
		host=osip_uri_get_host(uri);
		proxyIndex=eXosip_find_TCP_socket_form_proxy(jc->nPort, host);
		if(proxyIndex != -1)
			osip_transaction_set_out_socket(transaction, eXosip.j_tls_sockets[jc->nPort][proxyIndex]);
	}else{
  osip_transaction_set_out_socket(transaction, eXosip.j_sockets[jc->nPort]);
	}
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  osip_transaction_set_out_socket(transaction, eXosip.j_sockets[jc->nPort]);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/


  osip_list_add(jd->d_out_trs, transaction, 0);
  
  sipevent = osip_new_outgoing_sipmessage(options);
  sipevent->transactionid =  transaction->transactionid;
  
  osip_transaction_set_your_instance(transaction, __eXosip_new_jinfo(jc->nPort, jc, jd, NULL, NULL));
  osip_transaction_add_event(transaction, sipevent);
  __eXosip_wakeup();
  return 0;
}

int eXosip_answer_options   (int cid, int jid, int status)
{
  int i = -1;
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  if (jid>0)
    {
      eXosip_call_dialog_find(jid, &jc, &jd);
      if (jd==NULL)
	{
     OSIP_TRACE (osip_trace
	    (__FILE__, __LINE__, OSIP_ERROR, NULL,
	     "eXosip: No dialog here?\n"));
	  return -1;
	}
    }
  else
    {
      eXosip_call_find(cid, &jc);
      if (jc==NULL)
   {
     OSIP_TRACE (osip_trace
	    (__FILE__, __LINE__, OSIP_ERROR, NULL,
	     "eXosip: No call here?\n"));
	  return -1;
	}
    }
  if (status>100 && status<200)
    {
      i = eXosip_answer_options_1xx(jc, jd, status);
    }
  else if (status>199 && status<300)
    {
      i = eXosip_answer_options_2xx(jc, jd, status);
    }
  else if (status>300 && status<699)
    {
      i = eXosip_answer_options_3456xx(jc, jd, status);
    }
  else
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: wrong status code (101<status<699)\n"));
      return -1;
    }
  if (i!=0)
    return -1;
  return 0;
}
#endif // DYNAMIC_PAYLOAD

int eXosip_set_call_reference(int jid, void *reference)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  if (jid>0)
    {
      eXosip_call_dialog_find(jid, &jc, &jd);
    }
  if (jc==NULL)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: No call here?\n"));
      return -1;
    }
    jc->external_reference = reference;
    return 0;
}

int eXosip_on_fax_call(int jid, int port, int session_version)
{
	eXosip_dialog_t * jd = NULL ;
	eXosip_call_t * jc = NULL ;
	osip_transaction_t * transaction ;
	osip_event_t * sipevent ;
	osip_message_t * invite ;
	int i ;
	sdp_message_t * sdp ;
	sdp_message_t * old_sdp ;
	char * body ;
	char * size ;
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	osip_uri_t *uri=NULL;
	char *host;
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/	  

	if( jid > 0 )
		eXosip_call_dialog_find( jid , &jc , &jd ) ;
	if( jd == NULL )
	{
		g_warning( "on_fax_call : No call here?\n" ) ;
		return -1 ;
	}

	transaction = eXosip_find_last_invite( jc , jd ) ;
	if( transaction == NULL )
	{
		g_warning( "on_fax_call : transaction == NULL\n" ) ;
		return -1 ;
	}
	if( transaction->state != ICT_TERMINATED && transaction->state != IST_TERMINATED && transaction->state != ICT_COMPLETED)
	{
		g_warning( "on_fax_call: transaction->state != ICT_TERMINATED or IST_TERMINATED\n" ) ;
		return -1 ;
	}

	sdp = eXosip_get_local_sdp_info(transaction);
	if( sdp == NULL )
	{
		g_warning( "on_fax_call: sdp == NULL\n" ) ;
		return -1 ;
	}
	i = osip_negotiation_sdp_message_put_on_fax( sdp, port ) ;
	if( i != 0 )
	{
		g_warning( "on_fax_call: osip_negotiation_sdp_message_put_on_fax Fail\n" ) ;
		sdp_message_free( sdp ) ;
		return -2 ;
	}

	// implement increment version in the o= field
	sprintf(sdp->o_sess_version, "%.10d", session_version);

#ifdef CONFIG_RTK_VOIP_SIP_TLS
	uri=osip_from_get_url(jd->d_dialog->local_uri);
  	host=osip_uri_get_host(uri);
	if(eXosip_check_tls_proxy(jc->nPort, host))
		i = _eXosip_build_request_within_dialog(jc->nPort,  &invite , "INVITE" , jd->d_dialog , "TLS" ) ;
	else
	i = _eXosip_build_request_within_dialog(jc->nPort,  &invite , "INVITE" , jd->d_dialog , "UDP" ) ;
#else
	i = _eXosip_build_request_within_dialog(jc->nPort,  &invite , "INVITE" , jd->d_dialog , "UDP" ) ;
#endif	
	if( i != 0 )
	{
		g_warning( "on_fax_call: _eXosip_build_request_within_dialog Fail\n" ) ;
		sdp_message_free( sdp ) ;
		return -2 ;
	}

	i = sdp_message_to_str( sdp , &body ) ;
	if( body != NULL )
	{
		size= ( char * )osip_malloc( 7 * sizeof( char ) ) ;
		sprintf( size , "%i" , strlen( body ) ) ;
		osip_message_set_content_length( invite , size ) ;
		osip_free( size ) ;
		osip_message_set_body( invite , body , strlen( body ) ) ;
		osip_free( body ) ;
		osip_message_set_content_type( invite , "application/sdp" ) ;
	}
	else
		osip_message_set_content_length( invite , "0" ) ;

	if( jc->c_subject == NULL || jc->c_subject[0] == '\0' )
	{
		/* nothing to do !! */
	}
	else
		osip_message_set_subject( invite , jc->c_subject ) ;

	transaction = NULL ;
	i = osip_transaction_init( &transaction , ICT , eXosip.j_osip , invite ) ;
	if( i != 0 )
	{
		g_warning( "on_fax_call: release the j_call..\n" ) ;
		osip_message_free( invite ) ;
		return -2 ;
	}

#ifdef CONFIG_RTK_VOIP_SIP_TLS
	if(eXosip_get_transport_type(invite)==TRANSPORT_TLS){
		osip_uri_t *uri=NULL;
		char *host;
		int proxyIndex;
		  
		uri=osip_from_get_url(invite->from);
		host=osip_uri_get_host(uri);
		proxyIndex=eXosip_find_TCP_socket_form_proxy(jc->nPort, host);
		if(proxyIndex != -1)
			osip_transaction_set_out_socket(transaction, eXosip.j_tls_sockets[jc->nPort][proxyIndex]);
	}else{
	osip_transaction_set_out_socket(transaction, eXosip.j_sockets[jc->nPort]);
	}
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
	osip_transaction_set_out_socket(transaction, eXosip.j_sockets[jc->nPort]);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/


	old_sdp = osip_negotiation_ctx_get_local_sdp( jc->c_ctx ) ;
	sdp_message_free( old_sdp ) ;
	osip_negotiation_ctx_set_local_sdp( jc->c_ctx , sdp ) ;

	osip_list_add( jd->d_out_trs , transaction , 0 ) ;
	sipevent = osip_new_outgoing_sipmessage( invite ) ;
	sipevent->transactionid = transaction->transactionid ;
	osip_transaction_set_your_instance( transaction , __eXosip_new_jinfo(jc->nPort, jc , jd , NULL , NULL ) ) ;
	osip_transaction_add_event( transaction , sipevent ) ;
	__eXosip_wakeup() ;
	g_message( "on_fax_call : OK!!\n" ) ;
	return 0 ;
}

int eXosip_on_update_call(int jid, int session_version)
{
	eXosip_dialog_t *jd = NULL;
	eXosip_call_t *jc = NULL;
	osip_transaction_t *transaction;
	osip_event_t *sipevent;
	osip_message_t *invite;
	int i;
	sdp_message_t *sdp;
	sdp_message_t *old_sdp;
	char *body;
	char *size;
	sdp_connection_t *conn;
	char *locip;
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	osip_uri_t *uri=NULL;
	char *host;
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/


	if (jid > 0)
		eXosip_call_dialog_find(jid, &jc, &jd);

	if (jd == NULL || jd->d_dialog==NULL)
	{
		g_warning("on_update_call: No call here? jid = %d\n", jid);
		return -1;
	}

	transaction = eXosip_find_last_invite(jc, jd);
	if (transaction == NULL)
	{
		g_warning("on_update_call: transaction == NULL\n");
		return -1;
	}

	if (transaction->state != ICT_TERMINATED && transaction->state != IST_TERMINATED)
	{
		g_warning("on_update_call: transaction->state != ICT_TERMINATED or IST_TERMINATED\n");
		return -1;
	}

	eXosip_get_localip_for(jd->d_dialog->remote_contact_uri->url->host, &locip);
	if (strcmp(locip, "127.0.0.1") == 0)
	{
		osip_free(locip);
		g_warning("on_update_call: disconnected\n");
		return -1;
	}

	// update connection address
	sdp = eXosip_get_local_sdp_info(transaction);
	if (sdp == NULL)
	{
		osip_free(locip);
		g_warning("on_update_call: sdp == NULL\n");
		return -1;
	}

	conn = sdp_message_connection_get(sdp, -1, 0);
	if (conn && conn->c_addr)
	{
		if (strcmp(conn->c_addr, locip) == 0)
		{
			osip_free(locip);
			g_warning("on_update_call: ip not change\n");
			return 1;
		}

		osip_free(conn->c_addr);
		conn->c_addr = osip_strdup(locip);
		g_warning("on_update_call: new ip = %s\n", locip);
	}
	else
	{
		osip_free(locip);
		g_warning("on_update_call: no old connection!?\n");
		return -1;
	}
	
	osip_free(locip);

	// implement increment version in the o= field
	sprintf(sdp->o_sess_version, "%.10d", session_version);
#ifdef CONFIG_RTK_VOIP_SIP_TLS
		uri=osip_from_get_url(jd->d_dialog->local_uri);
		host=osip_uri_get_host(uri);
		if(eXosip_check_tls_proxy(jc->nPort, host))
			i = _eXosip_build_request_within_dialog(jc->nPort, &invite, "INVITE", jd->d_dialog, "TLS");
		else
			i = _eXosip_build_request_within_dialog(jc->nPort, &invite, "INVITE", jd->d_dialog, "UDP");
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
	i = _eXosip_build_request_within_dialog(jc->nPort, &invite, "INVITE", jd->d_dialog, "UDP");
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
	if (i != 0)
	{
		g_warning("on_update_call: _eXosip_build_request_within_dialog Fail\n");
		sdp_message_free(sdp);
		return -2;
	}

	i = sdp_message_to_str(sdp, &body);
	if (body)
	{
		size = (char *) osip_malloc(7 * sizeof(char));
		sprintf(size, "%i", strlen(body));
		osip_message_set_content_length(invite, size);
		osip_free(size);
		osip_message_set_body(invite, body, strlen(body));
		osip_free(body);
		osip_message_set_content_type(invite, "application/sdp");
	}
	else
	{
		osip_message_set_content_length(invite, "0");
	}

	if (jc->c_subject == NULL || jc->c_subject[0] == '\0')
	{
		/* nothing to do !! */
	}
	else
	{
		osip_message_set_subject(invite, jc->c_subject);
	}

	transaction = NULL;
	i = osip_transaction_init(&transaction, ICT, eXosip.j_osip, invite);
	if (i != 0)
	{
		g_warning("on_update_call: release the j_call..\n");
		osip_message_free(invite);
		return -2;
	}

#ifdef CONFIG_RTK_VOIP_SIP_TLS
	if(eXosip_get_transport_type(invite)==TRANSPORT_TLS){
		osip_uri_t *uri=NULL;
		char *host;
		int proxyIndex;
		  
		uri=osip_from_get_url(invite->from);
		host=osip_uri_get_host(uri);
		proxyIndex=eXosip_find_TCP_socket_form_proxy(jc->nPort, host);
		if(proxyIndex != -1)
			osip_transaction_set_out_socket(transaction, eXosip.j_tls_sockets[jc->nPort][proxyIndex]);
	}else{
	osip_transaction_set_out_socket(transaction, eXosip.j_sockets[jc->nPort]);
	}
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
	osip_transaction_set_out_socket(transaction, eXosip.j_sockets[jc->nPort]);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

	old_sdp = osip_negotiation_ctx_get_local_sdp(jc->c_ctx);
	sdp_message_free(old_sdp);
	osip_negotiation_ctx_set_local_sdp(jc->c_ctx, sdp);

	osip_list_add(jd->d_out_trs, transaction, 0);
	sipevent = osip_new_outgoing_sipmessage(invite);
	sipevent->transactionid = transaction->transactionid;
	osip_transaction_set_your_instance(transaction, __eXosip_new_jinfo(jc->nPort, jc, jd, NULL, NULL));
	osip_transaction_add_event(transaction, sipevent);
	__eXosip_wakeup();

	g_message("on_update_call: OK!!\n");

	return 0;
}

int eXosip_on_hold_call(int jid, int session_version)
{
	eXosip_dialog_t * jd = NULL ;
	eXosip_call_t * jc = NULL ;
	osip_transaction_t * transaction ;
	osip_event_t * sipevent ;
	osip_message_t * invite ;
	int i ;
	sdp_message_t * sdp ;
	sdp_message_t * old_sdp ;
	char * body ;
	char * size ;
	sdp_connection_t *conn;
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	osip_uri_t *uri=NULL;
	char *host;
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

	if( jid > 0 )
		eXosip_call_dialog_find( jid , &jc , &jd ) ;
	if( jd == NULL || jd->d_dialog==NULL)
	{
		g_warning( "on_hold_call : No call here? jid=%d\n", jid ) ;
		return -1 ;
	}

	transaction = eXosip_find_last_invite( jc , jd ) ;
	if( transaction == NULL )
	{
		g_warning( "on_hold_call : transaction == NULL\n" ) ;
		return -1 ;
	}

	if( transaction->state != ICT_TERMINATED && transaction->state != IST_TERMINATED )
	{
		g_warning( "on_hold_call: transaction->state != ICT_TERMINATED or IST_TERMINATED\n" ) ;
		if (jd->d_dialog->state == DIALOG_EARLY)
			return 1; // need recover if hold in early media
		else
			return -1;
	}

	sdp = eXosip_get_local_sdp_info(transaction);
	if( sdp == NULL )
	{
		g_warning( "on_hold_call: sdp == NULL\n" ) ;
		return -1 ;
	}

	// mix 2543 & 3264 hold
	conn = sdp_message_connection_get(sdp, -1, 0);
	if (conn && conn->c_addr)
	{
		osip_free(conn->c_addr);
		conn->c_addr = osip_strdup("0.0.0.0");
	}
	
	i = osip_negotiation_sdp_message_put_media_mode(sdp, MEDIA_SENDONLY);
	if( i != 0 )
	{
		g_warning( "on_hold_call: osip_negotiation_sdp_message_put_on_hold Fail\n" ) ;
		sdp_message_free( sdp ) ;
		return -2 ;
	}

	// implement increment version in the o= field
	sprintf(sdp->o_sess_version, "%.10d", session_version);

#ifdef CONFIG_RTK_VOIP_SIP_TLS
	uri=osip_from_get_url(jd->d_dialog->local_uri);
	host=osip_uri_get_host(uri);
  	if(eXosip_check_tls_proxy(jc->nPort, host))
		i = _eXosip_build_request_within_dialog(jc->nPort, &invite, "INVITE", jd->d_dialog, "TLS");
	else
		i = _eXosip_build_request_within_dialog(jc->nPort, &invite, "INVITE", jd->d_dialog, "UDP");
#else
	i = _eXosip_build_request_within_dialog(jc->nPort, &invite, "INVITE", jd->d_dialog, "UDP");
#endif
	if( i != 0 )
	{
		g_warning( "on_hold_call: _eXosip_build_request_within_dialog Fail\n" ) ;
		sdp_message_free( sdp ) ;
		return -2 ;
	}

	i = sdp_message_to_str( sdp , &body ) ;
	if( body != NULL )
	{
		size= ( char * )osip_malloc( 7 * sizeof( char ) ) ;
		sprintf( size , "%i" , strlen( body ) ) ;
		osip_message_set_content_length( invite , size ) ;
		osip_free( size ) ;
		osip_message_set_body( invite , body , strlen( body ) ) ;
		osip_free( body ) ;
		osip_message_set_content_type( invite , "application/sdp" ) ;
	}
	else
		osip_message_set_content_length( invite , "0" ) ;

	if( jc->c_subject == NULL || jc->c_subject[0] == '\0' )
	{
		/* nothing to do !! */
	}
	else
		osip_message_set_subject( invite , jc->c_subject ) ;

	transaction = NULL ;
	i = osip_transaction_init( &transaction , ICT , eXosip.j_osip , invite ) ;
	if( i != 0 )
	{
		g_warning( "on_hold_call: release the j_call..\n" ) ;
		osip_message_free( invite ) ;
		return -2 ;
	}

#ifdef CONFIG_RTK_VOIP_SIP_TLS
	if(eXosip_get_transport_type(invite)==TRANSPORT_TLS){
		osip_uri_t *uri=NULL;
		char *host;
		int proxyIndex;
		  
		uri=osip_from_get_url(invite->from);
		host=osip_uri_get_host(uri);
		proxyIndex=eXosip_find_TCP_socket_form_proxy(jc->nPort, host);
		if(proxyIndex != -1)
			osip_transaction_set_out_socket(transaction, eXosip.j_tls_sockets[jc->nPort][proxyIndex]);
	}else{
	osip_transaction_set_out_socket(transaction, eXosip.j_sockets[jc->nPort]);
	}
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
	osip_transaction_set_out_socket(transaction, eXosip.j_sockets[jc->nPort]);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/


	old_sdp = osip_negotiation_ctx_get_local_sdp( jc->c_ctx ) ;
	sdp_message_free( old_sdp ) ;
	osip_negotiation_ctx_set_local_sdp( jc->c_ctx , sdp ) ;

	osip_list_add( jd->d_out_trs , transaction , 0 ) ;
	sipevent = osip_new_outgoing_sipmessage( invite ) ;
	sipevent->transactionid = transaction->transactionid ;
	osip_transaction_set_your_instance( transaction , __eXosip_new_jinfo(jc->nPort, jc , jd , NULL , NULL ) ) ;
	osip_transaction_add_event( transaction , sipevent ) ;
	__eXosip_wakeup() ;
	g_message( "on_hold_call : OK!!\n" ) ;
	return 0 ;
}

int eXosip_off_hold_call(int jid, char *rtp_ip, int port, int session_version)
{
	eXosip_dialog_t * jd = NULL ;
	eXosip_call_t * jc = NULL ;
	osip_transaction_t * transaction ;
	osip_event_t * sipevent ;
	osip_message_t * invite ;
	int i ;
	sdp_message_t * sdp ;
	char * body ;
	char * size ;
	sdp_connection_t *conn;
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	osip_uri_t *uri=NULL;
	char *host;
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/	

	if( jid > 0 )
		eXosip_call_dialog_find( jid , &jc , &jd ) ;
	if( jd == NULL || jd->d_dialog==NULL)
	{
		g_warning( "off_hold_call: No call here?\n" ) ;
		return -1 ;
	}

	transaction = eXosip_find_last_invite( jc , jd ) ;
	if( transaction == NULL )
	{
		g_warning( "off_hold_call: transaction == NULL!!\n" ) ;
		return -1 ;
	}
	if( transaction->state != ICT_TERMINATED && transaction->state != IST_TERMINATED )
	{
		g_warning( "off_hold_call: transaction->state != ICT_TERMINATED or IST_TERMINATED\n" ) ;
		return -1 ;
	}

	sdp = eXosip_get_local_sdp_info(transaction);
	if( sdp == NULL )
	{
		g_warning( "off_hold_call: sdp==NULL\n" ) ;
		return -1 ;
	}

	// mix 2543 & 3264 resume
	conn = sdp_message_connection_get(sdp, -1, 0);
	if (conn && conn->c_addr)
	{
		osip_free(conn->c_addr);
		conn->c_addr = osip_strdup(sdp->o_addr);
	}

	i = osip_negotiation_sdp_message_put_media_mode(sdp, MEDIA_SENDRECV);
	if (i!=0)
	{
		sdp_message_free(sdp);
		return -2;
	}

	// implement increment version in the o= field
	sprintf(sdp->o_sess_version, "%.10d", session_version);
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	uri=osip_from_get_url(jd->d_dialog->local_uri);
	host=osip_uri_get_host(uri);
  	if(eXosip_check_tls_proxy(jc->nPort, host))
		i = _eXosip_build_request_within_dialog(jc->nPort, &invite, "INVITE", jd->d_dialog, "TLS");
	else
  i = _eXosip_build_request_within_dialog(jc->nPort, &invite, "INVITE", jd->d_dialog, "UDP");
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  i = _eXosip_build_request_within_dialog(jc->nPort, &invite, "INVITE", jd->d_dialog, "UDP");
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
  if (i!=0) {
    sdp_message_free(sdp);
    return -2;
  }

  if (rtp_ip!=NULL)
    {
      /* modify the connection address of host */
      sdp_connection_t *conn;
      sdp_media_t *med;
      int pos_media = 0;
      conn = sdp_message_connection_get(sdp, -1, 0);
      if (conn!=NULL && conn->c_addr!=NULL)
	{
	  osip_free(conn->c_addr);
	  conn->c_addr = osip_strdup(rtp_ip);
	}
      med = (sdp_media_t *) osip_list_get (sdp->m_medias, pos_media);
      while (med != NULL)
	{
	  if (med->m_media!=NULL && 0==osip_strcasecmp(med->m_media, "audio"))
	    {
	      osip_free(med->m_port);
	      med->m_port=(char *)osip_malloc(15);
	      snprintf(med->m_port, 14, "%i", port);
	      break;
	    }
	  pos_media++;
	  med = (sdp_media_t *) osip_list_get (sdp->m_medias, pos_media);
	}
    }

  i = sdp_message_to_str(sdp, &body);
  if (body!=NULL)
    {
      size= (char *)osip_malloc(7*sizeof(char));
      sprintf(size,"%i",strlen(body));
      osip_message_set_content_length(invite, size);
      osip_free(size);

      osip_message_set_body(invite, body, strlen(body));
      osip_free(body);
      osip_message_set_content_type(invite, "application/sdp");
    }
  else
	{
    osip_message_set_content_length(invite, "0");
	}

  if (jc->c_subject==NULL || jc->c_subject[0]=='\0')
  {
#if 0
	  osip_message_set_subject(invite, "New Call");
#endif
  }
  else
	{
	  osip_message_set_subject(invite, jc->c_subject);
	}

  transaction=NULL;
  i = osip_transaction_init(&transaction,
		       ICT,
		       eXosip.j_osip,
		       invite);
  if (i!=0)
    {
      /* TODO: release the j_call.. */
      osip_message_free(invite);
      return -2;
    }
#ifdef CONFIG_RTK_VOIP_SIP_TLS
  if(eXosip_get_transport_type(invite)==TRANSPORT_TLS){
  	osip_uri_t *uri=NULL;
	char *host;
	int proxyIndex;

	uri=osip_from_get_url(invite->from);
	host=osip_uri_get_host(uri);
	proxyIndex=eXosip_find_TCP_socket_form_proxy(jc->nPort, host);
	if(proxyIndex != -1)
	  	osip_transaction_set_out_socket(transaction, eXosip.j_tls_sockets[jc->nPort][proxyIndex]);
  }else{
  osip_transaction_set_out_socket(transaction, eXosip.j_sockets[jc->nPort]);
  }
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  osip_transaction_set_out_socket(transaction, eXosip.j_sockets[jc->nPort]);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

  {
    sdp_message_t *old_sdp = osip_negotiation_ctx_get_local_sdp(jc->c_ctx);
    sdp_message_free(old_sdp);
    osip_negotiation_ctx_set_local_sdp(jc->c_ctx, sdp);
  }

  osip_list_add(jd->d_out_trs, transaction, 0);
  
  sipevent = osip_new_outgoing_sipmessage(invite);
  sipevent->transactionid =  transaction->transactionid;
  
  osip_transaction_set_your_instance(transaction, __eXosip_new_jinfo(jc->nPort, jc, jd, NULL, NULL));
  osip_transaction_add_event(transaction, sipevent);
  __eXosip_wakeup();
  return 0;
}

static int eXosip_create_transaction(eXosip_call_t *jc, eXosip_dialog_t *jd,
	osip_message_t *request)
{
  osip_event_t *sipevent;
  osip_transaction_t *tr;
  int i;
  i = osip_transaction_init(&tr,
		       NICT,
		       eXosip.j_osip,
		       request);
  if (i!=0)
    {
      /* TODO: release the j_call.. */

      osip_message_free(request);
      return -1;
    }
  
  #ifdef CONFIG_RTK_VOIP_SIP_TLS
  	if(eXosip_get_transport_type(request)==TRANSPORT_TLS){
		osip_uri_t *uri=NULL;
		char *host;
		int proxyIndex;
	  
		uri=osip_from_get_url(request->from);
		host=osip_uri_get_host(uri);
		g_message("@@@%s@@@\n", __FUNCTION__);
		proxyIndex=eXosip_find_TCP_socket_form_proxy(jc->nPort, host);
		if(proxyIndex != -1)
	  		osip_transaction_set_out_socket(tr, eXosip.j_tls_sockets[jc->nPort][proxyIndex]);
	}else{
		osip_transaction_set_out_socket(tr, eXosip.j_sockets[jc->nPort]);		
	}
  #else /*CONFIG_RTK_VOIP_SIP_TLS*/
  osip_transaction_set_out_socket(tr, eXosip.j_sockets[jc->nPort]);
  #endif /*CONFIG_RTK_VOIP_SIP_TLS*/

  if (jd!=NULL)
    osip_list_add(jd->d_out_trs, tr, 0);
  
  sipevent = osip_new_outgoing_sipmessage(request);
  sipevent->transactionid =  tr->transactionid;
  
  osip_transaction_set_your_instance(tr, __eXosip_new_jinfo(jc->nPort, jc, jd, NULL, NULL));
  osip_transaction_add_event(tr, sipevent);
  __eXosip_wakeup();
  return 0;
}

int eXosip_transfer_call_out_of_dialog(int nPort, char *refer_to, char *from, char *to, osip_list_t *routes)
{
  osip_message_t *refer;
  osip_transaction_t *transaction;
  osip_event_t *sipevent;
  int i;
  i = generating_refer_outside_dialog(nPort, &refer, refer_to, from, to, routes);
  if (i!=0) return -1;

  i = osip_transaction_init(&transaction,
		       NICT,
		       eXosip.j_osip,
		       refer);
  if (i!=0)
    {
      osip_message_free(refer);
      return -1;
    }
  #ifdef CONFIG_RTK_VOIP_SIP_TLS
	if(eXosip_get_transport_type(refer)==TRANSPORT_TLS){
		osip_uri_t *uri=NULL;
		char *host;
		int proxyIndex;
  
		uri=osip_from_get_url(refer->from);
		host=osip_uri_get_host(uri);
		proxyIndex=eXosip_find_TCP_socket_form_proxy(nPort, host);
		if(proxyIndex != -1)
			osip_transaction_set_out_socket(transaction, eXosip.j_tls_sockets[nPort][proxyIndex]);
	}else{
  osip_transaction_set_out_socket(transaction, eXosip.j_sockets[nPort]);
	}
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  osip_transaction_set_out_socket(transaction, eXosip.j_sockets[nPort]);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/


  osip_list_add(eXosip.j_transactions, transaction, 0);
  
  sipevent = osip_new_outgoing_sipmessage(refer);
  sipevent->transactionid =  transaction->transactionid;
  
  osip_transaction_set_your_instance(transaction, __eXosip_new_jinfo(nPort, NULL, NULL, NULL, NULL));
  osip_transaction_add_event(transaction, sipevent);
  __eXosip_wakeup();
  return 0;
}

int eXosip_transfer_call(int jid, const char *refer_to)
{
  int i;
  osip_message_t *request;
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  if (jid<=0)
    return -1;

  eXosip_call_dialog_find(jid, &jc, &jd);
  if (jd==NULL || jd->d_dialog==NULL || jd->d_dialog->state==DIALOG_EARLY)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: No established call here!"));
      return -1;
    }

  i = generating_refer(jc->nPort, &request, jd->d_dialog, refer_to);
  if (i!=0)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: cannot generate REFER for call!"));
      return -2;
    }
#ifdef ATTENDED_TRANSFER
  if (jd->d_dialog && jd->d_dialog->local_uri)
  {
  	char buf[128];
	if (jd->d_dialog->local_uri->url &&
		jd->d_dialog->local_uri->url->host)
	{
		if (jd->d_dialog->local_uri->url->username)
			sprintf(buf,"<sip:%s@%s>",osip_uri_get_username(jd->d_dialog->local_uri->url),
				osip_uri_get_host(jd->d_dialog->local_uri->url));
		else
			sprintf(buf,"<sip:%s>", osip_uri_get_host(jd->d_dialog->local_uri->url));
	}
	else
	{
		strcpy(buf, "<sip:127.0.0.1>");
	}

  	osip_message_set_header(request, "Referred-By", buf);
  }
#endif


  i = eXosip_create_transaction(jc, jd, request);
  if (i!=0)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: cannot initiate SIP transfer transaction!"));
      return i;
    }
  return 0;
}

static int eXosip_create_cancel_transaction(eXosip_call_t *jc, eXosip_dialog_t *jd,
	osip_message_t *request)
{
  osip_event_t *sipevent;
  osip_transaction_t *tr;
  int i;
  i = osip_transaction_init(&tr,
		       NICT,
		       eXosip.j_osip,
		       request);
  if (i!=0)
    {
      /* TODO: release the j_call.. */

      osip_message_free(request);
      return -2;
    }
  
  #ifdef CONFIG_RTK_VOIP_SIP_TLS
	if(eXosip_get_transport_type(request)==TRANSPORT_TLS){
		osip_uri_t *uri=NULL;
		char *host;
		int proxyIndex;
		  
		uri=osip_from_get_url(request->from);
		host=osip_uri_get_host(uri);
		proxyIndex=eXosip_find_TCP_socket_form_proxy(jc->nPort, host);
		if(proxyIndex != -1)
			osip_transaction_set_out_socket(tr, eXosip.j_tls_sockets[jc->nPort][proxyIndex]);
	}else{
		osip_transaction_set_out_socket(tr, eXosip.j_sockets[jc->nPort]);		
	}
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  osip_transaction_set_out_socket(tr, eXosip.j_sockets[jc->nPort]);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/


  osip_list_add(eXosip.j_transactions, tr, 0);
  
  sipevent = osip_new_outgoing_sipmessage(request);
  sipevent->transactionid =  tr->transactionid;
  
  osip_transaction_add_event(tr, sipevent);
  __eXosip_wakeup();
  return 0;
}

int eXosip_terminate_call(int cid, int jid)
{
  int i;
  osip_transaction_t *tr;
  osip_message_t *request;
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  if (jid>0)
    {
      eXosip_call_dialog_find(jid, &jc, &jd);
      if (jd==NULL)
	{
     OSIP_TRACE (osip_trace
	    (__FILE__, __LINE__, OSIP_ERROR, NULL,
	     "eXosip: No call here? "));
	  return -1;
	}
    }
  else
    {
      eXosip_call_find(cid, &jc);
    }

  if (jc==NULL)
    {
      return -1;
    }

  tr=eXosip_find_last_out_invite(jc, jd);

#ifdef RTK_DNS
	if (tr)
	{
		// flush unused fifo event (especically for dns retry) if terminate
		rtk_flush_fifo_event(tr);
	} 
#endif

  if (tr!=NULL && tr->last_response!=NULL && MSG_IS_STATUS_1XX(tr->last_response))
	{
	  i = generating_cancel(jc->nPort, &request, tr->orig_request);
	  if (i!=0)
	    {
         OSIP_TRACE (osip_trace
	        (__FILE__, __LINE__, OSIP_ERROR, NULL,
	         "eXosip: cannot terminate this call! "));
	      return -2;
	    }
	  i = eXosip_create_cancel_transaction(jc, jd, request);
	  if (i!=0)
	    {
         OSIP_TRACE (osip_trace
    		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
	         "eXosip: cannot initiate SIP transaction! "));
	      return i;
	    }
	  if (jd!=NULL)
	  {
		  osip_dialog_free(jd->d_dialog);
		  jd->d_dialog = NULL;
	  }
	  return 0;
	}

  if (jd==NULL || jd->d_dialog==NULL)
    {
	  /* Check if some dialog exists */
	  jd = jc->c_dialogs;
	  if (jd!=NULL && jd->d_dialog!=NULL)
	  {
		  i = generating_bye(jc->nPort, &request, jd->d_dialog);
		  if (i!=0)
			{
			  OSIP_TRACE (osip_trace
				  (__FILE__, __LINE__, OSIP_ERROR, NULL,
				 "eXosip: cannot terminate this call! "));
			  return -2;
			}

		  i = eXosip_create_transaction(jc, jd, request);
		  if (i!=0)
			{
			  OSIP_TRACE (osip_trace
				  (__FILE__, __LINE__, OSIP_ERROR, NULL,
				 "eXosip: cannot initiate SIP transaction! "));
			  return -2;
			}

		  osip_dialog_free(jd->d_dialog);
		  jd->d_dialog = NULL;
		  return 0;
	  }

#if 1
    // rock: force terminate if no dialog exist
    if (jc->c_out_tr && jc->c_out_tr->ict_context)
    {
        jc->c_out_tr->ict_context->timer_b_start.tv_usec = 0;
        jc->c_out_tr->ict_context->timer_b_start.tv_sec = 0;
        __eXosip_wakeup();
    }
    else
    {
        // what's wrong?
         OSIP_TRACE (osip_trace
              (__FILE__, __LINE__, OSIP_ERROR, NULL,
             "Rock: force terminate failed (%x)\n", jc->c_out_tr));
    }
#endif

      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: No established dialog!"));
      return -1;
    }

  if (tr==NULL)
	{
	  /*this may not be enough if it's a re-INVITE! */
	  tr = eXosip_find_last_inc_invite(jc, jd);
	  if (tr!=NULL && tr->last_response!=NULL &&
	      MSG_IS_STATUS_1XX(tr->last_response))
	    { /* answer with 603 */
	      i = eXosip_answer_call(jid, 603, 0);
	      return i;
	    }
	}


  i = generating_bye(jc->nPort, &request, jd->d_dialog);
  if (i!=0)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: cannot terminate this call! "));
      return -2;
    }

  i = eXosip_create_transaction(jc, jd, request);
  if (i!=0)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: cannot initiate SIP transaction! "));
      return -2;
    }

	/*++Modified by Jack Chan 29/01/07 for Voip.
		It should not be freed until response is received++*/ 
  //osip_dialog_free(jd->d_dialog);
  //jd->d_dialog = NULL;
  	/*--end--*/
  return 0;
}

jauthinfo_t *
eXosip_find_authentication_info(const char *username, const char *realm)
{
  jauthinfo_t *fallback = NULL;
  jauthinfo_t *authinfo;

  for (authinfo = eXosip.authinfos;
       authinfo!=NULL;
       authinfo = authinfo->next)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_INFO2, NULL,
		   "INFO: authinfo: %s %s\n", realm, authinfo->realm));
      if (0==strcmp(authinfo->username, username))
	{
	  if (authinfo->realm == NULL || authinfo->realm[0] == '\0')
	    {
	      fallback = authinfo;
	    }
	  else if (strcmp(realm,authinfo->realm)==0 || 0==strncmp(realm+1, authinfo->realm, strlen(realm)-2))
	    {
	      return authinfo;
	    }
	}
    }
  return fallback;
}


int eXosip_clear_authentication_info(){
  jauthinfo_t *jauthinfo;
  for (jauthinfo = eXosip.authinfos; jauthinfo!=NULL;
       jauthinfo = eXosip.authinfos)
    {
      REMOVE_ELEMENT(eXosip.authinfos, jauthinfo);
      osip_free(jauthinfo);
    }
  return 0;
}

int
eXosip_add_authentication_info(const char *username, const char *userid,
			       const char *passwd, const char *ha1,
			       const char *realm)
{
  jauthinfo_t *authinfos;

  if (username==NULL || username[0]=='\0') return -1;
  if (userid==NULL || userid[0]=='\0')     return -1;

  if ( passwd!=NULL && passwd[0]!='\0')    {}
  else if (ha1!=NULL && ha1[0]!='\0')      {}
  else return -1;

  authinfos = (jauthinfo_t *) osip_malloc(sizeof(jauthinfo_t));
  if (authinfos==NULL)
    return -1;
  memset(authinfos, 0, sizeof(jauthinfo_t));

  snprintf(authinfos->username, 50, "%s", username);
  snprintf(authinfos->userid,   50, "%s", userid);
  if ( passwd!=NULL && passwd[0]!='\0')
    snprintf(authinfos->passwd,   50, "%s", passwd);
  else if (ha1!=NULL && ha1[0]!='\0')
    snprintf(authinfos->ha1,      50, "%s", ha1);
  if(realm!=NULL && realm[0]!='\0')
    snprintf(authinfos->realm,    50, "%s", realm);

  ADD_ELEMENT(eXosip.authinfos, authinfos);
  return 0;
}

static int
eXosip_add_authentication_information(osip_message_t *req,
				      osip_message_t *last_response)
{
  osip_authorization_t *aut = NULL;
  osip_www_authenticate_t *wwwauth = NULL;
  osip_proxy_authorization_t *proxy_aut = NULL;
  osip_proxy_authenticate_t *proxyauth = NULL;
  jauthinfo_t *authinfo = NULL;
  int pos;
  int i;

  if (req==NULL
      ||req->from==NULL
      ||req->from->url==NULL
      ||req->from->url->username==NULL)
    return -1;

  pos=0;
  osip_message_get_www_authenticate(last_response, pos, &wwwauth);
  osip_message_get_proxy_authenticate(last_response, pos, &proxyauth);
  if (wwwauth==NULL && proxyauth==NULL) return -1;

  while (wwwauth!=NULL)
    {
      char *uri;
      authinfo = eXosip_find_authentication_info(req->from->url->username,
						 wwwauth->realm);
      if (authinfo==NULL) return -1;
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_INFO1, NULL,
		   "authinfo: %s\n", authinfo->username));
      i = osip_uri_to_str (req->req_uri, &uri);
      if (i!=0) return -1;
      
      i = __eXosip_create_authorization_header(last_response, uri,
					       authinfo->userid,
					       authinfo->passwd,
					       &aut);
      osip_free(uri);
      if (i!=0) return -1;

      if (aut != NULL)
	{
	  osip_list_add (req->authorizations, aut, -1);
	  osip_message_force_update(req);
	}

      pos++;
      osip_message_get_www_authenticate(last_response, pos, &wwwauth);
    }

  pos=0;
  while (proxyauth!=NULL)
    {
      char *uri;
      authinfo = eXosip_find_authentication_info(req->from->url->username,
						 proxyauth->realm);
      if (authinfo==NULL) return -1;
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_INFO1, NULL,
		   "authinfo: %s\n", authinfo->username));
      i = osip_uri_to_str (req->req_uri, &uri);
      if (i!=0) return -1;
      
      i = __eXosip_create_proxy_authorization_header(last_response, uri,
						     authinfo->userid,
						     authinfo->passwd,
						     &proxy_aut);
      osip_free(uri);
      if (i!=0) return -1;

      if (proxy_aut != NULL)
	{
	  osip_list_add (req->proxy_authorizations, proxy_aut, -1);
	  osip_message_force_update(req);
	}

      pos++;
      osip_message_get_proxy_authenticate (last_response, pos, &proxyauth);
    }

  return 0;
}


static int 
eXosip_update_top_via(int nPort, osip_message_t *sip)
{
#ifdef SM
  char *locip;
#else
aaaaaaaaaaaaaaaaaaaa
  char locip[50];
#endif
  char *tmp    = (char *)osip_malloc(256*sizeof(char));
  osip_via_t *via   = (osip_via_t *) osip_list_get (sip->vias, 0);

#ifdef CONFIG_RTK_VOIP_SIP_TLS
	int bTLS=0;

	if(!osip_strcasecmp(via->protocol, "TLS"))
		bTLS=1;
	else
		bTLS=0;
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

  osip_list_remove(sip->vias, 0);
  osip_via_free(via);
#ifdef SM
  eXosip_get_localip_for(sip->req_uri->host,&locip);
#else
aaaaaaaaaaaaaaaaaa
  eXosip_guess_ip_for_via(eXosip.ip_family, locip, 49);
#endif

    /* Mandy add for stun support */
#if 1
	if (eXosip.j_firewall_ip[nPort][0] != '\0')
	{
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	if(bTLS)
		snprintf( tmp, 256, "SIP/2.0/TLS %s:%s;rport;branch=z9hG4bK%u",
			eXosip.j_firewall_ip[nPort],  eXosip.j_firewall_ports[nPort], via_branch_new_random() ) ;
	else
		snprintf( tmp, 256, "SIP/2.0/UDP %s:%s;rport;branch=z9hG4bK%u",
		//snprintf( tmp, 256, "SIP/2.0/UDP %s:%s;branch=z9hG4bK%u",
				eXosip.j_firewall_ip[nPort],  eXosip.j_firewall_ports[nPort], via_branch_new_random() ) ;
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
		snprintf( tmp, 256, "SIP/2.0/UDP %s:%s;rport;branch=z9hG4bK%u",
				eXosip.j_firewall_ip[nPort],  eXosip.j_firewall_ports[nPort], via_branch_new_random() ) ;
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
	}
	else
	{
	#endif
  if (eXosip.ip_family==AF_INET6)
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	if(bTLS)
		snprintf(tmp, 256, "SIP/2.0/TLS [%s]:%s;branch=z9hG4bK%u",
			locip,
			eXosip.localports[nPort],
			via_branch_new_random());
	else
    snprintf(tmp, 256, "SIP/2.0/UDP [%s]:%s;branch=z9hG4bK%u",
	    locip,
	    eXosip.localports[nPort],
	    via_branch_new_random());
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
    snprintf(tmp, 256, "SIP/2.0/UDP [%s]:%s;branch=z9hG4bK%u",
	    locip,
	    eXosip.localports[nPort],
	    via_branch_new_random());
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
  else
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	if(bTLS)
		snprintf(tmp, 256, "SIP/2.0/TLS %s:%s;rport;branch=z9hG4bK%u",
	    		locip,
			eXosip.localports[nPort],
			via_branch_new_random());
  else
    snprintf(tmp, 256, "SIP/2.0/UDP %s:%s;rport;branch=z9hG4bK%u",
	    locip,
	    eXosip.localports[nPort],
	    via_branch_new_random());
#else
	snprintf(tmp, 256, "SIP/2.0/UDP %s:%s;rport;branch=z9hG4bK%u",
	    		locip,
			eXosip.localports[nPort],
			via_branch_new_random());
#endif
	}
      /* End of Mandy add for stun support */

  if (eXosip.nat_type[0])
    {
      strncat(tmp, ";xxx-nat-type=", 256);
      strncat(tmp, eXosip.nat_type, 256);
    }

	      
#ifdef SM
  osip_free(locip);
#endif
  osip_via_init(&via);
  osip_via_parse(via, tmp);
  osip_list_add(sip->vias, via, 0);
  osip_free(tmp);

  return 0;
}

eXosip_reg_t *
eXosip_reg_find(int rid)
{
    eXosip_reg_t *jr;

    for (jr = eXosip.j_reg; jr != NULL; jr = jr->next)
      {
	if (jr->r_id == rid)
	  {
	    return jr;
	  }
      }
    return NULL;
}

int eXosip_register      (int rid, int registration_period)
{
  osip_transaction_t *transaction;
  osip_event_t *sipevent;
  osip_message_t *reg;
  eXosip_reg_t *jr;
  int i;

  jr = eXosip_reg_find(rid);
  if (jr==NULL)
    {
      /* fprintf(stderr, "eXosip: no registration info saved!\n"); */
      return -1;
    }
  if (registration_period>=0)
    jr->r_reg_period = registration_period;
  if (jr->r_reg_period==0)
    {} /* unregistration */
#ifndef FIX_REGISTER_TIME
  else if (jr->r_reg_period>3600)
    jr->r_reg_period = 3600;
  else if (jr->r_reg_period<200) /* too low */
    jr->r_reg_period = 200;
#endif

  reg = NULL;
  if (jr->r_last_tr!=NULL)
    {
      if (jr->r_last_tr->state!=NICT_TERMINATED
	  && jr->r_last_tr->state!=NICT_COMPLETED)
	{
	  /* fprintf(stderr, "eXosip: a registration is already pending!\n"); */
	  return -1;
	}
      else
	{
	  osip_message_t *last_response;

	  reg = jr->r_last_tr->orig_request;
	  last_response = jr->r_last_tr->last_response;

	  jr->r_last_tr->orig_request = NULL;
	  jr->r_last_tr->last_response = NULL;
	  __eXosip_delete_jinfo(jr->r_last_tr);
	  osip_transaction_free(jr->r_last_tr);
	  jr->r_last_tr = NULL;

	  /* modify the REGISTER request */
	  {
	    int osip_cseq_num = osip_atoi(reg->cseq->number);
	    int length   = strlen(reg->cseq->number);


	    osip_authorization_t *aut;
	    osip_proxy_authorization_t *proxy_aut;
	    
	    aut = (osip_authorization_t *)osip_list_get(reg->authorizations, 0);
	    while (aut!=NULL)
	      {
		osip_list_remove(reg->authorizations, 0);
		osip_authorization_free(aut);
		aut = (osip_authorization_t *)osip_list_get(reg->authorizations, 0);
	      }

	    proxy_aut = (osip_proxy_authorization_t*)osip_list_get(reg->proxy_authorizations, 0);
	    while (proxy_aut!=NULL)
	      {
		osip_list_remove(reg->proxy_authorizations, 0);
		osip_proxy_authorization_free(proxy_aut);
		proxy_aut = (osip_proxy_authorization_t*)osip_list_get(reg->proxy_authorizations, 0);
	      }


	    if (-1 == eXosip_update_top_via(jr->nPort, reg))
	      {
		osip_message_free(reg);
		return -1;
	      }

	    osip_cseq_num++;
	    osip_free(reg->cseq->number);
	    reg->cseq->number = (char*)osip_malloc(length+2); /* +2 like for 9 to 10 */
	    sprintf(reg->cseq->number, "%i", osip_cseq_num);

	    {
	      osip_header_t *exp;
	      osip_message_header_get_byname(reg, "expires", 0, &exp);
	      osip_free(exp->hvalue);
	      exp->hvalue = (char*)osip_malloc(10);
	      snprintf(exp->hvalue, 9, "%i", jr->r_reg_period);
	    }

	    osip_message_force_update(reg);
	  }

	  if (last_response!=NULL)
	    {
	      if (MSG_IS_STATUS_4XX(last_response))
		{
		  eXosip_add_authentication_information(reg, last_response);
		}
	      osip_message_free(last_response);
	    }
	}
    }
  if (reg==NULL)
    {
      i = generating_register(jr->nPort, &reg, jr->r_aor, jr->r_routes, jr->r_registrar, jr->r_contact, jr->r_reg_period);
      if (i!=0)
	{
	  /* fprintf(stderr, "eXosip: cannot register (cannot build REGISTER)! "); */
	  return -2;
	}
    }

#if 1
  // use "*" to unregister all
  if (jr->r_reg_period == 0)
  {
    osip_contact_t *contact;

    while (!osip_list_eol(reg->contacts, 0))
    {
      contact = (osip_contact_t *) osip_list_get(reg->contacts, 0);
      osip_list_remove(reg->contacts, 0);
      osip_contact_free(contact);
    }

    osip_message_set_contact(reg, "*");
  }
#endif

  i = osip_transaction_init(&transaction,
		       NICT,
		       eXosip.j_osip,
		       reg);
  if (i!=0)
    {
      /* TODO: release the j_call.. */

      osip_message_free(reg);
      return -2;
    }

#ifdef CONFIG_RTK_VOIP_SIP_TLS
	if(eXosip_get_transport_type(reg)==TRANSPORT_TLS){
		osip_uri_t *uri=NULL;
		char *host;
		int proxyIndex;
		  
		uri=osip_from_get_url(reg->from);
		host=osip_uri_get_host(uri);
				
		proxyIndex=eXosip_find_TCP_socket_form_proxy(jr->nPort, host);
		if(proxyIndex != -1)
			osip_transaction_set_out_socket(transaction, eXosip.j_tls_sockets[jr->nPort][proxyIndex]);
	}else{
  osip_transaction_set_out_socket(transaction, eXosip.j_sockets[jr->nPort]);
	}
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  osip_transaction_set_out_socket(transaction, eXosip.j_sockets[jr->nPort]);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

  jr->r_last_tr = transaction;

  /* send REGISTER */
  sipevent = osip_new_outgoing_sipmessage(reg);
  sipevent->transactionid =  transaction->transactionid;
  osip_message_force_update(reg);
  
  osip_transaction_add_event(transaction, sipevent);
  __eXosip_wakeup();
  return 0;
}

int
eXosip_register_init(int nPort, char *from, char *proxy, char *contact, osip_list_t *routes)
{
  eXosip_reg_t *jr;
  int i;

  /* Avoid adding the same registration info twice to prevent mem leaks */
  for (jr = eXosip.j_reg; jr != NULL; jr = jr->next)
    {
      if (strcmp(jr->r_aor, from) == 0
	  && strcmp(jr->r_registrar, proxy) == 0)
	{
	  return jr->r_id;
	}
    }

  /* Add new registration info */
  i = eXosip_reg_init(nPort, &jr, from, proxy, contact, routes);
  if (i!=0)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: cannot register! "));
      return i;
    }
  ADD_ELEMENT(eXosip.j_reg, jr);
  return jr->r_id;
}


int eXosip_build_publish(int nPort, osip_message_t **message,
			 char *to,
			 char *from,
			 osip_list_t *routes,
			 const char *event,
			 const char *expires,
			 const char *ctype,
			 const char *body)
{
  int i;
  if (to==NULL || to[0]=='\0')
    return -1;
  if (from==NULL || from[0]=='\0')
    return -1;
  if (event==NULL || event[0]=='\0')
    return -1;
  if (ctype==NULL || ctype[0]=='\0')
    {
      if (body!=NULL && body[0]!='\0')
	return -1;
    }
  else
    {
      if (body==NULL || body[0]=='\0')
	return -1;
    }

  i = generating_publish(nPort, message, to, from, routes);
  if (i!=0)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: cannot send message (cannot build PUBLISH)! "));
      return -1;
    }

  if (body!=NULL && body[0]!='\0' && ctype!=NULL && ctype[0]!='\0')
    {
      osip_message_set_content_type(*message, ctype);
      osip_message_set_body(*message, body, strlen(body));
      osip_message_set_header(*message, "Content-Disposition", "render;handling=required");
    }
  if (expires!=NULL && expires[0]!='\0')
    osip_message_set_expires(*message, expires);
  else
    osip_message_set_expires(*message, "3600");

  osip_message_set_header(*message, "Event", event);
  return 0;
}

int eXosip_publish(int nPort, osip_message_t *message, const char *to)
{
  /* eXosip_call_t *jc;
     osip_header_t *subject; */
  osip_transaction_t *transaction;
  osip_event_t *sipevent;
  int i;
  eXosip_pub_t *pub=NULL;

  if (message==NULL)
    return -1;
  if (message->cseq==NULL||message->cseq->number==NULL)
    return -1;
  if (to==NULL)
    {
      osip_message_free(message);
      return -1;
    }

  i = _eXosip_pub_find_by_aor(&pub, to);
  if (i!=0 || pub==NULL)
    {
      osip_header_t *expires;
      osip_message_get_expires(message, 0, &expires);
      if (expires==NULL || expires->hvalue==NULL)
	{
	  OSIP_TRACE (osip_trace
		      (__FILE__, __LINE__, OSIP_ERROR, NULL,
		       "eXosip: missing expires header in PUBLISH!"));
	  osip_message_free(message);
	  return -1;
	}
      else
	{
	  /* start a new publication context */
	  _eXosip_pub_init(nPort, &pub, to, expires->hvalue);
	  if (pub==NULL) return -1;
	  ADD_ELEMENT(eXosip.j_pub, pub);
	}
    }
  else
    {
      if (pub->p_sip_etag!=NULL && pub->p_sip_etag[0]!='\0')
	{
	  /* increase cseq */
	  osip_message_set_header(message, "SIP-If-Match", pub->p_sip_etag);
	}

      if (pub->p_last_tr!=NULL && pub->p_last_tr->cseq!=NULL
	  &&pub->p_last_tr->cseq->number!=NULL)
	{
	  int osip_cseq_num = osip_atoi(pub->p_last_tr->cseq->number);
	  int length        = strlen(pub->p_last_tr->cseq->number);
	  
	  osip_cseq_num++;
	  osip_free(message->cseq->number);
	  message->cseq->number = (char*)osip_malloc(length+2); /* +2 like for 9 to 10 */
	  sprintf(message->cseq->number, "%i", osip_cseq_num);
	}
    }

  i = osip_transaction_init(&transaction,
		       NICT,
		       eXosip.j_osip,
		       message);
  if (i!=0)
    {
      osip_message_free(message);
      return -1;
    }
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	  if(eXosip_get_transport_type(message)==TRANSPORT_TLS){
	  	osip_uri_t *uri=NULL;
		char *host;
		int proxyIndex;

		uri=osip_from_get_url(message->from);
		host=osip_uri_get_host(uri);
		proxyIndex=eXosip_find_TCP_socket_form_proxy(pub->nPort, host);
		if(proxyIndex != -1)
		  	osip_transaction_set_out_socket(transaction, eXosip.j_tls_sockets[pub->nPort][proxyIndex]);
	  }else{
  osip_transaction_set_out_socket(transaction, eXosip.j_sockets[pub->nPort]);
	  }
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  osip_transaction_set_out_socket(transaction, eXosip.j_sockets[pub->nPort]);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/


  if (pub->p_last_tr!=NULL)
    osip_list_add(eXosip.j_transactions, pub->p_last_tr, 0);
  pub->p_last_tr = transaction;
  
  sipevent = osip_new_outgoing_sipmessage(message);
  sipevent->transactionid =  transaction->transactionid;
  
  osip_transaction_set_your_instance(transaction, __eXosip_new_jinfo(nPort, NULL, NULL, NULL, NULL));
  osip_transaction_add_event(transaction, sipevent);
  __eXosip_wakeup();
  return 0;
}

int eXosip_subscribe(int nPort, char *to, char *from, osip_list_t *routes)
{
  eXosip_subscribe_t *js;
  osip_message_t *subscribe;
  osip_transaction_t *transaction;
  osip_event_t *sipevent;
  int i;
    
  i = generating_initial_subscribe(nPort, &subscribe, to, from, routes);
  if (i!=0)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: cannot subscribe (cannot build SUBSCRIBE)! "));
      return -1;
    }

  i = eXosip_subscribe_init(nPort, &js, to);
  if (i!=0)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: cannot subscribe."));
      return -1;
    }
  
  i = osip_transaction_init(&transaction,
		       NICT,
		       eXosip.j_osip,
		       subscribe);
  if (i!=0)
    {
      osip_message_free(subscribe);
      return -1;
    }
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	  if(eXosip_get_transport_type(subscribe)==TRANSPORT_TLS){
	  	osip_uri_t *uri=NULL;
		char *host;
		int proxyIndex;

		uri=osip_from_get_url(subscribe->from);
		host=osip_uri_get_host(uri);
		proxyIndex=eXosip_find_TCP_socket_form_proxy(js->nPort, host);
		if(proxyIndex != -1)
		  	  osip_transaction_set_out_socket(transaction, eXosip.j_tls_sockets[js->nPort][proxyIndex]);
	  else
  osip_transaction_set_out_socket(transaction, eXosip.j_sockets[js->nPort]);
	  }
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  osip_transaction_set_out_socket(transaction, eXosip.j_sockets[js->nPort]);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/


  _eXosip_subscribe_set_refresh_interval(js, subscribe);
  js->s_out_tr = transaction;
  
  sipevent = osip_new_outgoing_sipmessage(subscribe);
  sipevent->transactionid =  transaction->transactionid;
  
  osip_transaction_set_your_instance(transaction, __eXosip_new_jinfo(nPort, NULL, NULL, js, NULL));
  osip_transaction_add_event(transaction, sipevent);

  ADD_ELEMENT(eXosip.j_subscribes, js);
  eXosip_update(); /* fixed? */
  __eXosip_wakeup();
  return 0;
}


int eXosip_subscribe_refresh  (int sid, char *expires)
{
  int i;
  eXosip_dialog_t *jd = NULL;
  eXosip_subscribe_t *js = NULL;

  if (sid>0)
    {
      eXosip_subscribe_dialog_find(sid, &js, &jd);
    }
  if (js==NULL)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: No subscribe here?\n"));
      return -1;
    }

  if (jd==NULL)
    {
      osip_transaction_t *tr;
      osip_transaction_t *newtr;
      osip_message_t *sub;
      osip_event_t *sipevent;
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: No subscribe dialog here?\n"));
      
      tr=eXosip_find_last_out_subscribe(js,NULL);
      if (tr==NULL){
	eXosip_trace(OSIP_INFO1,("eXosip_retry_last_sub: No such transaction."));
	return -1;
      }
      if (tr->last_response==NULL){
	eXosip_trace(OSIP_INFO1,("eXosip_retry_last_sub: transaction has not been answered."));
	return -1;
      }
      sub=eXosip_prepare_request_for_auth(js->nPort, tr->orig_request);
      if (sub==NULL) return -1;
      eXosip_add_authentication_information(sub,tr->last_response);

      if (expires==NULL)
	osip_message_set_expires(sub, "3600");
      else
	osip_message_set_expires(sub, expires);

      osip_message_force_update(sub);
      i = osip_transaction_init(&newtr,
				NICT,
				eXosip.j_osip,
				sub);
      if (i!=0)
	{
	  osip_message_free(sub);
	  return -1;
	}

#ifdef CONFIG_RTK_VOIP_SIP_TLS
	if(eXosip_get_transport_type(sub)==TRANSPORT_TLS){
		osip_uri_t *uri=NULL;
		char *host;
		int proxyIndex;

		uri=osip_from_get_url(sub->from);
		host=osip_uri_get_host(uri);
		proxyIndex=eXosip_find_TCP_socket_form_proxy(js->nPort, host);
		if(proxyIndex != -1)
			osip_transaction_set_out_socket(newtr, eXosip.j_tls_sockets[js->nPort][proxyIndex]);
	}else{
	osip_transaction_set_out_socket(newtr, eXosip.j_sockets[js->nPort]);
	}
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
	osip_transaction_set_out_socket(newtr, eXosip.j_sockets[js->nPort]);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

      if (jd!=NULL)
	osip_list_add(jd->d_out_trs, newtr, 0);
      else
	{
	  js->s_out_tr = newtr;
	  /* remove old transaction */
	  osip_list_add(eXosip.j_transactions, tr, 0);
	}
      
      sipevent = osip_new_outgoing_sipmessage(sub);
      
      osip_transaction_set_your_instance(newtr, tr->your_instance);
      osip_transaction_set_your_instance(tr, NULL);
      osip_transaction_add_event(newtr, sipevent);
      
      eXosip_update(); /* fixed? */
      __eXosip_wakeup();
      return -1;
    }

#ifdef LOW_EXPIRE
aaaaaaaaaaaaaaaaaaaa
  if (expires==NULL)
    i = eXosip_subscribe_send_subscribe(js, jd, "60");
  else
    i = eXosip_subscribe_send_subscribe(js, jd, expires);
#else
  if (expires==NULL)
    i = eXosip_subscribe_send_subscribe(js, jd, "3600");
  else
    i = eXosip_subscribe_send_subscribe(js, jd, expires);
#endif
  return i;
}

int eXosip_subscribe_close(int sid)
{
  int i;
  eXosip_dialog_t *jd = NULL;
  eXosip_subscribe_t *js = NULL;

  if (sid>0)
    {
      eXosip_subscribe_dialog_find(sid, &js, &jd);
    }
  if (jd==NULL)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: No subscribe dialog here?\n"));
      return -1;
    }

  i = eXosip_subscribe_send_subscribe(js, jd, "0");
  return i;
}

int eXosip_transfer_send_notify(int jid, int subscription_status, char *body)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  
  if (jid>0)
    {
      eXosip_call_dialog_find(jid, &jc, &jd);
    }
  if (jd==NULL)
    {
       OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: No call here?\n"));
      return -1;
    }
  if (jd==NULL || jd->d_dialog==NULL)
    {
       OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: No established dialog!"));
      return -1;
    }

  return _eXosip_transfer_send_notify(jc, jd, subscription_status, body);
}

int _eXosip_transfer_send_notify(eXosip_call_t *jc, eXosip_dialog_t *jd, 
	int subscription_status, char *body)
{
  osip_transaction_t *transaction;
  osip_message_t *notify;
  osip_event_t *sipevent;
  int   i;
  char  subscription_state[50];
  char *tmp;
#ifdef CONFIG_RTK_VOIP_SIP_TLS
  osip_uri_t *uri=NULL;
  char *host;
#endif

  transaction = eXosip_find_last_inc_refer(jc, jd);
  if (transaction==NULL)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
		   "eXosip: No pending transfer!\n"));
      return -1;
    }

  transaction = eXosip_find_last_out_notify_for_refer(jc, jd);
  if (transaction!=NULL)
    {
      if (transaction->state!=NICT_TERMINATED &&
	  transaction->state!=NIST_TERMINATED)
{
	printf("=>transaction is not terminated.\n");
#if 0
	return -1;
#endif	
}
      transaction=NULL;
    }

#ifdef CONFIG_RTK_VOIP_SIP_TLS
	uri=osip_from_get_url(jd->d_dialog->local_uri);
	host=osip_uri_get_host(uri);
	if(eXosip_check_tls_proxy(jc->nPort, host))
		i = _eXosip_build_request_within_dialog(jc->nPort, &notify, "NOTIFY", jd->d_dialog, "TLS");		
	else
		i = _eXosip_build_request_within_dialog(jc->nPort, &notify, "NOTIFY", jd->d_dialog, "UDP");
#else
  i = _eXosip_build_request_within_dialog(jc->nPort, &notify, "NOTIFY", jd->d_dialog, "UDP");
#endif

  if (i!=0)
    return -2;

  if (subscription_status==EXOSIP_SUBCRSTATE_PENDING)
    osip_strncpy(subscription_state, "pending;expires=", 16);
  else if (subscription_status==EXOSIP_SUBCRSTATE_ACTIVE)
    osip_strncpy(subscription_state, "active;expires=", 15);
  else if (subscription_status==EXOSIP_SUBCRSTATE_TERMINATED)
    {
      int reason = NORESOURCE;
      if (reason==DEACTIVATED)
	osip_strncpy(subscription_state, "terminated;reason=deactivated", 29);
      else if (reason==PROBATION)
	osip_strncpy(subscription_state, "terminated;reason=probation", 27);
      else if (reason==REJECTED)
	osip_strncpy(subscription_state, "terminated;reason=rejected", 26);
      else if (reason==TIMEOUT)
	osip_strncpy(subscription_state, "terminated;reason=timeout", 25);
      else if (reason==GIVEUP)
	osip_strncpy(subscription_state, "terminated;reason=giveup", 24);
      else if (reason==NORESOURCE)
	osip_strncpy(subscription_state, "terminated;reason=noresource", 29);
    }
  tmp = subscription_state + strlen(subscription_state);
  if (subscription_status!=EXOSIP_SUBCRSTATE_TERMINATED)
    sprintf(tmp, "%i", 180);
  osip_message_set_header(notify, "Subscription-State",
			 subscription_state);

  /* add a body */
  if (body!=NULL)
    {
      osip_message_set_body(notify, body, strlen(body));
      osip_message_set_content_type(notify, "message/sipfrag");
    }

  osip_message_set_header(notify, "Event", "refer");

  i = osip_transaction_init(&transaction,
		       NICT,
		       eXosip.j_osip,
		       notify);
  if (i!=0)
    {
      osip_message_free(notify);
      return -1;
    }

#ifdef CONFIG_RTK_VOIP_SIP_TLS
	if(eXosip_get_transport_type(notify)==TRANSPORT_TLS){
		osip_uri_t *uri=NULL;
		char *host;
		int proxyIndex;

		uri=osip_from_get_url(notify->from);
		host=osip_uri_get_host(uri);
		proxyIndex=eXosip_find_TCP_socket_form_proxy(jc->nPort, host);
		if(proxyIndex != -1)
			osip_transaction_set_out_socket(transaction, eXosip.j_tls_sockets[jc->nPort][proxyIndex]);
	}else{
  osip_transaction_set_out_socket(transaction, eXosip.j_sockets[jc->nPort]);
	}
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  osip_transaction_set_out_socket(transaction, eXosip.j_sockets[jc->nPort]);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
  
  osip_list_add(jd->d_out_trs, transaction, 0);
  
  sipevent = osip_new_outgoing_sipmessage(notify);
  sipevent->transactionid =  transaction->transactionid;
  
  osip_transaction_set_your_instance(transaction, __eXosip_new_jinfo(jc->nPort, jc, jd, NULL, NULL));
  osip_transaction_add_event(transaction, sipevent);
  __eXosip_wakeup();
  return 0;
}

int eXosip_notify_send_notify(eXosip_notify_t *jn, eXosip_dialog_t *jd,
	int subscription_status, int online_status)
{
  osip_transaction_t *transaction;
  osip_message_t *notify;
  osip_event_t *sipevent;
  int   i;
  char  subscription_state[50];
  char *tmp;
#ifdef CONFIG_RTK_VOIP_SIP_TLS
  osip_uri_t *uri=NULL;
  char *host;
#endif

  int   now = time(NULL);
  transaction = eXosip_find_last_out_notify(jn, jd);
  if (transaction!=NULL)
    {
      if (transaction->state!=NICT_TERMINATED &&
	  transaction->state!=NIST_TERMINATED)
	return -1;
      transaction=NULL;
    }

#ifndef SUPPORT_MSN

#else
aaaaaaaaaaaaaaaaaaaaaa

  /* DO NOT SEND ANY NOTIFY when the status
     is not active (or terminated?) */
  if (subscription_status!=EXOSIP_SUBCRSTATE_ACTIVE
      && subscription_status!=EXOSIP_SUBCRSTATE_TERMINATED)
    {
      /* set the new state anyway! */
      jn->n_online_status = online_status;
      jn->n_ss_status = subscription_status;
      return -1;
    }

#endif

#ifdef CONFIG_RTK_VOIP_SIP_TLS
	uri=osip_from_get_url(jd->d_dialog->local_uri);
	host=osip_uri_get_host(uri);
	if(eXosip_check_tls_proxy(jn->nPort, host))
		i = _eXosip_build_request_within_dialog(jn->nPort, &notify, "NOTIFY", jd->d_dialog, "TLS");
	else
		i = _eXosip_build_request_within_dialog(jn->nPort, &notify, "NOTIFY", jd->d_dialog, "UDP");
#else
  i = _eXosip_build_request_within_dialog(jn->nPort, &notify, "NOTIFY", jd->d_dialog, "UDP");
#endif
  if (i!=0)
    return -2;

  jn->n_online_status = online_status;
  jn->n_ss_status = subscription_status;

  /* add the notifications info */
  if (jn->n_ss_status==EXOSIP_SUBCRSTATE_UNKNOWN)
    jn->n_online_status=EXOSIP_SUBCRSTATE_PENDING;

#ifndef SUPPORT_MSN
  if (jn->n_ss_status==EXOSIP_SUBCRSTATE_PENDING)
    osip_strncpy(subscription_state, "pending;expires=", 16);
  else if (jn->n_ss_status==EXOSIP_SUBCRSTATE_ACTIVE)
    osip_strncpy(subscription_state, "active;expires=", 15);
  else if (jn->n_ss_status==EXOSIP_SUBCRSTATE_TERMINATED)
    {
      if (jn->n_ss_reason==DEACTIVATED)
	osip_strncpy(subscription_state, "terminated;reason=deactivated", 29);
      else if (jn->n_ss_reason==PROBATION)
	osip_strncpy(subscription_state, "terminated;reason=probation", 27);
      else if (jn->n_ss_reason==REJECTED)
	osip_strncpy(subscription_state, "terminated;reason=rejected", 26);
      else if (jn->n_ss_reason==TIMEOUT)
	osip_strncpy(subscription_state, "terminated;reason=timeout", 25);
      else if (jn->n_ss_reason==GIVEUP)
	osip_strncpy(subscription_state, "terminated;reason=giveup", 24);
      else if (jn->n_ss_reason==NORESOURCE)
	osip_strncpy(subscription_state, "terminated;reason=noresource", 29);
    }
  tmp = subscription_state + strlen(subscription_state);
  if (jn->n_ss_status!=EXOSIP_SUBCRSTATE_TERMINATED)
    sprintf(tmp, "%i", jn->n_ss_expires-now);
  osip_message_set_header(notify, "Subscription-State",
			 subscription_state);
#endif

  /* add a body */
  i = _eXosip_notify_add_body(jn, notify);
  if (i!=0)
    {

    }

#ifdef SUPPORT_MSN
aaaaaaaaaaaaaaaaaaaaaa
#else
  osip_message_set_header(notify, "Event", "presence");
#endif

  i = osip_transaction_init(&transaction,
		       NICT,
		       eXosip.j_osip,
		       notify);
  if (i!=0)
    {
      /* TODO: release the j_call.. */
      osip_message_free(notify);
      return -1;
    }
  
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	if(eXosip_get_transport_type(notify)==TRANSPORT_TLS){
		osip_uri_t *uri=NULL;
		char *host;
		int proxyIndex;

		uri=osip_from_get_url(notify->from);
		host=osip_uri_get_host(uri);
		proxyIndex=eXosip_find_TCP_socket_form_proxy(jn->nPort, host);
		if(proxyIndex != -1)
			osip_transaction_set_out_socket(transaction, eXosip.j_tls_sockets[jn->nPort][proxyIndex]);
	}else{
  osip_transaction_set_out_socket(transaction, eXosip.j_sockets[jn->nPort]);
	}
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  osip_transaction_set_out_socket(transaction, eXosip.j_sockets[jn->nPort]);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/


  osip_list_add(jd->d_out_trs, transaction, 0);
  
  sipevent = osip_new_outgoing_sipmessage(notify);
  sipevent->transactionid =  transaction->transactionid;
  
  osip_transaction_set_your_instance(transaction, __eXosip_new_jinfo(jn->nPort, NULL, jd, NULL, jn));
  osip_transaction_add_event(transaction, sipevent);
  __eXosip_wakeup();
  return 0;
}

int eXosip_notify  (int nid, int subscription_status, int online_status)
{
  int i;
  eXosip_dialog_t *jd = NULL;
  eXosip_notify_t *jn = NULL;

  if (nid>0)
    {
      eXosip_notify_dialog_find(nid, &jn, &jd);
    }
  if (jd==NULL)
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: No subscribe dialog here?\n"));
      return -1;
    }

  i = eXosip_notify_send_notify(jn, jd, subscription_status, online_status);
  return i;
}


int eXosip_notify_accept_subscribe(int nid, int code,
				   int subscription_status,
				   int online_status)
{
  int i = 0;
  eXosip_dialog_t *jd = NULL;
  eXosip_notify_t *jn = NULL;
  if (nid>0)
    {
      eXosip_notify_dialog_find(nid, &jn, &jd);
    }
  if (jd==NULL)
    {
       OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: No call here?\n"));
      return -1;
    }
  if (code>100 && code<200)
    {
      eXosip_notify_answer_subscribe_1xx(jn, jd, code);
    }
  else if (code>199 && code<300)
    {
      eXosip_notify_answer_subscribe_2xx(jn, jd, code);
      i = eXosip_notify(nid, subscription_status, online_status);
    }
  else if (code>300 && code<699)
    {
      eXosip_notify_answer_subscribe_3456xx(jn, jd, code);
    }
  else
    {
      OSIP_TRACE (osip_trace
		  (__FILE__, __LINE__, OSIP_ERROR, NULL,
         "eXosip: wrong status code (101<code<699)\n"));
      return -1;
    }

  return i;
}

#if 0
void eXosip_guess_contact_uri(const char *url, char *strbuf, int bufsize, int public_net)
{
  int i;
  osip_from_t *a_from;
  char *contact = strbuf;
  char locip[50];

  eXosip_guess_ip_for_via(eXosip.ip_family, locip, sizeof(locip) - 1);
  contact[0] = 0;

  i = osip_from_init(&a_from);
  if (i==0)
    i = osip_from_parse(a_from, url);
  
  if (i==0 && a_from!=NULL
      && a_from->url!=NULL && a_from->url->username!=NULL )
    {
      if (eXosip.j_firewall_ip[0]!='\0')
	{
	  if (public_net)
	    {
	      if (eXosip.j_firewall_port==NULL)
		snprintf(contact, bufsize, "<sip:%s@%s>", a_from->url->username,
			 eXosip.j_firewall_ip);
	      else
		snprintf(contact, bufsize, "<sip:%s@%s:%s>", a_from->url->username,
			 eXosip.j_firewall_ip,
			 eXosip.j_firewall_port);
		}
	  else
	    {
	      if (eXosip.localport==NULL)
		snprintf(contact, bufsize, "<sip:%s@%s>", a_from->url->username,
			locip);
	      else
		snprintf(contact, bufsize, "<sip:%s@%s:%s>", a_from->url->username,
			locip,
			eXosip.localport);
	    }
	}
      else
	{
	  if (eXosip.localport==NULL)
	    snprintf(contact, bufsize, "<sip:%s@%s>", a_from->url->username,
		    locip);
	  else
	    snprintf(contact, bufsize, "<sip:%s@%s:%s>", a_from->url->username,
		    locip,
		    eXosip.localport);
	}
	  
      osip_from_free(a_from);
    }
} 
#endif

void eXosip_set_answer_contact(const char *contacturl)
{
  osip_strncpy(eXosip.answer_contact, contacturl ? contacturl : "", sizeof(eXosip.answer_contact)-1);
}

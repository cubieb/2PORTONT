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


#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <netdb.h>
#include "eXosip.h"
#include "eXosip2.h"
#include "eXosip_cfg.h"
#ifdef FIX_REGISTER_TIME
#include "osip_contact.h"
#endif
#include <errno.h>

#ifdef RTK_DNS
#include "dns_api.h"
#endif

#ifdef NEW_STUN
#include "stun_udp.h"
#include "stun.h"
#include "stun_api.h"
#endif

#include "sdphandler.h"

#ifdef CONFIG_RTK_VOIP_SIP_TLS
#include "eXosip_tls.h"
extern tlsObject_t sipTLSObj[MAX_VOIP_PORTS][MAX_PROXY];
#endif


extern eXosip_t eXosip;
char strTempPort[12];

/* Private functions */
static void rcvregister_failure(int type, osip_transaction_t *tr,osip_message_t *sip);
static int cb_udp_snd_message(osip_transaction_t *tr, osip_message_t *sip,
			      char *host, int port, int out_socket);
static void cb_ict_kill_transaction(int type, osip_transaction_t *tr);
static void cb_ist_kill_transaction(int type, osip_transaction_t *tr);
static void cb_nict_kill_transaction(int type, osip_transaction_t *tr);
static void cb_nist_kill_transaction(int type, osip_transaction_t *tr);
static void cb_rcvinvite  (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcvack     (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcvack2    (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcvregister(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcvbye     (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcvcancel  (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcvinfo    (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcvoptions (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcvnotify  (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcvsubscribe (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcvunkrequest(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_sndinvite  (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_sndack     (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_sndregister(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_sndbye     (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_sndcancel  (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_sndinfo    (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_sndoptions (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_sndnotify  (int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_sndsubscribe(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_sndunkrequest(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcv1xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcv2xx_4invite(osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcv2xx_4subscribe(osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcv2xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcv3xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcv4xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcv5xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcv6xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_snd1xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_snd2xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_snd3xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_snd4xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_snd5xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_snd6xx(int type, osip_transaction_t *tr,osip_message_t *sip);
static void cb_rcvresp_retransmission(int type, osip_transaction_t *tr, osip_message_t *sip);
static void cb_sndreq_retransmission(int type, osip_transaction_t *tr, osip_message_t *sip);
static void cb_sndresp_retransmission(int type, osip_transaction_t *tr, osip_message_t *sip);
static void cb_rcvreq_retransmission(int type, osip_transaction_t *tr, osip_message_t *sip);
static void cb_transport_error(int type, osip_transaction_t *tr, int error);
static void report_call_event_with_status(int evt, eXosip_call_t *jc, eXosip_dialog_t *jd, osip_message_t *sip);
static void report_event_with_status(eXosip_event_t *je, osip_message_t *sip);

#ifdef NEW_STUN

#define STUN_TIMEOUT 30

static struct sockaddr_in g_keep_alive_addr[MAX_VOIP_PORTS];

int rtk_stun_init(int chid)
{
	struct timeval now;

	eXosip.j_firewall_ip[chid][0] = 0;

	osip_gettimeofday(&now, NULL);
	eXosip.j_firewall_ports_expire[chid] = now;
	eXosip.j_firewall_fax_ports_expire[chid] = now;
	eXosip.j_firewall_media_ports_expire[chid * 2] = now;
	eXosip.j_firewall_media_ports_expire[chid * 2 + 1] = now;
	rtk_stun_keep_alive(chid, (void *) -1);

	return 0;
}

int rtk_stun_update(int chid, int ssid, int type)
{
	struct timeval expire;

	osip_gettimeofday(&expire, NULL);
	add_gettimeofday(&expire, STUN_TIMEOUT * 1000);
	if (type & STUN_UPDATE_SIP)
	{
		eXosip.j_firewall_ports_expire[chid] = expire;
	}

	if (type & STUN_UPDATE_RTP)
	{
		eXosip.j_firewall_media_ports_expire[chid * 2 + ssid] = expire;
	}

	if (type & STUN_UPDATE_T38)
	{
		eXosip.j_firewall_fax_ports_expire[chid] = expire;
	}

	return 0;
}

int rtk_stun_check(int chid, int ssid, int type)
{
	struct timeval now;

	osip_gettimeofday(&now, NULL);
	if (type & STUN_UPDATE_SIP)
	{
		if (osip_timercmp(&now, &eXosip.j_firewall_ports_expire[chid], >=))
			return -1;
	}

	if (type & STUN_UPDATE_RTP)
	{
		if (osip_timercmp(&now, &eXosip.j_firewall_media_ports_expire[chid * 2 + ssid], >=))
			return -1;
	}

	if (type & STUN_UPDATE_T38)
	{
		if (osip_timercmp(&now, &eXosip.j_firewall_fax_ports_expire[chid], >=))
			return -1;
	}

	return 0;
}

int rtk_stun_send(int chid, int ssid, int flag)
{
	voipCfgPortParam_t *voip_ptr = &g_pVoIPCfg->ports[chid];
	struct addrinfo *addrinfo;
	struct in_addr addr;
	int i;

	i = eXosip_get_addrinfo(&addrinfo, voip_ptr->stun_addr, voip_ptr->stun_port);
	if (i==0)
	{
		addr = ((struct sockaddr_in *) addrinfo->ai_addr)->sin_addr;
		freeaddrinfo(addrinfo);
		OSIP_TRACE (osip_trace
			(__FILE__, __LINE__, OSIP_INFO1, NULL,
			"Here is the resolved STUN server ip address=%s.\n", inet_ntoa(addr)));
	}
	else 
	{
		OSIP_TRACE (osip_trace
			(__FILE__, __LINE__, OSIP_INFO1, NULL,
			"We can't resolve the STUN server ip address...%s\n", voip_ptr->stun_addr));
		rtk_stun_init(chid); // dns failed
		return i;
	}

	if (flag & STUN_UPDATE_SIP) 
	{
		if (!stun_sendmessage(eXosip.j_sockets[chid], 
			addr.s_addr, voip_ptr->stun_port))
		{
			rtk_stun_init(chid); // stun failed
			return -1;
		}
	}

	if (flag & STUN_UPDATE_RTP) 
	{
		if (eXosip.j_media_sockets[chid * 2 + ssid] == -1)
		{
			int port;
			port = atoi(eXosip.media_ports[chid]) + ssid * 2;
			if (!eXosip_bind_socket(&eXosip.j_media_sockets[chid * 2 + ssid], port))
				g_error("rtk_stun_send: bind rtp socket failed\n");

			printf("stun malloc rtp%d socket %d\n", chid * 2 + ssid, port);
		}

		if (!stun_sendmessage(eXosip.j_media_sockets[chid * 2 + ssid], 
			addr.s_addr, voip_ptr->stun_port))
		{
			rtk_stun_init(chid); // stun failed
			return -1;
		}
	}

	if (flag & STUN_UPDATE_T38)
	{
		if (eXosip.j_t38_sockets[chid] == -1)
		{
			int port;
			port = atoi(eXosip.fax_ports[chid]);
			if (!eXosip_bind_socket(&eXosip.j_t38_sockets[chid], port))
				g_error("rtk_stun_send: bind t38 socket failed\n");
		}

		if (!stun_sendmessage(eXosip.j_t38_sockets[chid], 
			addr.s_addr, voip_ptr->stun_port))
		{
			rtk_stun_init(chid); // stun failed
			return -1;
		}
	}

	return 0;
}

int rtk_stun_keep_alive(int chid, struct __eXosip_sockaddr *addr)
{
	voipCfgPortParam_t *voip_ptr = &g_pVoIPCfg->ports[chid];
	char buf[2];

	if ((voip_ptr->stun_enable == 0) || (voip_ptr->stun_addr[0] == '\0'))
		return 0;

	if (addr == NULL)
	{
		if (g_keep_alive_addr[chid].sin_addr.s_addr)
		{
			return sendto(eXosip.j_sockets[chid], buf, sizeof(buf), 0, 
				&g_keep_alive_addr[chid], sizeof(g_keep_alive_addr[0])); 
		}
	}
	else if ((int) addr == -1)
	{
		memset(&g_keep_alive_addr[chid], 0, sizeof(g_keep_alive_addr[0]));
	}
	else
	{
		memcpy(&g_keep_alive_addr[chid], addr, sizeof(g_keep_alive_addr[0]));
	}

	return 0;
}

#endif

#ifdef SIP_PING

//#define SIP_PING_DEBUG
#define PING_TIMEOUT 30

static char *ping_from[MAX_VOIP_PORTS] = {0};
static char *ping_to[MAX_VOIP_PORTS] = {0};
static osip_list_t *ping_routes[MAX_VOIP_PORTS] = {0};

int rtk_ping_init(int chid, char *from, char *to, osip_list_t *routes)
{
	ping_from[chid] = from;
	ping_to[chid] = to;
	ping_routes[chid] = routes;

	eXosip.pings[chid] = NULL;
	eXosip.pinged_ip[chid][0] = 0;
	return 0;
}

int rtk_ping_close(int chid)
{
	ping_from[chid] = NULL;
	ping_to[chid] = NULL;
	ping_routes[chid] = NULL;

	eXosip.pings[chid] = NULL;
	eXosip.pinged_ip[chid][0] = 0;
	return 0;
}

int rtk_ping_send(int chid)
{
	osip_transaction_t *tr;

	tr = eXosip.pings[chid];
	if (tr && 
		tr->state != NICT_TERMINATED &&
		tr->state != NICT_COMPLETED)
	{
		OSIP_TRACE (osip_trace
			(__FILE__, __LINE__, OSIP_WARNING, NULL,
			"rtk_ping_send: ping is running\n"));
#ifdef SIP_PING_DEBUG
		printf("pinging...\n");
#endif
		return 0;
	}

	eXosip_ping(chid, ping_from[chid], ping_to[chid], ping_routes[chid], &tr);
	if (tr == NULL)
		return -1;

#ifdef SIP_PING_DEBUG
	printf("new ping\n");
#endif
	eXosip.pings[chid] = tr;
	return 0;
}

int rtk_ping_update(int chid, const char *pinged_ip, const char *pinged_port)
{
	struct timeval expire;

	strcpy(eXosip.pinged_ip[chid], pinged_ip);
	strcpy(eXosip.pinged_ports[chid], pinged_port);

	osip_gettimeofday(&expire, NULL);
	add_gettimeofday(&expire, PING_TIMEOUT * 1000);
	eXosip.pinged_ports_expire[chid] = expire;
	return 0;
}

int rtk_ping_check(int chid)
{
	struct timeval now;

	if (eXosip.pinged_ip[chid][0] == 0)
	{
#ifdef SIP_PING_DEBUG
		printf("rtk_ping_check [%d]: pinged ip failed\n", chid);
#endif
		return -1;
	}

	osip_gettimeofday(&now, NULL);
	if (osip_timercmp(&now, &eXosip.pinged_ports_expire[chid], >=))
	{
#ifdef SIP_PING_DEBUG
		printf("rtk_ping_check [%d]: pinged time failed %d vs %d\n", 
			chid, now.tv_sec, eXosip.pinged_ports_expire[chid].tv_sec);
#endif
		// timeout, reset transaction
		eXosip.pings[chid] = NULL;
		return -1;
	}

	return 0;
}

#endif

int rtk_update_via(int nPort, osip_message_t *sip, const char *local_ip, const char *local_port)
{
	osip_via_t *via;
	int i;
	osip_generic_param_t *tag_param_remote;

	via = (osip_via_t *) osip_list_get(sip->vias, 0);
	if (via == NULL || via->host == NULL)
	{
		// invalid via
		return -1;
	}

#if 0
	if (strcmp(via->host, "127.0.0.1") != 0 &&
		strcmp(via->host, "0.0.0.0") != 0)
	{
		return 0;
	}
#endif

	OSIP_TRACE(osip_trace(__FILE__, __LINE__, OSIP_INFO1, NULL, "via = %s\n", via->host));

	// update via as nat ip and port
	osip_free(via->host);
	via_set_host(via, osip_strdup(local_ip));
	osip_free(via->port);
	via_set_port(via, osip_strdup(local_port));
	/* Mandy add for stun support */
	#if 1
	i = osip_from_get_tag (sip->to, &tag_param_remote);
	if (i != 0)	// no to tag, its new request
	{
		osip_free(sip->call_id->host);
		osip_call_id_set_host(sip->call_id, osip_strdup(local_ip));
	}
	#endif
	/* End of Mandy add for stun support */

	return 0;
}

int rtk_update_contact(int nPort, osip_message_t *sip, const char *local_ip, const char *local_port)
{
	osip_contact_t *contact;

	if (osip_message_get_contact(sip, 0, &contact) != 0)
		return -1;

	if (contact == NULL)
		return 0; // no contact

	if (MSG_IS_REGISTER(sip) &&
		contact->displayname &&
		(strncmp(contact->displayname, "*", 1) == 0))
	{
		// unregister all
		return 0;
	}
	
	if (contact->url->host)
		osip_free(contact->url->host);

	if (contact->url->port)
		osip_free(contact->url->port);

	contact->url->host = osip_strdup(local_ip);
	contact->url->port = osip_strdup(local_port);
	return 0;
}

sdp_message_t *rtk_get_sdp_info(osip_message_t *sip)
{
	osip_content_type_t *ctt;
	osip_mime_version_t *mv;
	sdp_message_t *sdp;
	osip_body_t *oldbody;
	int pos = 0, i = 0;

	if (sip == NULL)
		return NULL;

	/* get content-type info */
	ctt = osip_message_get_content_type(sip);
	mv = osip_message_get_mime_version(sip);
	if (mv == NULL && ctt == NULL)
	{
//		g_warning("previous message was not correct or empty");
		return NULL;
	}
	
	if (mv != NULL)
	{
		/* look for the SDP body */
		/* ... */
	}
	else if (ctt != NULL)
	{
		if (ctt->type == NULL || ctt->subtype == NULL)
		{
			g_warning("it can be application/sdp or mime...\n");
			return NULL;
		}

		if (osip_strcasecmp(ctt->type , "application" ) || osip_strcasecmp(ctt->subtype, "sdp"))
		{
			g_warning("it can not be application/sdp type !!\n");
			return NULL;
		}
	}

	while (!osip_list_eol(sip->bodies, pos))
	{
		oldbody = (osip_body_t *) osip_list_get(sip->bodies, pos);
		pos++ ;

		sdp_message_init(&sdp);
		i = sdp_message_parse(sdp, oldbody->body);
		if (i == 0)
			return sdp;

		sdp_message_free(sdp);
		sdp = NULL;
	}

	return NULL;
}

int rtk_get_media_port(int nPort, osip_transaction_t *tr, osip_message_t *sip, char **media_port,
	int bUseStun)
{
	LinphoneCall *call;
	jinfo_t *jinf;
	int chid, s_id;

	jinf = (jinfo_t *) osip_transaction_get_your_instance(tr);
	if (NULL == jinf || NULL == jinf->jc || NULL == jinf->jc->external_reference)
		return -1;

	/* get the session number */
	// Don't use external_reference check, it may be destroied by EXOSIP_CALL_CLOSED (bye message)
	if (linphone_find_chid_ssid(solar_ptr, jinf->jc->c_id, &chid, &s_id) != 0)
	{
		g_warning("rtk_get_media_port: Call not found\n");
		return -1;
	}

	call = (LinphoneCall *) jinf->jc->external_reference;
	if (call == NULL)
	{
		g_warning("rtk_get_media_port: Call is NULL\n");
		return -1;
	}
	
	// check STUN
	if (bUseStun)
	{
		if (call->t38_params.initialized)
		{
#ifdef NEW_STUN
			if (rtk_stun_check(nPort, -1, STUN_UPDATE_T38) != 0)
			{
				rtk_stun_send(nPort, s_id, STUN_UPDATE_T38);
				return 1; // need stun retry
			}

			// close socket if stun check t38 ok 
			if (eXosip.j_t38_sockets[nPort] != -1)
			{
				close(eXosip.j_t38_sockets[nPort]);
				eXosip.j_t38_sockets[nPort] = -1;
			}
#endif
			(*media_port) = eXosip.j_firewall_fax_ports[nPort];
		}
		else
		{
#ifdef NEW_STUN
			if (rtk_stun_check(nPort, s_id, STUN_UPDATE_RTP) != 0)
			{
				rtk_stun_send(nPort, s_id, STUN_UPDATE_RTP);
				return 1; // need stun retry
			}

			// close socket if stun check rtp ok 
			if (eXosip.j_media_sockets[nPort * 2 + s_id] != -1)
			{
				printf("stun close rtp%d socket %d\n", nPort * 2 + s_id, 
					atoi(eXosip.media_ports[nPort]) + s_id * 2);
				close(eXosip.j_media_sockets[nPort * 2 + s_id]);
				eXosip.j_media_sockets[nPort * 2 + s_id] = -1;
			}
#endif
			(*media_port) = eXosip.j_firewall_media_ports[nPort * 2 + s_id];
		}
	}
	else
	{
		if (call->t38_params.initialized)
		{
			(*media_port) = eXosip.fax_ports[nPort];
		}
		else
		{
			int itempPort;
			
			itempPort = atoi(eXosip.media_ports[nPort]) +  s_id * 2;
			sprintf(strTempPort, "%d", itempPort);
			(*media_port) = strTempPort;
		}
	}

	return 0;
}

int rtk_update_sdp(int nPort, osip_transaction_t *tr, osip_message_t *sip, const char *local_ip, 
	const char *media_port, char *orig_local_ip)
{
	LinphoneCall *call;
	jinfo_t *jinf;
	int chid, s_id;
	sdp_message_t *sdp;
	char *szbody;
	char szContentLen[10];
	int pos;
    osip_body_t *body;
	char *c_address = NULL;
	sdp_connection_t *conn = NULL;

	jinf = (jinfo_t *) osip_transaction_get_your_instance(tr);
	if (NULL == jinf || NULL == jinf->jc || NULL == jinf->jc->external_reference)
		return -1;

	/* get the session number */
	// Don't use external_reference check, it may be destroied by EXOSIP_CALL_CLOSED (bye message)
	if (linphone_find_chid_ssid(solar_ptr, jinf->jc->c_id, &chid, &s_id) != 0)
	{
		g_warning("rtk_get_media_port: Call not found\n");
		return -1;
	}

	call = (LinphoneCall *) jinf->jc->external_reference;
	if (call == NULL || call->sdpctx == NULL)
	{
		g_warning("rtk_update_sdp: Call is NULL\n");
		return -1;
	}

	sdp = rtk_get_sdp_info(sip);
	if (sdp == NULL)
	{
		return 0; // no sdp
	}

	// rock: check c_connection
	if (sdp_message_connection_get(sdp, -1, 0) == NULL &&
		sdp_message_connection_get(sdp, 0, 0) == NULL)
	{
		sdp_message_free(sdp);
		return -1;
	}

	if (tr->ctx_type == IST)
	{
		// rock: get global c_connection in session
		c_address = sdp_message_c_addr_get(call->sdpctx->remote, -1, 0);
		// rock: check c_connection in first media line
		if (c_address == NULL)
			c_address = sdp_message_c_addr_get(call->sdpctx->remote, 0, 0);
		// rock: shouldn't occur
		if (c_address == NULL)
			g_error("c_address is null?\n");

		// don't update connection address if call be hold
		if (strcmp(c_address, "0.0.0.0") == 0)
		{
			sdp_message_free(sdp);
			return 0;
		}
	}
	else
	{
		// rock: get global c_connection in session
		c_address = sdp_message_c_addr_get(sdp, -1, 0);
		// rock: check c_connection in first media line
		if (c_address == NULL)
			c_address = sdp_message_c_addr_get(sdp, 0, 0);
		// rock: shouldn't occur
		if (c_address == NULL)
			g_error("c_address is null?\n");

		// don't update connection address if call hold
		if (strcmp(c_address, "0.0.0.0") == 0)
		{
			sdp_message_free(sdp);
			return 0;
		}
	}

	if (call->sdpctx->remote) // update local ip from SDP
	{
		int i;
		char *local_ip_tmp;

		// rock: get global c_connection in session
		c_address = sdp_message_c_addr_get(call->sdpctx->remote, -1, 0);
		// rock: check c_connection in first media line
		if (c_address == NULL)
			c_address = sdp_message_c_addr_get(call->sdpctx->remote, 0, 0);
		// rock: shouldn't occur
		if (c_address == NULL)
			g_error("c_address is null?\n");

		// TODO: c_addr is FQDN ?
		i = eXosip_get_localip_for(
			c_address,
			&local_ip_tmp
		);

		if (i != 0)
		{
			sdp_message_free(sdp);
			osip_free(local_ip_tmp);
		#ifdef RTK_DNS
			return i;	// dns failed or retry
		#else
			return -1;	// dns failed only
		#endif
		}

		osip_free(call->sdpctx->localip);
		osip_free(call->sdpctx->rtp_localip);
		if (orig_local_ip[0]) // STUN or SIP PING has used
		{
			if (strcmp(orig_local_ip, local_ip_tmp) == 0) // SIP and SDP use the same interface
			{
				call->sdpctx->localip = osip_strdup(local_ip); // SDP use STUN ip
				call->sdpctx->rtp_localip = osip_strdup(orig_local_ip); // RTP use orig local ip
			}
			else // SIP and SDP in the different interface (ex: SIP use eth1, SDP use br0)
			{
				int itempPort;

				call->sdpctx->localip = osip_strdup(local_ip_tmp); // SDP use new local ip to replace
				call->sdpctx->rtp_localip = osip_strdup(local_ip_tmp); // RTP use new local ip to replace
				// media port should use default media port to replace
				itempPort = atoi(eXosip.media_ports[nPort]) +  s_id * 2;
				sprintf(strTempPort, "%d", itempPort);
				g_warning("rtk_update_sdp: local address change, %s:%s -> %s:%s\n",
					orig_local_ip, media_port, call->sdpctx->localip, strTempPort);
				media_port = strTempPort;
			}
		}
		else 
		{
			if (strcmp(local_ip, local_ip_tmp) == 0) // SIP and SDP use the same interface
			{
				call->sdpctx->localip = osip_strdup(local_ip); // SDP use orig local ip
				call->sdpctx->rtp_localip = osip_strdup(local_ip); // RTP use orig local ip
			}
			else // SDP use different interface (ex: SIP use eth1, SDP use br0)
			{
				call->sdpctx->localip = osip_strdup(local_ip_tmp); // SDP use new local ip to replace
				call->sdpctx->rtp_localip = osip_strdup(local_ip_tmp); // RTP use new local ip to replace
				g_warning("rtk_update_sdp: local address change, %s:%s -> %s:%s\n",
					local_ip, media_port, call->sdpctx->localip, media_port);
			}
		}
		osip_free(local_ip_tmp);
	}
	else // update local ip from SIP
	{
		osip_free(call->sdpctx->localip);
		call->sdpctx->localip = osip_strdup(local_ip);
		osip_free(call->sdpctx->rtp_localip);
		if (orig_local_ip[0]) // STUN or SIP PING has used
			call->sdpctx->rtp_localip = osip_strdup(orig_local_ip);	// RTP should use orig local ip
		else
			call->sdpctx->rtp_localip = osip_strdup(local_ip);	
	}

	// rock: get global c_connection in session
	conn = sdp_message_connection_get(sdp, -1, 0);
	// rock: check c_connection in first media line
	if (conn == NULL)
		conn = sdp_message_connection_get(sdp, 0, 0);
	// rock: shouldn't occur
	if (conn == NULL)
		g_error("conn is null?\n");

	if (strcmp(conn->c_addr, call->sdpctx->localip) == 0 &&
		(strcmp(sdp_message_m_port_get(sdp, 0), media_port) == 0))
	{
		// the same ip & port
		sdp_message_free(sdp);
		return 0;
	}

	osip_free(conn->c_addr);
	conn->c_addr = osip_strdup(call->sdpctx->localip);
	sdp_message_m_port_set(sdp, 0, osip_strdup(media_port));
	sdp_message_to_str(sdp, &szbody);
	sdp_message_free(sdp); // free sdp message

	if (szbody == NULL)
	{
		return -1;
	}

	// update content length
	sprintf(szContentLen, "%i", strlen(szbody));
	osip_content_length_free(sip->content_length);
	sip->content_length = NULL;
	osip_message_set_content_length(sip, szContentLen);

	// clear old body
	pos = 0;
	while (!osip_list_eol (sip->bodies, pos))
	{
		body = (osip_body_t *) osip_list_get (sip->bodies, pos);
		osip_list_remove (sip->bodies, pos);
		osip_body_free (body);
	}

	// create new body
	osip_message_set_body(sip, szbody, strlen(szbody));
	osip_free(szbody);

	return 0;
}

int rtk_fix_uri(osip_uri_t *uri, char *host, char *port)
{
	if (uri->host)
	{
		osip_free(uri->host);
	}

	uri->host = osip_strdup(host);

	if (uri->port)
	{
		osip_free(uri->port);
		uri->port = NULL;
	}

	if (port && strcmp(port, "0") != 0 && strcmp(port, "5060") != 0)
	{
		uri->port = osip_strdup(port);
	}

	return 0;
}

int rtk_fix_domain(osip_message_t *sip, char *domain_name, int fix)
{
	static char *orig_host;
	static char *orig_port;

	if (fix)
	{
		osip_uri_t *url;
		char *domain;

		orig_host = osip_strdup(sip->req_uri->host);
		orig_port = osip_strdup(sip->req_uri->port);

		// parsing sip domain in WEB pages
		osip_uri_init(&url);
		if (url == NULL)
			return -1;

		if (strncmp(domain_name, "sip:", 4) == 0)
			domain = osip_strdup(domain_name);
		else
			domain = g_strdup_printf("sip:%s", domain_name);

		if (osip_uri_parse(url, domain) != 0)
		{
			osip_free(domain);
			osip_uri_free(url);
			g_warning("invalid domain = %s\n", domain_name);
			return -1;
		}
		
		// update request uri
		rtk_fix_uri(sip->req_uri, url->host, url->port);
		// update from uri
		rtk_fix_uri(sip->from->url, url->host, url->port);
		// update to uri
		rtk_fix_uri(sip->to->url, url->host, url->port);

		osip_uri_free(url);
		osip_free(domain);
	}
	else
	{
		// restore request uri
		rtk_fix_uri(sip->req_uri, orig_host, orig_port);
		// restore from uri
		rtk_fix_uri(sip->from->url, orig_host, orig_port);
		// restore to uri
		rtk_fix_uri(sip->to->url, orig_host, orig_port);
		osip_free(orig_host);
		osip_free(orig_port);
	}

	return 0;
}

int rtk_get_local_ip(struct addrinfo *dest, char **loc)
{
	int err, tmp, sock;
	struct sockaddr_storage addr;
	socklen_t s;
	
	*loc = osip_malloc(MAXHOSTNAMELEN);
	if (eXosip.ip_family == AF_INET)
		strcpy(*loc, "127.0.0.1");  /* always fallback to local loopback */
	else 
		strcpy(*loc, "::1"); 

	sock = socket(dest->ai_family, SOCK_DGRAM, 0);

	tmp = 1;
	err = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(int));
	if (err < 0)
	{
		eXosip_trace(OSIP_ERROR, ("Error in setsockopt: %s\n", strerror(errno)));
		goto rtk_get_local_ip_done;
	}

	err = connect(sock, dest->ai_addr, dest->ai_addrlen);
	if (err < 0) 
	{
		eXosip_trace(OSIP_ERROR, ("Error in connect: %s\n", strerror(errno)));
		goto rtk_get_local_ip_done;
	}

	s = sizeof(addr);
	err = getsockname(sock, (struct sockaddr*) &addr, &s);
	if (err != 0) 
	{
		eXosip_trace(OSIP_ERROR, ("Error in getsockname: %s\n", strerror(errno)));
		goto rtk_get_local_ip_done;
	}
	
	err = getnameinfo((struct sockaddr *) &addr, s, *loc, MAXHOSTNAMELEN, NULL, 0, NI_NUMERICHOST);
	if (err != 0)
	{
		eXosip_trace(OSIP_ERROR, ("getnameinfo error:%s", strerror(errno)));
		goto rtk_get_local_ip_done;
	}

rtk_get_local_ip_done:
	close(sock);
	return err;
}

int cb_udp_snd_message(osip_transaction_t *tr, osip_message_t *sip, char *host,
		       int port, int out_socket)
{
  int len = 0;
  size_t length = 0;
  static int num = 0;
  char *message;
  int i, res;
  int nPort;
  struct addrinfo *addrinfo;
#if 0
  struct addrinfo *addrinfo2;
#endif
  char dest_ip[INET_ADDRSTRLEN];
  char local_ip[INET_ADDRSTRLEN];
  char local_ip2[INET_ADDRSTRLEN];
  char *local_ip_tmp = NULL;
  char *sip_port = NULL;
  char *media_port = NULL;
  struct __eXosip_sockaddr addr;
#if 0
  int bUseStun, bFirstInvite;
#else
  int bUseStun;
  osip_route_t *route;
#endif
  voipCfgPortParam_t *voip_ptr;
  osip_generic_param_t *to_tag;
  voipCfgProxy_t *proxy;
#ifdef CONFIG_RTK_VOIP_SIP_TLS
  int proxyIndex=0;
  int tlsProxy=0;
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

	if (out_socket==0) return -1;

	nPort = 0;
	for (i=0; i<g_MaxVoIPPorts; i++)
	{
#ifdef CONFIG_RTK_VOIP_SIP_TLS
		int bFound=0;
		if(eXosip_get_transport_type(sip) == TRANSPORT_TLS){
			for(proxyIndex=0; proxyIndex < MAX_PROXY; proxyIndex++){
				if(eXosip.j_tls_sockets[i][proxyIndex] == out_socket)
				{
					nPort =i;
					tlsProxy=proxyIndex;
					bFound=1;
					break;
				}
			}
		}else if(eXosip.j_sockets[i] == out_socket){
			nPort=i;
			break;
		}
		/*if out_socket is found in TLS go out*/
		if(bFound)
			break;
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
		if (eXosip.j_sockets[i] == out_socket)
		{
			nPort = i;
			break;
		}
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
	}

	voip_ptr = &g_pVoIPCfg->ports[nPort];

	if (host==NULL)
	{
		host = sip->req_uri->host;
		if (sip->req_uri->port!=NULL)
			port = osip_atoi(sip->req_uri->port);
		else
			port = 5060;
    }

#if 0

	// rock: check first invite 
	bFirstInvite = 
		( (MSG_IS_INVITE(sip) || MSG_IS_CANCEL(sip)) 
			&& (osip_to_get_tag(sip->to, &to_tag) != 0)) ||
		( MSG_IS_ACK(sip) && tr && 
			tr->last_response->status_code >= 300 && 
			tr->last_response->status_code <= 699 && 
			osip_to_get_tag(tr->orig_request->to, &to_tag) != 0 );

	// check outgoing proxy if register or first invite
	// TODO: NICT without dialog
	proxy = NULL;
	if (MSG_IS_REGISTER(sip) || bFirstInvite)
	{
		for (i=0; i<MAX_PROXY; i++)
			if ((voip_ptr->proxies[i].enable & PROXY_ENABLED) &&
				(strcmp(voip_ptr->proxies[i].addr, host) == 0) &&
				(voip_ptr->proxies[i].port == port))
				break;	// found 

		if (i < MAX_PROXY)
			proxy = &voip_ptr->proxies[i];
	}

#else

	// use outbound proxy if no route in SIP request
	proxy = NULL;
	osip_message_get_route(sip, 0, &route);
	if (MSG_IS_REQUEST(sip) && route == NULL)
	{
		for (i=0; i<MAX_PROXY; i++)
			if ((voip_ptr->proxies[i].enable & PROXY_ENABLED) &&
				(strcmp(voip_ptr->proxies[i].addr, host) == 0) &&
				(voip_ptr->proxies[i].port == port))
				break;	// found

		if (i < MAX_PROXY)
			proxy = &voip_ptr->proxies[i];
	}

#endif

	if (proxy && proxy->outbound_enable)
	{
		i = eXosip_get_addrinfo(
				&addrinfo, 
				proxy->outbound_addr,
				proxy->outbound_port
			);

		if (i!=0)
		#ifdef RTK_DNS
			return i;	// dns failed or retry
		#else
			return -1;	// dns failed only
		#endif

		i = rtk_get_local_ip(addrinfo, &local_ip_tmp);
		if (i != 0)
			return -1;

		strncpy(local_ip, local_ip_tmp, sizeof(local_ip));
		local_ip[sizeof(local_ip) - 1] = 0;
		osip_free(local_ip_tmp);

		memcpy(&addr, addrinfo->ai_addr, addrinfo->ai_addrlen);
		len = addrinfo->ai_addrlen;
		freeaddrinfo(addrinfo);

	#if 0
		// don't use outbound if host and outbound in different interface
		i = eXosip_get_addrinfo(&addrinfo2, host, port);
		#ifdef RTK_DNS
		if (i == 1)
			return 1;	// dns failed or retry
		#endif

		if (i == 0)
		{
			i = rtk_get_local_ip(addrinfo2, &local_ip_tmp);
			if (i != 0)
				return -1;

			strncpy(local_ip2, local_ip_tmp, sizeof(local_ip2));
			local_ip2[sizeof(local_ip2) - 1] = 0;
			osip_free(local_ip_tmp);

			// update local ip and dest if don't use outbound
			if (strcmp(local_ip, local_ip2) != 0)
			{
				strcpy(local_ip, local_ip2);
				memcpy(&addr, addrinfo2->ai_addr, addrinfo2->ai_addrlen);
				len = addrinfo2->ai_addrlen;
			}
		
			freeaddrinfo(addrinfo2);
		}
	#endif
	}
	else
	{
		i = eXosip_get_addrinfo(&addrinfo, host, port);
		if (i!=0)
		#ifdef RTK_DNS
			return i;	// dns failed or retry
		#else
			return -1;	// dns failed only
		#endif

		i = rtk_get_local_ip(addrinfo, &local_ip_tmp);
		if (i != 0)
			return -1;

		strncpy(local_ip, local_ip_tmp, sizeof(local_ip));
		local_ip[sizeof(local_ip) - 1] = 0;
		osip_free(local_ip_tmp);

		memcpy(&addr, addrinfo->ai_addr, addrinfo->ai_addrlen);
		len = addrinfo->ai_addrlen;
		freeaddrinfo(addrinfo);
	}

	if (inet_ntop(AF_INET, &((struct sockaddr_in *) &addr)->sin_addr, 
		dest_ip, INET_ADDRSTRLEN) == NULL)
		return -1;

	// check STUN and SIP PING
	bUseStun = 0;
	if (voip_ptr->stun_enable && 
		voip_ptr->stun_addr[0] != '\0' && 
		eXosip_is_public_address(dest_ip))
	{
		bUseStun = 1;
#ifdef NEW_STUN
		if (eXosip.j_firewall_ip[nPort][0] == 0)
		{
			rtk_stun_send(nPort, -1, STUN_UPDATE_SIP);
			return 1; // need stun retry
		}

		if (rtk_stun_check(nPort, -1, STUN_UPDATE_SIP) != 0)
		{
			rtk_stun_send(nPort, -1, STUN_UPDATE_SIP);
			return 1; // need stun retry
		}
#endif
		strcpy(local_ip2, local_ip);	// backup orig local ip if STUN used
		strcpy(local_ip, eXosip.j_firewall_ip[nPort]);
		sip_port = eXosip.j_firewall_ports[nPort];
	}
#ifdef SIP_PING
	// SIP PING rule by Rock! 
	// 1. Only proxy0 can be Nortel Softswitch (use SIP PING)
	// 2. If the proxy0 is Nortel Softswitch, then disble all other proxy, disable STUN,
	//	  and ALL SIP packets use pinged address (like STUN, because we couldn't check 
	//    incoming call comes from ...)
	// TODO: modify the rule if we get more information of SIP PING
	else if (
		(!MSG_IS_PING(sip)) && 
		(voip_ptr->proxies[0].enable & PROXY_ENABLED) &&
		(voip_ptr->proxies[0].enable & PROXY_NORTEL) &&
		(eXosip_is_public_address(dest_ip)))
	{
		if (rtk_ping_check(nPort) != 0)
		{
			rtk_ping_send(nPort);
			return 1; // need ping retry
		}

		strcpy(local_ip2, local_ip);	// backup orig local ip if SIP PING used
		strcpy(local_ip, eXosip.pinged_ip[nPort]);
		sip_port = eXosip.pinged_ports[nPort];
	}
#endif
	else
	{
		local_ip2[0] = 0; // orig local ip not changed
		sip_port = eXosip.localports[nPort];
	}

#ifdef RTK_DNS
	// check dns is done? if not then retry later
	res = 0;
	if (tr == NULL)
	{
		// "2xx retry" and "ack for 2xx" both are no transaction defined,
		// using ixt (ack or 200ok) or send_ack_directly to implement

	// don't care if ixt
	#if 0
		res = rtk_update_via(nPort, sip, local_ip, sip_port);
		if (res == 0)
			res = rtk_update_contact(nPort, sip, local_ip, sip_port);
		if (res == 0)
			res = rtk_update_sdp(nPort, sip, local_ip, media_port, T38_port, bUseStun);
	#endif
	}
	else switch (tr->ctx_type)
	{
	case ICT:
		if (tr->state == ICT_PRE_CALLING)
		{
			res = rtk_update_via(nPort, sip, local_ip, sip_port);
			if (res == 0)
				res = rtk_update_contact(nPort, sip, local_ip, sip_port);
			if (res == 0)
				res = rtk_get_media_port(nPort, tr, sip, &media_port, bUseStun);
			if (res == 0)
				res = rtk_update_sdp(nPort, tr, sip, local_ip, media_port, local_ip2);
		}
		break;
	case NICT:
		if ((tr->state == NICT_PRE_TRYING))
		{
			if (MSG_IS_CANCEL(sip))
			{
				// use the same via in orig_request, do nothing
			}
			else
			{
				/*
				MSG_IS_REGISTER(sip)
				MSG_IS_BYE(sip)
				MSG_IS_OPTIONS(sip)
				MSG_IS_INFO(sip)
				MSG_IS_REFER(sip)
				MSG_IS_NOTIFY(sip)
				MSG_IS_SUBSCRIBE(sip)
				MSG_IS_MESSAGE(sip)
				MSG_IS_PUBLISH(sip)
				MSG_IS_UPDATE(sip)
				MSG_IS_PRACK(sip)
				*/
				res = rtk_update_via(nPort, sip, local_ip, sip_port);

#ifdef SIP_PING
				if (MSG_IS_PING(sip))
				{
					osip_message_set_proxy_require(sip, "com.nortelnetworks.firewall");
				}
				else
#endif
				if (res == 0)
				{
					res = rtk_update_contact(nPort, sip, local_ip, sip_port);
				}
			}
		}
		break;
	case IST:
		if (tr->state == IST_PROCEEDING)
		{
			if ((MSG_IS_STATUS_1XX(sip) && sip->status_code > 100) ||
				MSG_IS_STATUS_2XX(sip))
			{
				res = rtk_update_contact(nPort, sip, local_ip, sip_port);
				if (res == 0 && (sip->status_code == 183 || MSG_IS_STATUS_2XX(sip)))
				{
					res = rtk_get_media_port(nPort, tr, sip, &media_port, bUseStun);
					if (res == 0)
						res = rtk_update_sdp(nPort, tr, sip, local_ip, media_port, local_ip2);
				}
			}
			else
			{
				// 3456X
				// contact, sdp?
			}
		}
		break;
	case NIST:
		if (tr->state == NIST_TRYING || tr->state == NIST_PROCEEDING)
		{
			if (MSG_IS_REFER(sip) ||
				MSG_IS_NOTIFY(sip) ||
				MSG_IS_SUBSCRIBE(sip))
			{
				if ((MSG_IS_STATUS_1XX(sip) && sip->status_code > 100) ||
					MSG_IS_STATUS_2XX(sip))
				{
					res = rtk_update_contact(nPort, sip, local_ip, sip_port);
				}
			}
			else 
			{
				/*
				MSG_IS_REGISTER(sip)
				MSG_IS_BYE(sip)
				MSG_IS_OPTIONS(sip)
				MSG_IS_INFO(sip)
				MSG_IS_CANCEL(sip)
				MSG_IS_MESSAGE(sip)
				MSG_IS_PRACK(sip)
				MSG_IS_UPDATE(sip)
				MSG_IS_PUBLISH(sip)
				*/
			}
		}
		break;
	}

	if (res != 0)
	{
//		printf("cb_udp_snd_message: err=%d, check state machine failed\n", res);
		return res; // 1 (retry), or -1 (failed)
	}
#endif

	// fix domain if register or first invite
	if (proxy && proxy->domain_name[0])
	{
		rtk_fix_domain(sip, proxy->domain_name, 1);
	}

  i = osip_message_to_str(sip, &message, &length);

	// restore sip for retry
	if (proxy && proxy->domain_name[0])
	{
		rtk_fix_domain(sip, proxy->domain_name, 0);
	}

  if (i!=0 || length<=0) {
    return -1;
  }

  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,
			"Message sent: \n%s (len=%i sizeof(addr)=%i %i)\n",
			message, len, sizeof(addr), sizeof(struct sockaddr_in6)));

#ifdef NEW_STUN
	if (MSG_IS_REGISTER(sip))
	{
		rtk_stun_keep_alive(nPort, &addr);
	}
#endif

if(101 != sip->status_code){
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	int ret=-1;

	if(eXosip_get_transport_type(sip) == TRANSPORT_TLS){
		if(0 > (ret=tls_send_message(sipTLSObj[nPort][tlsProxy].ssl_conn,  message, length)))
			g_warning("tls send error: %s\n", strerror(errno));
	}else{
		if (0  > (ret=sendto (out_socket, (const void*) message, length, 0,
		   (struct sockaddr *) ((void *) &addr), len /* sizeof(addr) */ )))	
			g_warning("out_socket:send by udp: %s\n", strerror(errno));
	}
	if(ret < 0)
	{
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
  if (0  > sendto (out_socket, (const void*) message, length, 0,
		   (struct sockaddr *) ((void *) &addr), len /* sizeof(addr) */ )) 
    {
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
	if (ECONNREFUSED==errno)
	  {
	    /* This can be considered as an error, but for the moment,
	       I prefer that the application continue to try sending
	       message again and again... so we are not in a error case.
	       Nevertheless, this error should be announced!
	       ALSO, UAS may not have any other options than retry always
	       on the same port.
	    */
	    osip_free(message);
	    return 1;
	  }
	else
	  {
	    /* SIP_NETWORK_ERROR; */
	    osip_free(message);
	    return -1;
	  }
    }
}

#ifdef SIP_PING_DEBUG
	if (MSG_IS_PING(sip))
	{
		printf("ping %s\n", dest_ip);
	}
#endif

#if 0
	// send invite twice for testing
	if (tr && tr->ctx_type == ICT && tr->state == ICT_PRE_CALLING)
		sendto (out_socket, (const void*) message, length, 0,
           (struct sockaddr *) ((void *) &addr), len /* sizeof(addr) */ );
#endif

  if (strncmp(message, "INVITE", 7)==0)
    {
      num++;
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO4,NULL,"number of message sent: %i\n", num));
    }

  osip_free(message);
  return 0;
  
}

static void cb_ict_kill_transaction(int type, osip_transaction_t *tr)
{
#ifdef FREE_TRANSACTION_LATER
  // remove transaction later
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_ict_kill_transaction (id=%i)\r\n", tr->transactionid));
#else
  int i;
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_ict_kill_transaction (id=%i)\r\n", tr->transactionid));

  i = osip_remove_transaction(eXosip.j_osip, tr);
  if (i!=0)
    {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_BUG,NULL,"cb_ict_kill_transaction Error: Could not remove transaction from the oSIP stack? (id=%i)\r\n", tr->transactionid));
    }
#endif
}

static void cb_ist_kill_transaction(int type, osip_transaction_t *tr)
{
#ifdef FREE_TRANSACTION_LATER
  // remove transaction later
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_ist_kill_transaction (id=%i)\r\n", tr->transactionid));
#else
  int i;
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_ist_kill_transaction (id=%i)\r\n", tr->transactionid));
  i = osip_remove_transaction(eXosip.j_osip, tr);
  if (i!=0)
    {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_BUG,NULL,"cb_ist_kill_transaction Error: Could not remove transaction from the oSIP stack? (id=%i)\r\n", tr->transactionid));
    }
#endif
}

static void cb_nict_kill_transaction(int type, osip_transaction_t *tr)
{
  eXosip_dialog_t    *jd;
  eXosip_call_t      *jc;
  eXosip_subscribe_t *js;
  eXosip_notify_t    *jn;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);

#ifdef FREE_TRANSACTION_LATER
  // remove transaction later
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_nict_kill_transaction (id=%i)\r\n", tr->transactionid));
#else
  int i;

  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_nict_kill_transaction (id=%i)\r\n", tr->transactionid));
  i = osip_remove_transaction(eXosip.j_osip, tr);
  g_message("+++++DEBUG:cb_nict_kill_transaction:i%d:method:%s+++++\n",i,tr->orig_request->sip_method);
  if (i!=0)
    {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_BUG,NULL,"cb_nict_kill_transaction Error: Could not remove transaction from the oSIP stack? (id=%i)\r\n", tr->transactionid));
    }
#endif

  if (MSG_IS_REGISTER(tr->orig_request)
      && type==OSIP_NICT_KILL_TRANSACTION
      && tr->last_response==NULL)
    {
      eXosip_event_t *je;
      eXosip_reg_t *jreg=NULL;
      /* find matching j_reg */
_eXosip_reg_find( &jreg , tr ) ;
      if (jreg!=NULL)
	{
	  je = eXosip_event_init_for_reg(EXOSIP_REGISTRATION_FAILURE, jreg);
	  report_event_with_status(je, NULL);
	}
      return;
    }

  if (jinfo==NULL)
    return;
  
  jd = jinfo->jd;
  jc = jinfo->jc;
  jn = jinfo->jn;
  js = jinfo->js;

#ifdef SIP_PING
  if (MSG_IS_PING(tr->orig_request))
  {
      eXosip_event_t *je;

	  eXosip_event_init(jinfo->nPort, &je, EXOSIP_PING_CLOSED);
	  if (je)
	  	report_event_with_status(je, NULL);
  }
#endif

	/* ++added by Jack Chan  29/01/07 for VoIP
	freed the BYE dialog for no response++*/
	/*if(MSG_IS_BYE(tr->orig_request)){
		g_message("+++++DEBUG:cb_nict_kill_transaction:BYE+++++\n");
		if( NULL != jd ){
			osip_dialog_free(jd->d_dialog);
			jd->d_dialog=NULL;
		}
		return;
	}*/
	/*--end--*/
	
  if (MSG_IS_BYE(tr->orig_request))
  {
	  if (jd)
	  {
		  report_call_event_with_status(EXOSIP_CALL_EXIT, jc, jd, NULL);
		  jd->d_STATE = JD_TERMINATED;
	  }
  }

  if (jn==NULL && js==NULL)
    return;

  /* no answer to a NOTIFY request! */
  if (MSG_IS_NOTIFY(tr->orig_request)
      && type==OSIP_NICT_KILL_TRANSACTION
      && tr->last_response==NULL)
    {
      /* delete the dialog! */
      REMOVE_ELEMENT(eXosip.j_notifies, jn);
      eXosip_notify_free(jn);
      return;
    }

  if (MSG_IS_NOTIFY(tr->orig_request)
      && type==OSIP_NICT_KILL_TRANSACTION
      && tr->last_response!=NULL
      && tr->last_response->status_code > 299)
    {
      /* delete the dialog! */
      REMOVE_ELEMENT(eXosip.j_notifies, jn);
      eXosip_notify_free(jn);
      return;
    }

  if (MSG_IS_NOTIFY(tr->orig_request)
      && type==OSIP_NICT_KILL_TRANSACTION
      && tr->last_response!=NULL
      && tr->last_response->status_code > 199
      && tr->last_response->status_code < 300)
    {
      if (jn->n_ss_status==EXOSIP_SUBCRSTATE_TERMINATED)
	{
	  /* delete the dialog! */
	  REMOVE_ELEMENT(eXosip.j_notifies, jn);
	  eXosip_notify_free(jn);
	  return;
	}
    }

  /* no answer to a SUBSCRIBE request! */
  if (MSG_IS_SUBSCRIBE(tr->orig_request)
      && type==OSIP_NICT_KILL_TRANSACTION
      && tr->last_response==NULL)
    {
      /* delete the dialog! */
      REMOVE_ELEMENT(eXosip.j_subscribes, js);
      eXosip_subscribe_free(js);
      return;
    }

  /* detect SUBSCRIBE request that close the dialogs! */
  /* expires=0 with MSN */
  if (MSG_IS_SUBSCRIBE(tr->orig_request)
      && type==OSIP_NICT_KILL_TRANSACTION)
    {
      osip_header_t *expires;
      osip_message_get_expires(tr->orig_request, 0, &expires);
      if (expires==NULL || expires->hvalue==NULL)
	{
	}
      else if (0==strcmp(expires->hvalue, "0"))
	{
	  /* delete the dialog! */
	  REMOVE_ELEMENT(eXosip.j_subscribes, js);
	  eXosip_subscribe_free(js);
	  return;
	}
    }
}

static void cb_nist_kill_transaction(int type, osip_transaction_t *tr)
{
#ifdef FREE_TRANSACTION_LATER
  // remove transaction later
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_nist_kill_transaction (id=%i)\r\n", tr->transactionid));
#else
  int i;
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_nist_kill_transaction (id=%i)\r\n", tr->transactionid));
  i = osip_remove_transaction(eXosip.j_osip, tr);
  if (i!=0)
    {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_BUG,NULL,"cb_nist_kill_transaction Error: Could not remove transaction from the oSIP stack? (id=%i)\r\n", tr->transactionid));
    }
#endif

}
  
static void cb_rcvinvite  (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvinvite (id=%i)\n", tr->transactionid));
}

static void cb_rcvack     (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvack (id=%i)\n", tr->transactionid));
}

static void cb_rcvack2    (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvack2 (id=%i)\r\n", tr->transactionid));
}
  
static void cb_rcvregister(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvregister (id=%i)\r\n", tr->transactionid));
}

static void cb_rcvbye     (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvbye (id=%i)\r\n", tr->transactionid));
}

static void cb_rcvcancel  (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvcancel (id=%i)\r\n", tr->transactionid));
}

static void cb_rcvinfo    (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_event_t     *je;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvinfo (id=%i)\r\n", tr->transactionid));
  
  if (jinfo==NULL)
    return;
  if (jinfo->jc==NULL)
    return;

  je = eXosip_event_init_for_call(EXOSIP_INFO_NEW, jinfo->jc, jinfo->jd);
  if (je!=NULL)
    {
      char *tmp;
      osip_uri_to_str(sip->req_uri, &tmp);
      if (tmp!=NULL)
 	{
 	  snprintf(je->req_uri, 255, "%s", tmp);
 	  osip_free(tmp);
 	}
      
      if (sip!=NULL)
	{
	  int pos;
	  /* get content-type info */
	  osip_content_type_clone(osip_message_get_content_type(sip), &(je->i_ctt));
	  /* get list of bodies */
	  je->i_bodies = (osip_list_t*) osip_malloc(sizeof(osip_list_t));
	  osip_list_init(je->i_bodies);
	  for (pos=0;!osip_list_eol(sip->bodies, pos);pos++)
	    {
	      osip_body_t *body;
	      osip_body_t *_body;
	      body = (osip_body_t *)osip_list_get(sip->bodies, pos);
	      osip_body_clone(body, &_body);
	      osip_list_add(je->i_bodies, _body, -1);
	    }
	}
    }

  report_event_with_status(je, NULL);
}

static void cb_rcvoptions (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_event_t     *je;
  eXosip_dialog_t    *jd;
  eXosip_call_t      *jc;
  eXosip_subscribe_t *js;
  eXosip_notify_t    *jn;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);

  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvoptions (id=%i)\r\n", tr->transactionid));

  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  jn = jinfo->jn;
  js = jinfo->js;
  if (jinfo->jc==NULL)
    return;
  
  je = eXosip_event_init_for_call(EXOSIP_OPTIONS_NEW, jc, jd);
  if (je!=NULL)
    {
      char *tmp;
      osip_uri_to_str(sip->req_uri, &tmp);
      if (tmp!=NULL)
	{
	  snprintf(je->req_uri, 255, "%s", tmp);
	  osip_free(tmp);
	}
    }
  report_event_with_status(je, NULL);

}

static void cb_rcvnotify  (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvnotify (id=%i)\r\n", tr->transactionid));
}

static void cb_rcvsubscribe (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_event_t     *je;
  eXosip_dialog_t    *jd;
  eXosip_notify_t    *jn;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);

  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvsubscribe (id=%i)\r\n", tr->transactionid));

  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jn = jinfo->jn;
  if (jinfo->jn==NULL)
    return;
  
  je = eXosip_event_init_for_notify(EXOSIP_IN_SUBSCRIPTION_NEW, jn, jd);
  if (je!=NULL)
    {
      char *tmp;
      osip_uri_to_str(sip->req_uri, &tmp);
      if (tmp!=NULL)
	{
	  snprintf(je->req_uri, 255, "%s", tmp);
	  osip_free(tmp);
	}
    }
  report_event_with_status(je, NULL);
}

static void cb_rcvunkrequest(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t    *jd;
  eXosip_call_t      *jc;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);

  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvunkrequest (id=%i)\r\n", tr->transactionid));

  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  if (jinfo->jc==NULL)
    return;

  
  if (MSG_IS_REFER(sip))
    {
      eXosip_event_t *je;

      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvrefer (id=%i)\r\n", tr->transactionid));

      je = eXosip_event_init_for_call(EXOSIP_CALL_REFERED, jc, jd);
      if (je!=NULL)
	{
	  report_event_with_status(je, NULL);
	}
    }

}

static void cb_sndinvite  (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndinvite (id=%i)\r\n", tr->transactionid));
}

static void cb_sndack     (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndack (id=%i)\r\n", tr->transactionid));
}
  
static void cb_sndregister(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndregister (id=%i)\r\n", tr->transactionid));
}

static void cb_sndbye     (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndbye (id=%i)\r\n", tr->transactionid));
}

static void cb_sndcancel  (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndcancel (id=%i)\r\n", tr->transactionid));
}

static void cb_sndinfo    (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndinfo (id=%i)\r\n", tr->transactionid));
}

static void cb_sndoptions (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndoptions (id=%i)\r\n", tr->transactionid));
}

static void cb_sndnotify  (int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndnotify (id=%i)\r\n", tr->transactionid));
}

static void cb_sndsubscribe(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndsubscibe (id=%i)\r\n", tr->transactionid));
}

static void cb_sndunkrequest(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndunkrequest (id=%i)\r\n", tr->transactionid));
}

void __eXosip_delete_jinfo(osip_transaction_t *transaction)
{
  jinfo_t *ji;
  if (transaction==NULL)
    return;
  ji = osip_transaction_get_your_instance(transaction);
  osip_free(ji);
  osip_transaction_set_your_instance(transaction, NULL);
}

#ifdef MALLOC_DEBUG
jinfo_t *____eXosip_new_jinfo(int nPort, eXosip_call_t *jc, eXosip_dialog_t *jd,
			    eXosip_subscribe_t *js, eXosip_notify_t *jn, char *file, int line)
#else
jinfo_t *__eXosip_new_jinfo(int nPort, eXosip_call_t *jc, eXosip_dialog_t *jd,
			    eXosip_subscribe_t *js, eXosip_notify_t *jn)
#endif
{
#ifdef MALLOC_DEBUG
  jinfo_t *ji = (jinfo_t *) __osip_malloc(sizeof(jinfo_t), file, line);
#else
  jinfo_t *ji = (jinfo_t *) osip_malloc(sizeof(jinfo_t));
#endif
  if (ji==NULL) return NULL;
  ji->nPort = nPort;
  ji->jd = jd;
  ji->jc = jc;
  if (jc && jc->nPort != nPort)
	printf("Warning: jc->nPort != nPort\n");
  ji->js = js;
  if (js && js->nPort != nPort)
	printf("Warning: js->nPort != nPort\n");
  ji->jn = jn;
  if (jn && jn->nPort != nPort)
	printf("Warning: jn->nPort != nPort\n");
  return ji;
}

static void cb_rcv1xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t    *jd;
  eXosip_call_t      *jc;
  eXosip_subscribe_t *js;
  eXosip_notify_t    *jn;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv1xx (id=%i)\r\n", tr->transactionid));
  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  jn = jinfo->jn;
  js = jinfo->js;

  if (MSG_IS_RESPONSE_FOR(sip, "OPTIONS"))
    {
      if (jc==NULL)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv1xx (id=%i) Error: no call or transaction info for OPTIONS transaction\r\n", tr->transactionid));
	  return;
	}
      else if (jc->c_out_options_tr==NULL)
	{
	  /* options is within a call */
	}
      report_call_event_with_status(EXOSIP_OPTIONS_PROCEEDING, jc, jd, sip);
      return;
    }

  if ((MSG_IS_RESPONSE_FOR(sip, "INVITE")
       || MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
      && !MSG_TEST_CODE(sip, 100))
    {
      int i;
      /* for SUBSCRIBE, test if the dialog has been already created
	 with a previous NOTIFY */
      if (jd==NULL && js!=NULL && js->s_dialogs!=NULL && MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
	{
	  /* find if existing dialog match the to tag */
	  osip_generic_param_t *tag;
	  int i;
	  i = osip_to_get_tag (sip->to, &tag);
	  if (i==0 && tag!=NULL && tag->gvalue!=NULL )
	    {
	      for (jd = js->s_dialogs; jd!= NULL ; jd=jd->next)
		{
		  if (0==strcmp(jd->d_dialog->remote_tag, tag->gvalue))
		    {
		      OSIP_TRACE (osip_trace
				  (__FILE__, __LINE__, OSIP_INFO2, NULL,
				   "eXosip: found established early dialog for this subscribe\n"));
		      jinfo->jd = jd;
		      break;
		    }
		}
	    }
	}

      if (jd == NULL) /* This transaction initiate a dialog in the case of
			 INVITE (else it would be attached to a "jd" element. */
	{
	  /* allocate a jd */

	  i = eXosip_dialog_init_as_uac(&jd, sip);
	  if (i!=0)
	    {
	      OSIP_TRACE (osip_trace
		     (__FILE__, __LINE__, OSIP_ERROR, NULL,
	         "eXosip: cannot establish a dialog\n"));
	      return;
	    }
	  if (jc!=NULL)
	    {
	      ADD_ELEMENT(jc->c_dialogs, jd);
	      jinfo->jd = jd;
	      eXosip_update();
	    }
	  else if (js!=NULL)
	    {
	      ADD_ELEMENT(js->s_dialogs, jd);
	      jinfo->jd = jd;
	      eXosip_update();
	    }
	  else if (jn!=NULL)
	    {
	      ADD_ELEMENT(jn->n_dialogs, jd);
	      jinfo->jd = jd;
	      eXosip_update();
	    }
	  else
	    {
#ifndef WIN32
	      assert(0==0);
#else
eeeeeeeeeeeeeeeee
		  exit(0);
#endif
	    }
	  osip_transaction_set_your_instance(tr, jinfo);
	}
      else
	{
	  osip_dialog_update_route_set_as_uac(jd->d_dialog, sip);
	}

      if ( jd!=NULL)
	jd->d_STATE = JD_TRYING;
      if ( jd!=NULL && MSG_IS_RESPONSE_FOR(sip, "INVITE")
	   && sip->status_code < 180)
	{
	  eXosip_event_t *je;
	  je = eXosip_event_init_for_call(EXOSIP_CALL_PROCEEDING, jc, jd);
	  if (je!=NULL)
	    {
	      if (sip->status_code>100)
		eXosip_event_add_sdp_info(je, sip);
	      report_event_with_status(je, sip);
	    }
	}
      else if ( jd!=NULL && MSG_IS_RESPONSE_FOR(sip, "INVITE")
		&& sip->status_code >= 180)
	{
	  eXosip_event_t *je;
	  je = eXosip_event_init_for_call(EXOSIP_CALL_RINGING, jc, jd);
	  if (je!=NULL)
	    {
	      eXosip_event_add_sdp_info(je, sip);
	      report_event_with_status(je, sip);
	    }
	}
      else if ( jd!=NULL && MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
	{
	  eXosip_event_t *je;
	  je = eXosip_event_init_for_subscribe(EXOSIP_SUBSCRIPTION_PROCEEDING, js, jd);
	  if (je!=NULL)
	    report_event_with_status(je, sip);
	}
      if (MSG_TEST_CODE(sip, 180) && jd!=NULL)
	{
	  jd->d_STATE = JD_RINGING;
	}
      else if (MSG_TEST_CODE(sip, 183) && jd!=NULL)
	{
	  jd->d_STATE = JD_QUEUED;
	}

    }
}

sdp_message_t *eXosip_get_remote_sdp(osip_transaction_t *transaction)
{
  osip_message_t *message;
  osip_body_t *body;
  sdp_message_t *sdp;
  int pos = 0;
  int i;
  if (transaction->ist_context!=NULL)
    /* remote sdp is in INVITE (or ACK!) */
    message = transaction->orig_request;
  else
    /* remote sdp is in response */
    message = transaction->last_response;

  if (message==NULL)
    {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"No remote sdp body found\r\n"));
      return NULL;
    }
  sdp=NULL;
  body = (osip_body_t *)osip_list_get(message->bodies,0);
  while (body!=NULL)
    {
      i = sdp_message_init(&sdp);
      if (i!=0)
	{ sdp = NULL; break; }
      i = sdp_message_parse(sdp,body->body);
      if (i==0)
	return sdp;
      sdp_message_free(sdp);
      sdp = NULL;
      pos++;
      body = (osip_body_t *)osip_list_get(message->bodies,pos);
    }
  return NULL;
}

sdp_message_t *eXosip_get_local_sdp(osip_transaction_t *transaction)
{
  osip_message_t *message;
  osip_body_t *body;
  sdp_message_t *sdp;
  int i;
  int pos = 0;
  if (transaction->ict_context!=NULL)
    /* local sdp is in INVITE (or ACK!) */
    message = transaction->orig_request;
  else
    /* local sdp is in response */
    message = transaction->last_response;

  sdp=NULL;
  body = (osip_body_t *)osip_list_get(message->bodies,0);
  while (body!=NULL)
    {
      i = sdp_message_init(&sdp);
      if (i!=0)
	{ sdp = NULL; break; }
      i = sdp_message_parse(sdp,body->body);
      if (i==0)
	return sdp;
      sdp_message_free(sdp);
      sdp = NULL;
      pos++;
      body = (osip_body_t *)osip_list_get(message->bodies,pos);
    }
  return NULL;
}

static
void report_call_event_with_status(int evt, eXosip_call_t *jc, eXosip_dialog_t *jd, osip_message_t *sip)
{
  eXosip_event_t *je;
  je = eXosip_event_init_for_call(evt, jc, jd);
  if (je!=NULL)
    {
      if (sip != NULL)
	eXosip_event_add_status(je, sip);
      if (eXosip.j_call_callbacks[evt]!=NULL)
#ifdef FIX_RESPONSE_TIME
	eXosip.j_call_callbacks[evt](eXosip.user, evt, je);
#else
	eXosip.j_call_callbacks[evt](evt, je);
#endif
      else if (eXosip.j_runtime_mode==EVENT_MODE)
	eXosip_event_add(je);
    }

}

static
void report_event_with_status(eXosip_event_t *je, osip_message_t *sip)
{
  if (je!=NULL)
    {
      int evt = je->type;

      if (sip != NULL)
	eXosip_event_add_status(je, sip);
      if (eXosip.j_call_callbacks[evt]!=NULL)
#ifdef FIX_RESPONSE_TIME
	eXosip.j_call_callbacks[evt](eXosip.user, evt, je);
#else
	eXosip.j_call_callbacks[evt](evt, je);
#endif
      else if (eXosip.j_runtime_mode==EVENT_MODE)
	eXosip_event_add(je);
    }
}

osip_message_t *build_ack_for_receiving_2xx_response(eXosip_call_t *jc,
	eXosip_dialog_t *jd, osip_message_t *msg_2xx)
{
	int i;
	osip_message_t *ack;
#ifdef DYNAMIC_PAYLOAD
#else
	char *body;
	char *size;
#endif // DYNAMIC_PAYLOAD

#ifdef CONFIG_RTK_VOIP_SIP_TLS
	if(eXosip_get_transport_type(msg_2xx)==TRANSPORT_TLS)
		i = _eXosip_build_request_within_dialog(jc->nPort, &ack, "ACK", jd->d_dialog, "TLS");	
	else
    i = _eXosip_build_request_within_dialog(jc->nPort, &ack, "ACK", jd->d_dialog, "UDP");
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
    i = _eXosip_build_request_within_dialog(jc->nPort, &ack, "ACK", jd->d_dialog, "UDP");
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
	if (i != 0)
		goto build_ack_within_dialog_fail_0;

#ifdef DYNAMIC_PAYLOAD
	return ack;
#else
	if (jc->c_ack_sdp == 0)
		return ack;

	/* need to build sdp answer */
	body = generating_sdp_answer(jc->nPort, msg_2xx, jc->c_ctx);
	if (body == NULL)
	{
		goto build_ack_within_dialog_fail_1;
	}

	i = osip_message_set_body(ack, body, strlen(body));
	if (i!=0)
	{
		goto build_ack_within_dialog_fail_2;
	}
	
	size = (char *) osip_malloc(6 * sizeof(char));
	if (size == NULL)
	{
		goto build_ack_within_dialog_fail_3;
	}

	sprintf(size, "%i", strlen(body));
	i = osip_message_set_content_length(ack, size);
	if (i!=0)
	{
		goto build_ack_within_dialog_fail_4;
	}

	i = osip_message_set_content_type(ack, "application/sdp");
	if (i!=0)
	{
		goto build_ack_within_dialog_fail_5;
	}

	osip_free(size);
	osip_free(body);
	return ack;

build_ack_within_dialog_fail_5:
	// free content type in osip_message_free
build_ack_within_dialog_fail_4:
	osip_free(size);
build_ack_within_dialog_fail_3:
	// free body in osip_message_free
build_ack_within_dialog_fail_2:
	osip_free(body);
build_ack_within_dialog_fail_1:
	osip_message_free(ack);
#endif // DYNAMIC_PAYLOAD
build_ack_within_dialog_fail_0:
	return NULL;
}

int send_ack_directly(int nPort, osip_message_t *ack)
{
	osip_route_t *route;
	char *host;
	int port;

	osip_message_get_route(ack, 0, &route);

#ifdef RTK_FIX_ROUTE
	if (route != NULL && route->url != NULL)
    {
		osip_uri_param_t *lr_param;
		osip_uri_uparam_get_byname(route->url, "lr", &lr_param);
		if (lr_param == NULL) /* using uncompliant proxy: destination is the request-uri */
		{
			// rock: remove strict route in route set
			osip_list_remove(ack->routes, 0);
			osip_route_free(route);
			route = NULL;
		}
    }
#endif // RTK_FIX_ROUTE

	if (route != NULL)
	{
		port = 5060;
		if (route->url->port != NULL)
			port = osip_atoi(route->url->port);

		host = route->url->host;
	}
	else
	{
		port = 5060;
		if (ack->req_uri->port != NULL)
			port = osip_atoi(ack->req_uri->port);

		host = ack->req_uri->host;
	}

//    cb_udp_snd_message(NULL, ack, host, port, eXosip.j_socket);
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	if(eXosip_get_transport_type(ack)==TRANSPORT_TLS){
		osip_uri_t *uri=NULL;
		char *host;
		int proxyIndex;
	  
		uri=osip_from_get_url(ack->from);
		host=osip_uri_get_host(uri);
		proxyIndex=eXosip_find_TCP_socket_form_proxy(nPort, host);
		if(proxyIndex != -1)
			cb_udp_snd_message(NULL, ack, host, port, eXosip.j_tls_sockets[nPort][proxyIndex]);
	}else{
    cb_udp_snd_message(NULL, ack, host, port, eXosip.j_sockets[nPort]);
	}
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
    cb_udp_snd_message(NULL, ack, host, port, eXosip.j_sockets[nPort]);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

	return 0;
}

static void cb_rcv2xx_4invite(osip_transaction_t *tr,osip_message_t *sip)
{
  int i;
  eXosip_dialog_t *jd;
  eXosip_call_t *jc;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  if (jd == NULL) /* This transaction initiate a dialog in the case of
		     INVITE (else it would be attached to a "jd" element. */
    {
      /* allocate a jd */
      i = eXosip_dialog_init_as_uac(&jd, sip);
      if (i!=0)
	{
     OSIP_TRACE (osip_trace
	    (__FILE__, __LINE__, OSIP_ERROR, NULL,
	     "eXosip: cannot establish a dialog\n"));
	  return;
	}
      ADD_ELEMENT(jc->c_dialogs, jd);
      jinfo->jd = jd;
      eXosip_update();
      osip_transaction_set_your_instance(tr, jinfo);
    }
  else
    {
      /* Here is a special case:
	 We have initiated a dialog and we have received informationnal
	 answers from 2 or more remote SIP UA. Those answer can be
	 differentiated with the "To" header's tag.

	 We have used the first informationnal answer to create a
	 dialog, but we now want to be sure the 200ok received is
	 for the dialog this dialog.
	 
	 We have to check the To tag and if it does not match, we
	 just have to modify the existing dialog and replace it. */
      osip_generic_param_t *tag;
      int i;
      i = osip_to_get_tag (sip->to, &tag);
      i=1; /* default is the same dialog */

      if (jd->d_dialog==NULL || jd->d_dialog->remote_tag==NULL)
	{
	  /* There are real use-case where a BYE is received/processed before
	     the 200ok of the previous INVITE. In this case, jd->d_dialog is
	     empty and the transaction should be silently discarded. */
	  /* a ACK should still be sent... -but there is no dialog built- */
	  return;
	}

      if (jd->d_dialog->remote_tag==NULL && tag==NULL)
	{  } /* non compliant remote UA -> assume it is the same dialog */
      else if (jd->d_dialog->remote_tag!=NULL && tag==NULL)
	{ i=0; } /* different dialog! */
      else if (jd->d_dialog->remote_tag==NULL && tag!=NULL)
	{ i=0; } /* different dialog! */
      else if (jd->d_dialog->remote_tag!=NULL && tag!=NULL && tag->gvalue!=NULL
	       && 0!=strcmp(jd->d_dialog->remote_tag, tag->gvalue))
	{ i=0; } /* different dialog! */
      
      if (i==1) /* just update the dialog */
	{
	  osip_dialog_update_route_set_as_uac(jd->d_dialog, sip);
	  osip_dialog_set_state(jd->d_dialog, DIALOG_CONFIRMED);
	}
      else
	{
	  /* the best thing is to update the repace the current dialog
	     information... Much easier than creating a useless dialog! */
	  osip_dialog_free(jd->d_dialog);
	  i = osip_dialog_init_as_uac(&(jd->d_dialog), sip);
	  if (i!=0)
	    {
	      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"Cannot replace the dialog.\r\n"));
	    }
	  else
	    {
	      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_WARNING,NULL,"The dialog has been replaced with the new one fro 200ok.\r\n"));
	    }
	}
    }

  jd->d_STATE = JD_ESTABLISHED;

  eXosip_dialog_set_200ok(jd, sip);

  {
    osip_message_t *ack;

	ack = build_ack_for_receiving_2xx_response(jc, jd, tr->last_response);
	if (ack == NULL)
	{
		// TODO: error handling
		jd->d_STATE = JD_ESTABLISHED;
		return ;
	}
	else
	{
		send_ack_directly(jc->nPort, ack);

		if (jd->d_ack) 
			osip_message_free(jd->d_ack);

		jd->d_ack  = ack;
	}
  }

  {
    eXosip_event_t *je;
    je = eXosip_event_init_for_call(EXOSIP_CALL_ANSWERED, jc, jd);
    if (je!=NULL)
      {
	eXosip_event_add_sdp_info(je, sip);
	report_event_with_status(je, sip);
      }


    je = eXosip_event_init_for_call(EXOSIP_CALL_STARTAUDIO, jc, jd);
    if (je!=NULL)
      {
	eXosip_event_add_sdp_info(je, sip);
	report_event_with_status(je, sip);
      }

  }

  /* look for the SDP information and decide if this answer was for
     an initial INVITE, an HoldCall, or a RetreiveCall */

  /* don't handle hold/unhold by now... */
  /* eXosip_update_audio_session(tr); */

}

static void cb_rcv2xx_4subscribe(osip_transaction_t *tr,osip_message_t *sip)
{
  int i;
  eXosip_dialog_t    *jd;
  eXosip_subscribe_t *js;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  js = jinfo->js;
  _eXosip_subscribe_set_refresh_interval(js, sip);


  /* for SUBSCRIBE, test if the dialog has been already created
     with a previous NOTIFY */
  if (jd==NULL && js!=NULL && js->s_dialogs!=NULL && MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
      /* find if existing dialog match the to tag */
      osip_generic_param_t *tag;
      int i;
      i = osip_to_get_tag (sip->to, &tag);
      if (i==0 && tag!=NULL && tag->gvalue!=NULL )
	{
	  for (jd = js->s_dialogs; jd!= NULL ; jd=jd->next)
	    {
	      if (0==strcmp(jd->d_dialog->remote_tag, tag->gvalue))
		{
		  OSIP_TRACE (osip_trace
			      (__FILE__, __LINE__, OSIP_INFO2, NULL,
			       "eXosip: found established early dialog for this subscribe\n"));
		  jinfo->jd = jd;
		  break;
		}
	    }
	}
    }

  if (jd == NULL) /* This transaction initiate a dialog in the case of
		     SUBSCRIBE (else it would be attached to a "jd" element. */
    {
      /* allocate a jd */
      i = eXosip_dialog_init_as_uac(&jd, sip);
      if (i!=0)
	{
     OSIP_TRACE (osip_trace
	    (__FILE__, __LINE__, OSIP_ERROR, NULL,
	     "eXosip: cannot establish a dialog\n"));
	  return;
	}
      ADD_ELEMENT(js->s_dialogs, jd);
      jinfo->jd = jd;
      eXosip_update();
      osip_transaction_set_your_instance(tr, jinfo);
    }
  else
    {
      osip_dialog_update_route_set_as_uac(jd->d_dialog, sip);
      osip_dialog_set_state(jd->d_dialog, DIALOG_CONFIRMED);
    }

  jd->d_STATE = JD_ESTABLISHED;
  /* look for the body information */

  {
    eXosip_event_t *je;
    je = eXosip_event_init_for_subscribe(EXOSIP_SUBSCRIPTION_ANSWERED, js, jd);
    if (je!=NULL)
      {
	report_event_with_status(je, sip);
      }
  }

}

static void cb_rcv2xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t    *jd;
  eXosip_call_t      *jc;
  eXosip_subscribe_t *js;
  eXosip_notify_t    *jn;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv2xx (id=%i)\r\n", tr->transactionid));

  if (MSG_IS_RESPONSE_FOR(sip, "PUBLISH"))
    {
      eXosip_pub_t *pub;
      int i;
i = _eXosip_pub_update( &pub , tr , sip ) ;
      if (i!=0)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"cb_rcv2xx (id=%i) No publication to update\r\n", tr->transactionid));
	}
      return;
    }
  else if (MSG_IS_RESPONSE_FOR(sip, "REGISTER"))
    {
      eXosip_event_t *je;
      eXosip_reg_t *jreg=NULL;
      osip_contact_t *contact_info;
      osip_contact_t *req_contact;

      /* find matching j_reg */
      _eXosip_reg_find( &jreg , tr ) ;
      if (jreg!=NULL)
      {
        /* If contact is NULL, it's for unregister. So the event should be "Register Fail" */
	    contact_info = osip_list_get (sip->contacts, 0);
		// unregister if originial request is "*"
		req_contact = osip_list_get(tr->orig_request->contacts, 0);
        if (contact_info == NULL || (req_contact && req_contact->url == NULL))
        {
            je = eXosip_event_init_for_reg(EXOSIP_REGISTRATION_FAILURE, jreg);

            if (je!=NULL)
            {
               je->server_expires = 0;
               report_event_with_status(je, sip);
            }
            return;
        }
	  
        je = eXosip_event_init_for_reg(EXOSIP_REGISTRATION_SUCCESS, jreg);
	  if (je!=NULL)
	    {
#ifdef FIX_REGISTER_TIME
		int pos;
		osip_generic_param_t *u_param;
		osip_contact_t *contact;

		// Currently, we support one contact only.
		// TODO: multiple conatcts?
		req_contact = osip_list_get(tr->orig_request->contacts, 0);
		if (req_contact == NULL)
		{
			// Unknown behavior
			OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,
				"cb_rcv2xx (id=%i) Error: request contact is NULL\r\n", 
				tr->transactionid));
			je->server_expires = 0;
		    report_event_with_status(je, sip);
			return;
		}

		pos = 0;
		contact = NULL;
		while (!osip_list_eol(sip->contacts, pos))
		{
			contact = osip_list_get (sip->contacts, pos);
			if (osip_uri_compare(contact->url, req_contact->url) == 0)
			{
				break; // found!
			}
			else
			{
				contact = NULL; // not match, reset to NULL
				pos++; // try next
			}
		}

		if (contact == NULL)
		{
			// contact can be NULL if unregister
			je->server_expires = 0;
		    report_event_with_status(je, sip);
			return;
		}

		pos = 0;
		je->server_expires = 0;
    		while (!osip_list_eol(contact->gen_params, pos))
      		{
			u_param =
		  		(osip_generic_param_t *) osip_list_get (contact->gen_params, pos);
		  	if ( strncmp("expires", u_param->gname, strlen("expires")) == 0) {	
				//printf ("** server expires =%d **\n", atoi(u_param->gvalue));	  		
				je->server_expires = atoi(u_param->gvalue);
				break;
			} else {
				//printf ("** %s=%s **\n", u_param->gname, u_param->gvalue);	  		
			}
			pos++;
		}
		
#endif	      
	      report_event_with_status(je, sip);
	    }
	}
      return;
    }

  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  jn = jinfo->jn;
  js = jinfo->js;

  if (MSG_IS_RESPONSE_FOR(sip, "OPTIONS"))
    {
      if (jc==NULL)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv2xx (id=%i) Error: no call or transaction info for OPTIONS transaction\r\n", tr->transactionid));
	  return;
	}
      else if (jc->c_out_options_tr==NULL)
	{
	  /* options is within a call */
	}
      report_call_event_with_status(EXOSIP_OPTIONS_ANSWERED, jc, jd, sip);
      return;
    }

  if (MSG_IS_RESPONSE_FOR(sip, "INVITE"))
    {
      cb_rcv2xx_4invite(tr, sip);
    }
  else if (MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
      cb_rcv2xx_4subscribe(tr, sip);
    }
  else if (MSG_IS_RESPONSE_FOR(sip, "BYE"))
    {
      if (jd!=NULL)
		{	
			report_call_event_with_status(EXOSIP_CALL_EXIT, jc, jd, sip);
			jd->d_STATE = JD_TERMINATED;
		}
    }
  else if (MSG_IS_RESPONSE_FOR(sip, "MESSAGE"))
    {
      eXosip_event_t *je;
      je = eXosip_event_init_for_message(EXOSIP_MESSAGE_SUCCESS, tr, sip);
      if (je!=NULL)
	report_event_with_status(je, sip);
      return;
    }
#ifdef SIP_PING
  else if (MSG_IS_RESPONSE_FOR(sip, "PING"))
	{
		eXosip_event_t *je;
		osip_contact_t *contact;

		eXosip_event_init(jinfo->nPort, &je, EXOSIP_PING_SUCCESS);
		if (je==NULL) return;

		osip_message_get_contact(sip, 0, &contact);
		if (contact)
		{
			char *tmp;
			osip_contact_to_str(contact, &tmp);
			snprintf(je->remote_contact, 255, "%s", tmp);
			osip_free(tmp);
		}
		report_event_with_status(je, sip);
		return;
	}
#endif
  else if (MSG_IS_RESPONSE_FOR(sip, "NOTIFY"))
    {
#ifdef SUPPORT_MSN
ddddddddddddddddddddd
      osip_header_t  *expires;
      osip_message_header_get_byname(tr->orig_request, "expires",
				     0, &expires);
      if (expires==NULL || expires->hvalue==NULL)
	{
	  /* UNCOMPLIANT UA without a subscription-state header */
	}
      else if (0==osip_strcasecmp(expires->hvalue, "0"))
	{
	  /* delete the dialog! */
	  if (jn!=NULL)
	    {
	      REMOVE_ELEMENT(eXosip.j_notifies, jn);
	      eXosip_notify_free(jn);
	    }
	}
#else
      osip_header_t  *sub_state;
      osip_message_header_get_byname(tr->orig_request, "subscription-state",
				     0, &sub_state);
      if (sub_state==NULL || sub_state->hvalue==NULL)
	{
	  /* UNCOMPLIANT UA without a subscription-state header */
	}
      else if (0==osip_strncasecmp(sub_state->hvalue, "terminated", 10))
	{
	  /* delete the dialog! */
	  if (jn!=NULL)
	    {
	      REMOVE_ELEMENT(eXosip.j_notifies, jn);
	      eXosip_notify_free(jn);
	    }
	}
#endif
    }
}

void eXosip_delete_early_dialog(eXosip_dialog_t *jd)
{
  if (jd == NULL) /* bug? */
      return;

  /* an early dialog was created, but the call is not established */
  if (jd->d_dialog!=NULL && jd->d_dialog->state==DIALOG_EARLY)
    {
      osip_dialog_free(jd->d_dialog);
      jd->d_dialog = NULL;
      eXosip_dialog_set_state(jd, JD_TERMINATED);
    }
}

static void
rcvregister_failure(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_event_t *je;
  eXosip_reg_t *jreg=NULL;
  /* find matching j_reg */
_eXosip_reg_find( &jreg , tr ) ;
  if (jreg!=NULL)
    {
      je = eXosip_event_init_for_reg(EXOSIP_REGISTRATION_FAILURE, jreg);
      report_event_with_status(je, sip);
    }
}

static void cb_rcv3xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t *jd;
  eXosip_call_t *jc;
  eXosip_subscribe_t *js;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv3xx (id=%i)\r\n", tr->transactionid));

  if (MSG_IS_RESPONSE_FOR(sip, "PUBLISH"))
    {
      eXosip_pub_t *pub;
      int i;
i = _eXosip_pub_update( &pub , tr , sip ) ;
      if (i!=0)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"cb_rcv3xx (id=%i) No publication to update\r\n", tr->transactionid));
	}
      return;
    }
  else if (MSG_IS_RESPONSE_FOR(sip, "REGISTER"))
    {
      rcvregister_failure(type, tr, sip);
      return;
    }

  if (jinfo==NULL) return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  js = jinfo->js;

  if (MSG_IS_RESPONSE_FOR(sip, "OPTIONS"))
    {
      if (jc==NULL)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv3xx (id=%i) Error: no call or transaction info for INFO transaction\r\n", tr->transactionid));
	  return;
	}
      else if (jc->c_out_options_tr==NULL)
	{
	  /* options is within a call */
	}

      report_call_event_with_status(EXOSIP_OPTIONS_REDIRECTED, jc, jd, sip);
      return;
    }

  if (MSG_IS_RESPONSE_FOR(sip, "INVITE"))
    {
      if (jd)
      {
        osip_dialog_update_route_set_as_uac(jd->d_dialog, sip);
        report_call_event_with_status(EXOSIP_CALL_REDIRECTED, jc, jd, sip);
      }
      else
      {
        eXosip_event_t *je;

        eXosip_event_init(jc->nPort, &je, EXOSIP_CALL_REDIRECTED);
        if (je)
        {
          osip_contact_t *contact;

          osip_message_get_contact(sip, 0, &contact);
          if (contact)
          {
            char *tmp;

            osip_contact_to_str(contact, &tmp);
            snprintf(je->remote_contact, 255, "%s", tmp);
            osip_free(tmp);

            je->cid = jc->c_id;
            report_event_with_status(je, sip);
          }
        }
      }
    }
  else if (MSG_IS_RESPONSE_FOR(sip, "MESSAGE"))
    {      
      eXosip_event_t *je;
      je = eXosip_event_init_for_message(EXOSIP_MESSAGE_FAILURE, tr, sip);
      if (je)
	report_event_with_status(je, sip);
      return;
    }    
  else if (MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
      eXosip_event_t *je;
      je = eXosip_event_init_for_subscribe(EXOSIP_SUBSCRIPTION_REDIRECTED, js, jd);
      if (je)
	report_event_with_status(je, sip);
    }
  
  if (jd==NULL) return;
  if (MSG_IS_RESPONSE_FOR(sip, "INVITE")
      || MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
      eXosip_delete_early_dialog(jd);
      if (jd->d_dialog==NULL)
	jd->d_STATE = JD_REDIRECTED;
    }

}

static void cb_rcv4xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t *jd;
  eXosip_call_t *jc;
  eXosip_subscribe_t *js;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv4xx (id=%i)\r\n", tr->transactionid));

  if (MSG_IS_RESPONSE_FOR(sip, "PUBLISH"))
    {
      eXosip_pub_t *pub;
      int i;
i = _eXosip_pub_update( &pub , tr , sip ) ;
      if (i!=0)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"cb_rcv4xx (id=%i) No publication to update\r\n", tr->transactionid));
	}
      return;
    }
  else if (MSG_IS_RESPONSE_FOR(sip, "REGISTER"))
    {
      rcvregister_failure(type, tr, sip);
      return;
    }

  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  js = jinfo->js;

  if (MSG_IS_RESPONSE_FOR(sip, "OPTIONS"))
    {
      if (jc==NULL)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv4xx (id=%i) Error: no call or transaction info for INFO transaction\r\n", tr->transactionid));
	  return;
	}
      else if (jc->c_out_options_tr==NULL)
	{
	  /* options is within a call */
	}

      report_call_event_with_status(EXOSIP_OPTIONS_REQUESTFAILURE, jc, jd, sip);      
      return;
    }

  if (MSG_IS_RESPONSE_FOR(sip, "INVITE"))
  {
    report_call_event_with_status(EXOSIP_CALL_REQUESTFAILURE, jc, jd, sip);      
  }
  else if (MSG_IS_RESPONSE_FOR(sip, "MESSAGE"))
  {
    eXosip_event_t *je;
    je = eXosip_event_init_for_message(EXOSIP_MESSAGE_FAILURE, tr, sip);
    if (je!=NULL)
      report_event_with_status(je, sip);
    return;
  }
  else if (MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
  {
    eXosip_event_t *je;
    je = eXosip_event_init_for_subscribe(EXOSIP_SUBSCRIPTION_REQUESTFAILURE, js, jd);
    if (je!=NULL)
      report_event_with_status(je, sip);
  }
  else if (MSG_IS_RESPONSE_FOR(sip, "REFER"))
  {
    report_call_event_with_status(EXOSIP_CALL_REFER_REQUESTFAILURE, jc, jd, sip);      
  }
  else if (MSG_IS_RESPONSE_FOR(sip, "BYE"))
  {
    report_call_event_with_status(EXOSIP_CALL_REQUESTFAILURE, jc, jd, sip);      
  }

  if (jd==NULL) return;
  if (MSG_IS_RESPONSE_FOR(sip, "INVITE")
      || MSG_IS_RESPONSE_FOR(sip, "BYE")
      || MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
      eXosip_delete_early_dialog(jd);
      if (MSG_TEST_CODE(sip, 401) || MSG_TEST_CODE(sip, 407))
	jd->d_STATE = JD_AUTH_REQUIRED;
      else
	jd->d_STATE = JD_CLIENTERROR;
    }

}

static void cb_rcv5xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t *jd;
  eXosip_call_t *jc;
  eXosip_subscribe_t *js;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv5xx (id=%i)\r\n", tr->transactionid));

  if (MSG_IS_RESPONSE_FOR(sip, "PUBLISH"))
    {
      eXosip_pub_t *pub;
      int i;
i = _eXosip_pub_update( &pub , tr , sip ) ;
      if (i!=0)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"cb_rcv3xx (id=%i) No publication to update\r\n", tr->transactionid));
	}
      return;
    }
  else if (MSG_IS_RESPONSE_FOR(sip, "REGISTER"))
    {
      rcvregister_failure(type, tr, sip);
      return;
    }

  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  js = jinfo->js;

  if (MSG_IS_RESPONSE_FOR(sip, "OPTIONS"))
    {
      if (jc==NULL)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv5xx (id=%i) Error: no call or transaction info for INFO transaction\r\n", tr->transactionid));
	  return;
	}
      else if (jc->c_out_options_tr==NULL)
	{
	  /* options is within a call */
	}
    
      report_call_event_with_status(EXOSIP_OPTIONS_SERVERFAILURE, jc, jd, sip);      
      return;
    }

  if (MSG_IS_RESPONSE_FOR(sip, "INVITE"))
  {
    report_call_event_with_status(EXOSIP_CALL_SERVERFAILURE, jc, jd, sip);      
  }
  else if (MSG_IS_RESPONSE_FOR(sip, "MESSAGE"))
  {
      eXosip_event_t *je;
      je = eXosip_event_init_for_message(EXOSIP_MESSAGE_FAILURE, tr, sip);
      if (je!=NULL)
	report_event_with_status(je, sip);
      return;
  }    
  else if (MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
  {
    eXosip_event_t *je;
    je = eXosip_event_init_for_subscribe(EXOSIP_SUBSCRIPTION_SERVERFAILURE, js, jd);
    if (je!=NULL)
      report_event_with_status(je, sip);
  }

  if (jd==NULL) return;
  if (MSG_IS_RESPONSE_FOR(sip, "INVITE")
      || MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
      eXosip_delete_early_dialog(jd);
      jd->d_STATE = JD_SERVERERROR;
    }

}

static void cb_rcv6xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t *jd;
  eXosip_call_t *jc;
  eXosip_subscribe_t *js;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv6xx (id=%i)\r\n", tr->transactionid));

  if (MSG_IS_RESPONSE_FOR(sip, "PUBLISH"))
    {
      eXosip_pub_t *pub;
      int i;
i = _eXosip_pub_update( &pub , tr , sip ) ;
      if (i!=0)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"cb_rcv6xx (id=%i) No publication to update\r\n", tr->transactionid));
	}
      return;
    }
  else if (MSG_IS_RESPONSE_FOR(sip, "REGISTER"))
    {
      rcvregister_failure(type, tr, sip);
      return;
    }

  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  js = jinfo->js;

  if (MSG_IS_RESPONSE_FOR(sip, "OPTIONS"))
    {
      if (jc==NULL)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcv6xx (id=%i) Error: no call or transaction info for INFO transaction\r\n", tr->transactionid));
	  return;
	}
      else if (jc->c_out_options_tr==NULL)
	{
	  /* options is within a call */
	}
      report_call_event_with_status(EXOSIP_OPTIONS_GLOBALFAILURE, jc, jd, sip);
      return;
    }

  if (MSG_IS_RESPONSE_FOR(sip, "INVITE"))
  {
      report_call_event_with_status(EXOSIP_CALL_GLOBALFAILURE, jc, jd, sip);
  }
  else if (MSG_IS_RESPONSE_FOR(sip, "MESSAGE"))
  {
      eXosip_event_t *je;
      je = eXosip_event_init_for_message(EXOSIP_MESSAGE_FAILURE, tr, sip);
      if (je!=NULL)
	report_event_with_status(je, sip);
      return;
  }    
  else if (MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
  {
    eXosip_event_t *je;
    je = eXosip_event_init_for_subscribe(EXOSIP_SUBSCRIPTION_GLOBALFAILURE, js, jd);
    if (je!=NULL)
      report_event_with_status(je, sip);
  }

  if (jd==NULL) return;
  if (MSG_IS_RESPONSE_FOR(sip, "INVITE")
      || MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
      eXosip_delete_early_dialog(jd);
      jd->d_STATE = JD_GLOBALFAILURE;
    }

}

static void cb_snd1xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t *jd;
  eXosip_call_t *jc;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_snd1xx (id=%i)\r\n", tr->transactionid));

  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  if (jd==NULL) return;
  jd->d_STATE = JD_TRYING;
}

static void cb_snd2xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t *jd=NULL;
  eXosip_call_t *jc;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_snd2xx (id=%i)\r\n", tr->transactionid));
  if( OSIP_IST_STATUS_2XX_SENT == type ){
  	/* check if it is re-INVITE */
	osip_generic_param_t *tag = NULL;
	osip_message_t *orgRequest = NULL;
	int i = -1;

	orgRequest = tr->orig_request;
	i = osip_to_get_tag(orgRequest->to, &tag); 
	if(0 == i) {	//  to tag in the request and it is a re-INVITE
		jd = NULL;
		/* first, look for a Dialog in the map of element */
		for (jc = eXosip.j_calls; jc!= NULL ; jc=jc->next)
		{
			for (jd = jc->c_dialogs; jd!= NULL ; jd=jd->next)
			{
				if (jd->d_dialog!=NULL)
				{
					if (osip_dialog_match_as_uas(jd->d_dialog, orgRequest)==0)
						break;
				}
			}
			if (jd!=NULL) break;
		}
	}
  }
  if (jinfo==NULL && NULL == jd){ 
  	 return;
  }
  if(NULL == jd){
  	jd = jinfo->jd;
  	jc = jinfo->jc;
  }
  
  if (jd==NULL) {
  	return;
  }
  if (MSG_IS_RESPONSE_FOR(sip, "INVITE")
      || MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
#if 1
	if (MSG_IS_RESPONSE_FOR(sip, "INVITE"))
	{
		osip_stop_retransmissions_from_dialog(tr->config, jd->d_dialog);
		osip_start_200ok_retransmissions(tr->config, jd->d_dialog, sip, tr->out_socket);
	}
#endif
      jd->d_STATE = JD_ESTABLISHED;
      return;
    }
  jd->d_STATE = JD_ESTABLISHED;
}

static void cb_snd3xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t *jd;
  eXosip_call_t *jc;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_snd3xx (id=%i)\r\n", tr->transactionid));
  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  if (jd==NULL) return;
  if (MSG_IS_RESPONSE_FOR(sip, "INVITE")
      || MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
      eXosip_delete_early_dialog(jd);
    }
  jd->d_STATE = JD_REDIRECTED;

  if (MSG_IS_RESPONSE_FOR(sip, "INVITE"))
  {
    report_call_event_with_status(EXOSIP_CALL_CLOSED, jc, jd, sip);
  }
}

static void cb_snd4xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t *jd;
  eXosip_call_t *jc;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_snd4xx (id=%i)\r\n", tr->transactionid));
  if (jinfo==NULL)
    return;
  /*+++++added by jack for handling 488 response re-INVITE+++++*/
	if(488 == sip->status_code){
		osip_generic_param_t *tag;
		int i;
		
		i = osip_to_get_tag (tr->orig_request->to, &tag);
		if(0 == i) /*only re-INVITE message has to tag */
			return;
		
	}
  /*-----end-----*/
  jd = jinfo->jd;
  jc = jinfo->jc;
  if (jd==NULL) return;
  if (MSG_IS_RESPONSE_FOR(sip, "INVITE")
      || MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
      eXosip_delete_early_dialog(jd);
    }
  jd->d_STATE = JD_CLIENTERROR;

  if (MSG_IS_RESPONSE_FOR(sip, "INVITE"))
  {
    report_call_event_with_status(EXOSIP_CALL_CLOSED, jc, jd, sip);
  }

}

static void cb_snd5xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t *jd;
  eXosip_call_t *jc;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_snd5xx (id=%i)\r\n", tr->transactionid));
  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  if (jd==NULL) return;
  if (MSG_IS_RESPONSE_FOR(sip, "INVITE")
      || MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
      eXosip_delete_early_dialog(jd);
    }
  jd->d_STATE = JD_SERVERERROR;

  if (MSG_IS_RESPONSE_FOR(sip, "INVITE"))
  {
    report_call_event_with_status(EXOSIP_CALL_CLOSED, jc, jd, sip);
  }

}

static void cb_snd6xx(int type, osip_transaction_t *tr,osip_message_t *sip)
{
  eXosip_dialog_t *jd;
  eXosip_call_t *jc;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_snd6xx (id=%i)\r\n", tr->transactionid));
  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  if (jd==NULL) return;
  if (MSG_IS_RESPONSE_FOR(sip, "INVITE")
      || MSG_IS_RESPONSE_FOR(sip, "SUBSCRIBE"))
    {
      eXosip_delete_early_dialog(jd);
    }
  jd->d_STATE = JD_GLOBALFAILURE;

  if (MSG_IS_RESPONSE_FOR(sip, "INVITE"))
  {
    report_call_event_with_status(EXOSIP_CALL_CLOSED, jc, jd, sip);
  }

}

static void cb_rcvresp_retransmission(int type, osip_transaction_t *tr, osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvresp_retransmission (id=%i)\r\n", tr->transactionid));
}

static void cb_sndreq_retransmission(int type, osip_transaction_t *tr, osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndreq_retransmission (id=%i)\r\n", tr->transactionid));
}

static void cb_sndresp_retransmission(int type, osip_transaction_t *tr, osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_sndresp_retransmission (id=%i)\r\n", tr->transactionid));
}

static void cb_rcvreq_retransmission(int type, osip_transaction_t *tr, osip_message_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_rcvreq_retransmission (id=%i)\r\n", tr->transactionid));
}

static void cb_transport_error(int type, osip_transaction_t *tr, int error)
{
  eXosip_dialog_t    *jd;
  eXosip_call_t      *jc;
  eXosip_subscribe_t *js;
  eXosip_notify_t    *jn;
  jinfo_t *jinfo =  (jinfo_t *)osip_transaction_get_your_instance(tr);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"cb_transport_error (id=%i)\r\n", tr->transactionid));
  if (jinfo==NULL)
    return;
  jd = jinfo->jd;
  jc = jinfo->jc;
  jn = jinfo->jn;
  js = jinfo->js;

  if (jn==NULL && js==NULL)
    return;

  if (MSG_IS_NOTIFY(tr->orig_request)
      && type==OSIP_NICT_TRANSPORT_ERROR)
    {
      /* delete the dialog! */
      REMOVE_ELEMENT(eXosip.j_notifies, jn);
      eXosip_notify_free(jn);
    }

  if (MSG_IS_SUBSCRIBE(tr->orig_request)
      && type==OSIP_NICT_TRANSPORT_ERROR)
    {
      /* delete the dialog! */
      REMOVE_ELEMENT(eXosip.j_subscribes, js);
      eXosip_subscribe_free(js);
    }

  if (MSG_IS_OPTIONS(tr->orig_request) && jc->c_dialogs==NULL
      && type==OSIP_NICT_TRANSPORT_ERROR)
    {
      /* delete the dialog! */
      REMOVE_ELEMENT(eXosip.j_calls, jc);
      eXosip_call_free(jc);
    }
}



int
eXosip_set_callbacks(osip_t *osip)
{
  /* register all callbacks */

  osip_set_cb_send_message(osip, &cb_udp_snd_message);
  
  osip_set_kill_transaction_callback(osip ,OSIP_ICT_KILL_TRANSACTION,
				 &cb_ict_kill_transaction);
  osip_set_kill_transaction_callback(osip ,OSIP_IST_KILL_TRANSACTION,
				 &cb_ist_kill_transaction);
  osip_set_kill_transaction_callback(osip ,OSIP_NICT_KILL_TRANSACTION,
				 &cb_nict_kill_transaction);
  osip_set_kill_transaction_callback(osip ,OSIP_NIST_KILL_TRANSACTION,
				 &cb_nist_kill_transaction);
          
  osip_set_message_callback(osip ,OSIP_ICT_STATUS_2XX_RECEIVED_AGAIN,
			&cb_rcvresp_retransmission);
  osip_set_message_callback(osip ,OSIP_ICT_STATUS_3456XX_RECEIVED_AGAIN,
			&cb_rcvresp_retransmission);
  osip_set_message_callback(osip ,OSIP_ICT_INVITE_SENT_AGAIN,
			&cb_sndreq_retransmission);
  osip_set_message_callback(osip ,OSIP_IST_STATUS_2XX_SENT_AGAIN,
			&cb_sndresp_retransmission);
  osip_set_message_callback(osip ,OSIP_IST_STATUS_3456XX_SENT_AGAIN,
			&cb_sndresp_retransmission);
  osip_set_message_callback(osip ,OSIP_IST_INVITE_RECEIVED_AGAIN,
			&cb_rcvreq_retransmission);
  osip_set_message_callback(osip ,OSIP_NICT_STATUS_2XX_RECEIVED_AGAIN,
			&cb_rcvresp_retransmission);
  osip_set_message_callback(osip ,OSIP_NICT_STATUS_3456XX_RECEIVED_AGAIN,
			&cb_rcvresp_retransmission);
  osip_set_message_callback(osip ,OSIP_NICT_REQUEST_SENT_AGAIN,
			&cb_sndreq_retransmission);
  osip_set_message_callback(osip ,OSIP_NIST_STATUS_2XX_SENT_AGAIN,
			&cb_sndresp_retransmission);
  osip_set_message_callback(osip ,OSIP_NIST_STATUS_3456XX_SENT_AGAIN,
			&cb_sndresp_retransmission);
  osip_set_message_callback(osip ,OSIP_NIST_REQUEST_RECEIVED_AGAIN,
			&cb_rcvreq_retransmission);
          
  osip_set_transport_error_callback(osip ,OSIP_ICT_TRANSPORT_ERROR,
				    &cb_transport_error);
  osip_set_transport_error_callback(osip ,OSIP_IST_TRANSPORT_ERROR,
				    &cb_transport_error);
  osip_set_transport_error_callback(osip ,OSIP_NICT_TRANSPORT_ERROR,
				    &cb_transport_error);
  osip_set_transport_error_callback(osip ,OSIP_NIST_TRANSPORT_ERROR,
				    &cb_transport_error);
  
  osip_set_message_callback(osip ,OSIP_ICT_INVITE_SENT,     &cb_sndinvite);
  osip_set_message_callback(osip ,OSIP_ICT_ACK_SENT,        &cb_sndack);
  osip_set_message_callback(osip ,OSIP_NICT_REGISTER_SENT,  &cb_sndregister);
  osip_set_message_callback(osip ,OSIP_NICT_BYE_SENT,       &cb_sndbye);
  osip_set_message_callback(osip ,OSIP_NICT_CANCEL_SENT,    &cb_sndcancel);
  osip_set_message_callback(osip ,OSIP_NICT_INFO_SENT,      &cb_sndinfo);
  osip_set_message_callback(osip ,OSIP_NICT_OPTIONS_SENT,   &cb_sndoptions);
  osip_set_message_callback(osip ,OSIP_NICT_SUBSCRIBE_SENT, &cb_sndsubscribe);
  osip_set_message_callback(osip ,OSIP_NICT_NOTIFY_SENT,    &cb_sndnotify);
  /*  osip_set_cb_nict_sndprack   (osip,&cb_sndprack); */
  osip_set_message_callback(osip ,OSIP_NICT_UNKNOWN_REQUEST_SENT, &cb_sndunkrequest);

  osip_set_message_callback(osip ,OSIP_ICT_STATUS_1XX_RECEIVED, &cb_rcv1xx);
  osip_set_message_callback(osip ,OSIP_ICT_STATUS_2XX_RECEIVED, &cb_rcv2xx);
  osip_set_message_callback(osip ,OSIP_ICT_STATUS_3XX_RECEIVED, &cb_rcv3xx);
  osip_set_message_callback(osip ,OSIP_ICT_STATUS_4XX_RECEIVED, &cb_rcv4xx);
  osip_set_message_callback(osip ,OSIP_ICT_STATUS_5XX_RECEIVED, &cb_rcv5xx);
  osip_set_message_callback(osip ,OSIP_ICT_STATUS_6XX_RECEIVED, &cb_rcv6xx);
  
  osip_set_message_callback(osip ,OSIP_IST_STATUS_1XX_SENT, &cb_snd1xx);
  osip_set_message_callback(osip ,OSIP_IST_STATUS_2XX_SENT, &cb_snd2xx);
  osip_set_message_callback(osip ,OSIP_IST_STATUS_3XX_SENT, &cb_snd3xx);
  osip_set_message_callback(osip ,OSIP_IST_STATUS_4XX_SENT, &cb_snd4xx);
  osip_set_message_callback(osip ,OSIP_IST_STATUS_5XX_SENT, &cb_snd5xx);
  osip_set_message_callback(osip ,OSIP_IST_STATUS_6XX_SENT, &cb_snd6xx);
  
  osip_set_message_callback(osip ,OSIP_NICT_STATUS_1XX_RECEIVED, &cb_rcv1xx);
  osip_set_message_callback(osip ,OSIP_NICT_STATUS_2XX_RECEIVED, &cb_rcv2xx);
  osip_set_message_callback(osip ,OSIP_NICT_STATUS_3XX_RECEIVED, &cb_rcv3xx);
  osip_set_message_callback(osip ,OSIP_NICT_STATUS_4XX_RECEIVED, &cb_rcv4xx);
  osip_set_message_callback(osip ,OSIP_NICT_STATUS_5XX_RECEIVED, &cb_rcv5xx);
  osip_set_message_callback(osip ,OSIP_NICT_STATUS_6XX_RECEIVED, &cb_rcv6xx);
      
  osip_set_message_callback(osip ,OSIP_NIST_STATUS_1XX_SENT, &cb_snd1xx);
  osip_set_message_callback(osip ,OSIP_NIST_STATUS_2XX_SENT, &cb_snd2xx);
  osip_set_message_callback(osip ,OSIP_NIST_STATUS_3XX_SENT, &cb_snd3xx);
  osip_set_message_callback(osip ,OSIP_NIST_STATUS_4XX_SENT, &cb_snd4xx);
  osip_set_message_callback(osip ,OSIP_NIST_STATUS_5XX_SENT, &cb_snd5xx);
  osip_set_message_callback(osip ,OSIP_NIST_STATUS_6XX_SENT, &cb_snd6xx);
  
  osip_set_message_callback(osip ,OSIP_IST_INVITE_RECEIVED,     &cb_rcvinvite);
  osip_set_message_callback(osip ,OSIP_IST_ACK_RECEIVED,        &cb_rcvack);
  osip_set_message_callback(osip ,OSIP_IST_ACK_RECEIVED_AGAIN,  &cb_rcvack2);
  osip_set_message_callback(osip ,OSIP_NIST_REGISTER_RECEIVED,  &cb_rcvregister);
  osip_set_message_callback(osip ,OSIP_NIST_BYE_RECEIVED,       &cb_rcvbye);
  osip_set_message_callback(osip ,OSIP_NIST_CANCEL_RECEIVED,    &cb_rcvcancel);
  osip_set_message_callback(osip ,OSIP_NIST_INFO_RECEIVED,      &cb_rcvinfo);
  osip_set_message_callback(osip ,OSIP_NIST_OPTIONS_RECEIVED,   &cb_rcvoptions);
  osip_set_message_callback(osip ,OSIP_NIST_SUBSCRIBE_RECEIVED, &cb_rcvsubscribe);
  osip_set_message_callback(osip ,OSIP_NIST_NOTIFY_RECEIVED,    &cb_rcvnotify);
  osip_set_message_callback(osip ,OSIP_NIST_UNKNOWN_REQUEST_RECEIVED, &cb_rcvunkrequest);


  return 0;
}

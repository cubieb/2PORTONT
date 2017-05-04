/***************************************************************************
 *            authentication.c
 *
 *  Fri Jul 16 12:08:34 2004
 *  Copyright  2004  Simon MORLAT
 *  simon.morlat@linphone.org
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#include "linphonecore.h"
#include "private.h"
#include "misc.h"
#include "eXosip2/eXosip.h"
#include "osipparser2/osip_message.h"
#ifdef CONFIG_RTK_VOIP_IP_PHONE
#include "uihandler.h"
#endif
#include "eXosip2.h"
#include "lpconfig.h"

extern LinphoneProxyConfig *linphone_core_get_proxy_config_from_rid(LinphoneCore *lc, int rid);
extern void linphone_proxy_config_set_realm(LinphoneProxyConfig *cfg, const char *realm);
extern void linphone_core_retry_proxy_register(LinphoneCore *lc, const gchar *realm);


static gboolean linphone_core_update_auth_info(LinphoneCore *lc, LinphoneProxyConfig *cfg, 
	const char *username, const char *realm)
{
	jauthinfo_t *authinfo;
	voipCfgPortParam_t *voip_ptr = &g_pVoIPCfg->ports[lc->chid];
	voipCfgProxy_t *proxy;

	authinfo = eXosip_find_authentication_info(username, realm);
	if (authinfo == NULL)
	{
		if (cfg == NULL)
		{
			g_warning("no existed authinfo for realm %s! please register first.\n");
			return FALSE;
		}

		proxy = &voip_ptr->proxies[cfg->index];
		eXosip_lock();
		eXosip_add_authentication_info(proxy->number, proxy->login_id, proxy->password, 
			NULL, realm);
		eXosip_unlock();
	}

	return TRUE;
}

static gboolean linphone_process_authentication(LinphoneCore *lc, osip_message_t *req, 
	osip_message_t *resp, LinphoneProxyConfig *cfg)
{
	char *username;
	osip_proxy_authenticate_t *proxy_auth;
	osip_www_authenticate_t *www_auth;
	osip_authorization_t *auth;
	gboolean have_it = TRUE;
	int auth_failed = 1;
	voipCfgPortParam_t *voip_ptr = &g_pVoIPCfg->ports[lc->chid];
	voipCfgProxy_t *proxy;
	int pos, pos_auth;
	proxy = &voip_ptr->proxies[cfg->index];
	pos = 0;
	osip_message_get_www_authenticate(resp, pos++, &www_auth);
	while (www_auth != NULL)
	{
		if (www_auth->realm == NULL)
		{
			g_warning("No realm in the server response.\n");
			return FALSE;
		}

		pos_auth = 0;
		osip_message_get_authorization(req, pos_auth++, &auth);
		while (auth != NULL)
		{
			if (strcmp(www_auth->realm, auth->realm) == 0)
				break;	// existed realm

			osip_message_get_authorization(req, pos_auth++, &auth);
		}

		if (auth == NULL)
		{
			// www->realm is new realm
			auth_failed = 0; 
		}
		
		osip_message_get_www_authenticate(resp, pos++, &www_auth);
	}

	pos = 0;
	osip_message_get_proxy_authenticate(resp, pos++, &proxy_auth);
	while (proxy_auth != NULL)
	{
		if (proxy_auth->realm == NULL)
		{
			g_warning("No realm in the server response.\n");
			return FALSE;
		}

		pos_auth = 0;
		osip_message_get_proxy_authorization(req, pos_auth++, &auth);
		while (auth != NULL)
		{
			if (strcmp(proxy_auth->realm, auth->realm) == 0)
				break;	// existed realm

			osip_message_get_proxy_authorization(req, pos_auth++, &auth);
		}

		if (auth == NULL)
		{
			// proxy->realm is new realm
			auth_failed = 0; 
		}

		osip_message_get_proxy_authenticate(resp, pos++, &proxy_auth);
	}
 #if 0 //ericchung mark, when auth fail, need check other auth data.
	if (auth_failed)
	{
		g_warning("auth failed!\n");
		return FALSE;
	}
#endif
	username = osip_uri_get_username(&resp->from->url);

	pos = 0;
	osip_message_get_www_authenticate(resp, pos++, &www_auth);
	while (www_auth != NULL)
	{
//		have_it = linphone_core_update_auth_info(lc, cfg, username, www_auth->realm) && have_it;
		have_it = linphone_core_update_auth_info(lc, cfg, proxy->login_id, www_auth->realm) && have_it;

		osip_message_get_www_authenticate(resp, pos++, &www_auth);
	}

	pos = 0;
	osip_message_get_proxy_authenticate(resp, pos++, &proxy_auth);
	while (proxy_auth != NULL)
	{
	//have_it = linphone_core_update_auth_info(lc, cfg, username, proxy_auth->realm) && have_it;
	have_it = linphone_core_update_auth_info(lc, cfg, proxy->login_id, proxy_auth->realm) && have_it;
		osip_message_get_proxy_authenticate(resp, pos++, &proxy_auth);
	}

	return have_it;
}

void linphone_register_authentication_required(LinphoneCore *lc,eXosip_event_t *ev){
	osip_message_t *resp, *req;
	LinphoneProxyConfig *cfg;
	eXosip_reg_t *reg;
	int i;
	
#ifdef OLD_API
	eXosip_reg_t *reg=eXosip_event_get_reginfo(ev);
#endif

	i = eXosip_reg_find_id(&reg, ev->rid);
	if(i<0){

		g_error("\r\n eXosip_reg_find_id fail\n");
	}

	g_return_if_fail(reg!=NULL);
	
	resp=reg->r_last_tr->last_response;
	g_return_if_fail(resp!=NULL);
	cfg=linphone_core_get_proxy_config_from_rid(lc,ev->rid);
	g_return_if_fail(cfg!=NULL);

	req=reg->r_last_tr->orig_request;
	g_return_if_fail(req!=NULL);

#if 0
	/*+++Modified by Jack to fix the REGISTER fail re-SEND problem+++*/
	{
		osip_message_t *req = NULL;
		osip_authorization_t *auth=NULL;

		req=reg->r_last_tr->orig_request;
		g_return_if_fail(req!=NULL);
		auth = (osip_authorization_t *)osip_list_get(req->authorizations, 0);
		if(NULL != auth)  //authenticate has been submitted, stop until the next reg. time reaches
			return;
	}
	/*---end---*/
#endif

	//[SD6, ericchung, exosip 3.5 integration
	//if (linphone_process_authentication(lc, req, resp, cfg)) 
	if (linphone_process_authentication(lc, ev->request, ev->response, cfg)) 
	//]

	{
		/* we have the information, so retry the register */
		eXosip_lock();
		eXosip_default_action(ev);

		eXosip_unlock();
	}
	else
	{
		cfg->auth_failed = TRUE;
	}
}

	
//[SD6, ericchung, exosip 3.5 integration
int linphone_call_authentication_required(LinphoneCore *lc, LinphoneCall *call,eXosip_event_t *ev)
{
	osip_message_t *resp, *req;
	eXosip_call_t *jc = NULL;
	eXosip_dialog_t *jd;
	osip_transaction_t *tr;
	LinphoneProxyConfig *cfg;

	g_return_val_if_fail(call != NULL, -1);

#ifdef MULTI_PROXY
	cfg = call->proxy;
#else
	linphone_core_get_default_proxy(lc, &cfg);
#endif
	g_return_val_if_fail(cfg != NULL, -1);

	//[SD6, ericchung, exosip 3.5 integration
	if (linphone_process_authentication(lc, ev->request, ev->response, cfg) == FALSE)
	//]
	{
		call->auth_failed = TRUE;
		return -1;
	}

	// call retry
	eXosip_lock();
	eXosip_default_action(ev);

	eXosip_unlock();
	return 0;
}

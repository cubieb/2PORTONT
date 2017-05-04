/*
 *-----------------------------------------------------------------------------
 *
 *               @@@@@@@      *   @@@@@@@@@           *
 *              @       @             @               *
 *              @             *       @      ****     *
 *               @@@@@@@      *       @     *    *    *
 *        ___---        @     *       @     ******    *
 *  ___---      @       @     *       @     *         *
 *   -_          @@@@@@@  _   *       @      ****     *
 *     -_                 _ -
 *       -_          _ -
 *         -_   _ -        s   e   m   i   c   o   n   d   u   c   t   o   r
 *           -
 *
 * (C) Copyright SiTel Semiconductor BV, unpublished work.file:

 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:		 		 si_ua_instantmessage.c
 * Purpose:
 * Created:		 		 Mar 2009
 * By:		 		 	 YP
 * Country:		 		 Greece
 *
 *-----------------------------------------------------------------------------
 */
/*========================== Include files ==================================*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <si_print_api.h>

#include "../CCFSM/ccfsm_api.h"
#include "../CCFSM/ccfsm_init.h"
#include "si_ua_callcontrol.h"
#include "si_ua_instantmessage.h"
#include <osipparser2/osip_body.h>
#include "../exosip2/src/eXosip.h"
#include "../exosip2/src/eXosip2.h"
extern  SICORE siphoneCore;

int sc1445x_im_send ( char* text, int accountID,char*toURI, int IPCall)
{
	int ret;
	osip_message_t *message;
	SICORE* pCore = &siphoneCore;
  	char from[64];
	char to[64];
 //	char route[64];
 	char *route =NULL;

 	if (IPCall)
		sprintf(from,"sip:%s@%s",&pCore->regusername[accountID][0], siphoneCore.IPAddress);
	else
   		sprintf(from,"sip:%s@%s",&siphoneCore.regusername[accountID][0], pCore->RegistrarIPAddress);

		if (pCore->remoteRegistrarPort>0)
			sprintf(to,"%s:%d",toURI,  pCore->remoteRegistrarPort);
		 else
			sprintf(to,"%s",toURI);

 //			sprintf(route,"sip:%s", pCore->RegistrarIPAddress);

  	/* else 
 	{
 		sprintf(from,"sip:%s@%s",&pCore->regusername[accountID][0], siphoneCore.IPAddress);

		if (pCore->remoteRegistrarPort>0)
			sprintf(to,"%s:%d",toURI, pCore->remoteRegistrarPort);
    	else
			sprintf(to,"%s",toURI);
	}*/
 
 	ret = eXosip_message_build_request(&message, "MESSAGE", to, from, route);
	if (ret!=0)
	{
		si_print(PRINT_LEVEL_ERR, "Fail to build message request \n " );
		return -1;
	}
   	osip_message_set_allow (message, SipAllowHeader);
 	osip_message_set_content_type (message, "text/html");
	osip_message_set_body (message,text, strlen(text) );
 	eXosip_message_send_request(message);
	return 0;
}

int sc1445x_im_recv (eXosip_event_t *pevent)
{
	ccfsm_im_recv_indication_data	m_im_recv_ind;
 	osip_body_t *txt;
	osip_message_t *sip = (osip_message_t *)pevent->request;
	osip_message_get_body (pevent->request , 0,&txt);

  if (txt==NULL) return -1;
 	if (sip && sip->content_type)
	{
		if (txt->body)
		{
			m_im_recv_ind.cback_id = CCFSM_IM_RECV_IND;

			m_im_recv_ind.accountid= findAccount(pevent->request->to->url->username);
			m_im_recv_ind.attachedentity =findEntity(m_im_recv_ind.accountid);

 
			if (pevent->request->from->displayname)
				strcpy(m_im_recv_ind.display, pevent->request->from->displayname);

			if (pevent->request->from->url->username)
				strcpy(m_im_recv_ind.username, pevent->request->from->url->username);
			if (pevent->request->from->url->host)
				strcpy(m_im_recv_ind.host, pevent->request->from->url->host);
 			if (!memcmp(sip->content_type->type, "text" ,4) && !memcmp(sip->content_type->subtype, "plain" ,5))
 			{
				m_im_recv_ind.im_type = ID_IM_TYPE_PLAIN_TEXT;
				m_im_recv_ind.im_size= strlen(txt->body);
				m_im_recv_ind.im_text = (void*)malloc(strlen(txt->body)+1);
				strcpy((char*)m_im_recv_ind.im_text, txt->body);
 			}
 			else if (!memcmp(sip->content_type->type, "text" ,4) && !memcmp(sip->content_type->subtype, "html" ,4))
			{
				m_im_recv_ind.im_type = ID_IM_TYPE_PLAIN_HTML;
				m_im_recv_ind.im_size= strlen(txt->body);
				m_im_recv_ind.im_text = (void*)malloc(strlen(txt->body)+1);
 				strcpy((char*)m_im_recv_ind.im_text, txt->body);
 
			}else {
				si_print(PRINT_LEVEL_ERR, "UNSUPPORTED MEDIA \n");				
				return -1;
			}
  			//TODO need to transfer the char type as well UTF or plain text
 			 ccfsm_callback(&m_im_recv_ind);
		}
  }
  return 0;
}

//------------------------------------------------------------------------------------//

	//----------------------publish the status of the

	/*eXosip_build_publish(osip_message_t **message,
				 const char *to,
				 const char *from,
				 const char *route,
				 const char *event,
				 const char *expires,
				 const char *ctype,
				 const char *body);*/

	/*int eXosip_publish (osip_message_t *message, const char *to);*/




const char *pres_note_str [4]={"online","away","dnd","Idle" };
const char *pres_status_basic_str [2]={"open","closed" };

const char *pres_xml_part [5] = {"<?xml version=\"1.0\" encoding=\"UTF-8\"?> <presence xmlns=\"urn:ietf:params:xml:ns:pidf\" entity=\"pres:",
								"\"> \r\n <tuple> <note> ",
								"</note> <status> <basic>",
								"</basic> </status> <contact priority=\"1\">\n",
								"</contact> </tuple> </presence>\r\n"};


#define SIPHONE_UA_PR_ACCEPT_HEADER "application/pidf+xml"

int si_pres_subscribe();

int sc1445x_pr_send (int accountID, pr_status_basic pres, pr_note note, int expires)
{
 int ret;
 osip_message_t *message;
 SICORE* pCore = &siphoneCore;
 char from[64];
 char to[64];
 char expires_str[8];
 char *tbody;
 
 if (pCore->remoteRegistrarPort>0)
  sprintf(to,"sip:%s@%s:%d",&(pCore->regusername[accountID][0]), &(pCore->RegistrarIPAddress [0]), pCore->remoteRegistrarPort);
 else
  sprintf(to,"sip:%s@%s",&(pCore->regusername[accountID][0]), &(pCore->RegistrarIPAddress [0]));

// sprintf(to,"sip:%s@%s:%d",&(pCore->regusername[accountID][0]), &(pCore->RegistrarIPAddress [0]),5130);

 sprintf(from,"sip:%s@%s",&(pCore->regusername[accountID][0]), siphoneCore.IPAddress);

 si_print(PRINT_LEVEL_INFO, "Trying to send presence status....\n");
 tbody = (char*)malloc(2000);
 if (tbody)
  sprintf(tbody,"%s%s%s%s%s%s%s%s%s",pres_xml_part[0],to, pres_xml_part[1],pres_note_str[note], pres_xml_part[2],pres_status_basic_str[pres], pres_xml_part[3],from, pres_xml_part[4]);
 else 
  return -1;
 sprintf (expires_str,"%d",expires);
    ret = eXosip_build_publish (&message, to, to, NULL, "presence", expires_str, "application/pidf+xml", tbody);
 free(tbody);

  if (ret!=0)
 {
  si_print(PRINT_LEVEL_ERR, "Fail to build message request \n " );
  return -1;
 }

  ret=eXosip_publish (message, to);

  if (ret!=0)
 {
  si_print(PRINT_LEVEL_ERR, "Fail to send message request \n " );
  return -1;
 }
  else
   si_print(PRINT_LEVEL_INFO, "Send ok \n " );

 return 0;
}

int si_pr_unsubscribe(ccfsm_attached_entity attachedentity,int sid)
{
 
	/*	osip_message_t *sub=NULL;
		int i;
		eXosip_dialog_t *jd = NULL;
		eXosip_subscribe_t *js = NULL;
		ccfsm_pr_sub_cback_data m_cback_sub ;

		if (sid > 0)
		{
		  eXosip_subscribe_dialog_find (sid, &js, &jd);
		}


		jd = js->s_dialogs;
		if (js==NULL || jd==NULL )
		{
			m_cback_sub.attachedentity=attachedentity;
			m_cback_sub.accountid=-1;
			m_cback_sub.status=CCFSM_PR_SUB_FAILED;
			m_cback_sub.cback_id=CCFSM_PR_SUB_CB;
			m_cback_sub.subAccount[0]=0;
			m_cback_sub.subRealm[0]=0;
			m_cback_sub.sid=sid;
			ccfsm_callback(&m_cback_sub);

			si_print(PRINT_LEVEL_ERR, "No dialog \n");
			return -1;

		}
		eXosip_lock();
		i = eXosip_subscribe_build_refresh_request (jd->d_id, &sub);
		if (i != 0)
		{
			m_cback_sub.attachedentity=attachedentity;
			m_cback_sub.accountid=-1;
			m_cback_sub.status=CCFSM_PR_SUB_FAILED;
			m_cback_sub.cback_id=CCFSM_PR_SUB_CB;
			m_cback_sub.subAccount[0]=0;
			m_cback_sub.subRealm[0]=0;
			m_cback_sub.sid=sid;
			ccfsm_callback(&m_cback_sub);
			eXosip_unlock();
			return -1;
		}

		osip_message_set_expires (sub, "0");
		//osip_message_set_accept(sub, SIPHONE_UA_PR_ACCEPT_HEADER);
		//or
		//-----------------------------------------------------------------------------
		osip_transaction_t *out_tr;
		out_tr = osip_list_get (jd->d_out_trs, 0);


		{
			int pos=0;
			osip_accept_t *_accept = NULL;

			i = osip_message_get_accept (out_tr->orig_request, pos, &_accept);
			while (i == 0 && _accept != NULL)
			  {
				osip_accept_t *_accept2;

				i = osip_accept_clone (_accept, &_accept2);
				if (i != 0)
				  {
					OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL,
											"Error in Accept header\n"));
					break;
				  }
				osip_list_add (&sub->accepts, _accept2, -1);
				_accept = NULL;
				pos++;
				i = osip_message_get_accept (out_tr->orig_request, pos, &_accept);
			  }
		}
		//-----------------------------------------------------------------

		//osip_message_set_header (sub, "Event", presence);
		//or
		//-----------------------------------------------------------------
		{
			osip_header_t *hdr=NULL;
			i = osip_message_header_get_byname (out_tr->orig_request, "Event", 0, &hdr);
			if (hdr==NULL || i<0)
			{
				OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL,
											"Missing Event header\n"));
				osip_message_free(sub);

				m_cback_sub.attachedentity=attachedentity;
				m_cback_sub.accountid=findAccount(sub->from->url->username);
				m_cback_sub.status=CCFSM_PR_SUB_FAILED;
				m_cback_sub.cback_id=CCFSM_PR_SUB_CB;
				strcpy(m_cback_sub.subAccount,sub->to->url->username);
				strcpy(m_cback_sub.subRealm,sub->to->url->host);
				m_cback_sub.sid=sid;
				ccfsm_callback(&m_cback_sub);
				eXosip_unlock();
				return -1;
			}

			osip_message_set_header (sub, "Event", hdr->hvalue);
		}
		//-----------------------------------------------------------------
		i = eXosip_subscribe_send_refresh_request (jd->d_id, sub);

		eXosip_unlock();*/


		eXosip_subscribe_remove (sid);

		/*m_cback_sub.attachedentity=attachedentity;
		m_cback_sub.accountid=findAccount(sub->from->url->username);
		m_cback_sub.status=CCFSM_PR_SUB_TERMINATED;
		m_cback_sub.cback_id=CCFSM_PR_SUB_CB;
	 	strcpy(m_cback_sub.subAccount,sub->to->url->username);
	 	strcpy(m_cback_sub.subRealm,sub->to->url->host);
	 	m_cback_sub.sid=sid;

	 	ccfsm_callback(&m_cback_sub);*/
 	 	return 0;
}


int si_pr_subscribe(ccfsm_attached_entity attachedentity, int accountID, char *subAccount, char *subRealm, int expires  )
{
	ccfsm_presence_subscribe_cback_data m_cback_sub ;
	osip_message_t *sreg;
	SICORE* pCore = &siphoneCore;
 	int ret;
 	char from[256];
  char to[256];
 
 	//TODO the subAccount, subRealm should be altered to a address book id
 	sprintf(from,"sip:%s@%s",&(pCore->regusername[accountID][0]), &(pCore->RegistrarIPAddress [0]));
 	// if no Realm is provided try to the same registrar
	// if ((subRealm==NULL) ||(subRealm[0]==0))
 		sprintf(to,"sip:%s@%s",subAccount, &(pCore->RegistrarIPAddress [0]));
	// else
	//	sprintf(to,"sip:%s@%s",subAccount, subRealm);

	// sprintf(to,"sip:%s@%s:5600",subAccount, subRealm);

	si_print(PRINT_LEVEL_INFO, "Subscribing to %s\n",to);

	eXosip_lock();

	si_print(PRINT_LEVEL_DEBUG, "Expiration is %d\n",expires);
	ret = eXosip_subscribe_build_initial_request(&sreg,to, from, NULL, "presence", expires);

	// osip_message_set_allow(sreg, SipAllowHeader);
	osip_message_set_accept(sreg, SIPHONE_UA_PR_ACCEPT_HEADER);

	// ret=_eXosip_dialog_add_contact(sreg,NULL);
 	ret=eXosip_subscribe_send_initial_request(sreg);

	eXosip_unlock();
  if (ret==-1)
 	{
		m_cback_sub.attachedentity=attachedentity;
		m_cback_sub.status=CCFSM_PRESENCE_SUBSCRIBE_FAILED;
		m_cback_sub.cback_id=CCFSM_PRESENCE_SUBSCRIBE_CB;
		m_cback_sub.accountid=accountID;
 		strcpy(m_cback_sub.subAccount,subAccount);
 		strcpy(m_cback_sub.subRealm,subRealm);
 		m_cback_sub.sid=-1;
		si_print(PRINT_LEVEL_ERR, "Unable to subscribe");
 	}
 	else
 	{
 		m_cback_sub.attachedentity=attachedentity;
 		m_cback_sub.accountid=accountID;
 		m_cback_sub.status=CCFSM_PRESENCE_SUBSCRIBE_SUCCESS;
 		m_cback_sub.cback_id=CCFSM_PRESENCE_SUBSCRIBE_CB;
 		strcpy(m_cback_sub.subAccount,subAccount);
 		strcpy(m_cback_sub.subRealm,subRealm);
 		m_cback_sub.sid=ret;
 	}
  si_print(PRINT_LEVEL_DEBUG, "Sending cb sid=%d\n",ret);
  ccfsm_callback(&m_cback_sub);

 	return 0;
}
int sc1445x_pr_recv (eXosip_event_t *pevent)
{
	ccfsm_presence_notify_indication_data	m_pr_not_recv_ind;
 	osip_body_t *txt;
	osip_message_t *sip = (osip_message_t *)pevent->request;
	osip_message_get_body (pevent->request , 0,&txt);
 
 	if (sip && sip->content_type)
	{
 			m_pr_not_recv_ind.cback_id = CCFSM_PRESENCE_NOTIFY_IND;
		 
 			m_pr_not_recv_ind.accountid= findAccount(pevent->request->to->url->username);
			m_pr_not_recv_ind.attachedentity =findEntity(m_pr_not_recv_ind.accountid);

			if (pevent->request->from->displayname)
				strcpy(m_pr_not_recv_ind.display, pevent->request->from->displayname);

			if (pevent->request->from->url->username)
				strcpy(m_pr_not_recv_ind.username, pevent->request->from->url->username);
			if (pevent->request->from->url->host)
				strcpy(m_pr_not_recv_ind.host, pevent->request->from->url->host);

		/*1.	application/xpidf+xml
			2.	text/xml+msrtc.pidf
			3.	application/pidf+xml*/

 				strcpy(m_pr_not_recv_ind.not_type,sip->content_type->type);
 				strcpy(m_pr_not_recv_ind.not_subtype,sip->content_type->subtype);
 				if (txt->body)
 				{
 					m_pr_not_recv_ind.not_size= strlen(txt->body);
 					m_pr_not_recv_ind.not_text = (void*)malloc(strlen(txt->body)+1);
 					strcpy((char*)m_pr_not_recv_ind.not_text, txt->body);
 				}
 				else
 				{
 					m_pr_not_recv_ind.not_size=0;
 					m_pr_not_recv_ind.not_text=NULL;
 				}
 			 	/*
					si_print(PRINT_LEVEL_DEBUG, "Username: %s, AccId:%d, type: %s, subtype: %s  \n", \	
						pevent->request->to->url->username,\
						m_pr_not_recv_ind.accountid,\
						m_pr_not_recv_ind.not_type, \
						sip->content_type->subtype ); 
					*/

 				ccfsm_callback(&m_pr_not_recv_ind);
  	}

   	return 0;
}


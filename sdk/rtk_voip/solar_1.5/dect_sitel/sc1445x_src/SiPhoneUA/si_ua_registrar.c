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
 * File:		 		 si_ua_callcontrol.c
 * Purpose:		 		 
 * Created:		 		 Nov 2007
 * By:		 		 	 YP
 * Country:		 		 Greece
 *
 *-----------------------------------------------------------------------------
 */
/*========================== Include files ==================================*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <si_print_api.h>
#include "si_ua_registrar.h" 
#include "si_ua_notify.h" 
#include "../common/si_phone_api.h"
#include "../common/si_configfiles_api.h"
#include "../common/si_timers_api.h"
   
#include "../exosip2/src/eXosip2.h"
#include "../common/operation_mode_defs.h" 
#include "../CCFSM/ccfsm_init.h"
  
///////////////////////////////////////////////////////////////////////////////
//				REGISTRATION  SECTION
//
///////////////////////////////////////////////////////////////////////////////
extern SICORE siphoneCore;
int registration_start(int account, int status)
{
 int i;
  if (account>0)
 {
  if (siphoneCore.useRegistration[account]) 
  {
    if (status){
    si_ua_registration_init_notification(account,CCFSM_REG_INIT);
     si_ua_register(&siphoneCore, &siphoneCore.siregistration[account-1], 1);  
    return 0;
   }
  }
  si_ua_registration_init_notification(account,CCFSM_REG_IDLE);
  return 0;
  }
 
 for (i=0;i<MAX_SUPPORTED_ACCOUNTS;i++)
 {
     if (siphoneCore.useRegistration[i]) 
  {
    if (status)
   {
    si_ua_registration_init_notification(account,CCFSM_REG_INIT);
    si_ua_register(&siphoneCore, &siphoneCore.siregistration[i], 1);  
    continue;
   }
  }
   si_ua_registration_init_notification(i,CCFSM_REG_IDLE);
  }
 return 0;
}

int checkToReRegister(SICORE* pCore)
{
	int i;
 
 	for (i=0;i<MAX_SUPPORTED_ACCOUNTS;i++)
	{
		if (pCore->siregistration[i].state != CCFSM_REG_INIT){
   			return 0;
		}
	}

	eXosip_clear_authentication_info();
  	for (i=0;i<MAX_SUPPORTED_ACCOUNTS;i++)
	{
		if (siphoneCore.useRegistration[i])	
		{
			strcpy(pCore->siregistration[i].regusername,&pCore->regusername[i][0]);
			strcpy(pCore->siregistration[i].regpassword,&pCore->regpassword[i][0]);
			strcpy(pCore->siregistration[i].reglogin,&pCore->reglogin[i][0]);
 			if (eXosip_add_authentication_info(	&pCore->regusername[i][0],
				&pCore->reglogin[i][0],&pCore->regpassword[i][0],NULL,pCore->regrealm))
		  	    {
					si_print(PRINT_LEVEL_ERR, "\n\nFatalError(EXOSIP_SET_PARAMETER_FAILED REGISTRATION \n\n ");
 					return -1;
				}
		}else {
			ccfsm_registration_cback_data m_cback_reg ={CCFSM_REGISTRATION_EVENT, 0, CCFSM_REG_INIT};
			m_cback_reg.accountid = i;
  		 	ccfsm_callback(&m_cback_reg);
  		}
	}
   	for (i=0;i<MAX_SUPPORTED_ACCOUNTS;i++)
	{
 		if (siphoneCore.useRegistration[i])	
		{
 			si_ua_register(pCore, &pCore->siregistration[i], 1);		
 		}
 	}
	return 0;
}

int si_ua_unregister(SICORE* pCore, siregistration_t *pRegisterInfo)
{
	int ret;
 	ret = 	eXosip_register_remove(pRegisterInfo->registrationID);
	if (ret==-1) {
		 
		return -1;
	}
	//FIX BROADWORKS - uncomment
	//si_ua_register(&siphoneCore,pRegisterInfo, 0);
	return 0;
}
 
siregistration_t *si_ua_get_reg_account(int rid)
{
	int i;
	for (i=0;i<MAX_SUPPORTED_ACCOUNTS;i++)
	{
		if (siphoneCore.siregistration[i].registrationID==rid)
			return &siphoneCore.siregistration[i];
	}
	return NULL;
}
 
int si_ua_register(SICORE* pCore, siregistration_t *pRegisterInfo, int regEnable)
{
	int ret;	
  	char contact[64];
  	char from[64];
	char proxy[64];
 	osip_message_t *reg =NULL;
	 
 	eXosip_lock();
 
 
 	 if ((pCore->firewallIP[0]!='\0') &&  (eXosip_is_public_address(pCore->RegistrarIPAddress ) ) )
	 {	
  		 sprintf(contact, "sip:%s@%s:%d",pRegisterInfo->regusername,pCore->firewallIP, siphoneCore.SipServerPort); 
 		 sprintf(from,"sip:%s@%s", pRegisterInfo->regusername,pCore->RegistrarIPAddress);// >RegistrarIPAddress); 

	 }else{
   		sprintf(contact,"sip:%s@%s:%d", pRegisterInfo->regusername,pCore->IPAddress, siphoneCore.SipServerPort); 
 // 		 sprintf(contact, "sip:%s@%s:%d",pRegisterInfo->regusername,pCore->firewallIP, "6074"); 
		 sprintf(from,"sip:%s@%s", pRegisterInfo->regusername,pCore->RegistrarIPAddress);// >RegistrarIPAddress); 
	  }

	 if (pCore->remoteRegistrarPort>0)
	 {
		 sprintf(proxy,"sip:%s:%d",pCore->RegistrarIPAddress,pCore->remoteRegistrarPort); 
 	 }
	 else {
		 sprintf(proxy,"sip:%s",pCore->RegistrarIPAddress); 
  	 }

	 if (regEnable){
 
	   	 pRegisterInfo->registrationID = eXosip_register_build_initial_register(from, proxy,NULL,pCore->regExpTime, &reg);
	 }
	else 
	{
  		 pRegisterInfo->registrationID = eXosip_register_build_initial_register(from, proxy,"*",0, &reg);
	 }


 	
	 if (pRegisterInfo->registrationID<1)
 	 {
 
 	   FatalError(EXOSIP_BUILD_REGISTER_FAILED, "eXosip_register_build_initial_register"); 
 	   eXosip_unlock();
	   return -1;
	} 
  	// fill message headers according to RFC 3261,  
 	//FIX BROADWORKS
	//	osip_message_set_allow(reg, SipAllowHeader);
	{
		osip_from_t *display = osip_message_get_from(reg);
		osip_to_t *to = osip_message_get_to(reg);
		if (display)
		{
			sprintf(contact, "\"%s\"",pRegisterInfo->regusername); 
			osip_from_set_displayname(display, osip_strdup (contact));
 			osip_to_set_displayname(to, osip_strdup (contact));
 		}
		   //osip_contact_t *dest;
		//	osip_message_get_contact (reg,0,&dest);
		//	sprintf(contact, "\"%s\"",pRegisterInfo->regusername); 
		//	dest->displayname = osip_strdup (contact);

//			sprintf(contact,"<sip:%s;transport=tcp;>lr", pCore->RegistrarIPAddress); 
  //		
	 
			//osip_message_set_record_route (reg,  osip_strdup (contact));

		//		sprintf(contact,"<sip:%s@%s:%d;transport=tcp>", pRegisterInfo->regusername,pCore->IPAddress, 5095); 
 		//	    dest->url = osip_strdup (contact);
  
 	}
// 	osip_message_set_supported(reg, SIPHONE_UA_SUPPORTED_HEADER_REGISTER);
     //send a registration messagef
 
   	ret = eXosip_register_send_register( pRegisterInfo->registrationID, reg );
  	if (ret!=0)  
	{
 	   FatalError(EXOSIP_SEND_REGISTER_FAILED, "eXosip_register_send_register"); 
	   eXosip_unlock();	
	  return -1;
	} 

	if (regEnable){
 
		pRegisterInfo->state = CCFSM_REG_IN_PROGRESS;
	}
	else{
 
		pRegisterInfo->state = CCFSM_UNREG_IN_PROGRESS;
	}
   	return 0;
}

int si_get_account_from_rid(int rid)
{
	int i;
  
	for (i=0;i<MAX_SUPPORTED_ACCOUNTS;i++)
	{
		if (siphoneCore.siregistration[i].registrationID==rid){
			return i;
 		}
 	}
	return -1;
}

int si_ua_registrationended(int rid, eXosip_event_t *pEvent, int flag)

{
	osip_allow_t *allow;
	osip_header_t *allow_events;
 	ccfsm_registration_cback_data m_cback_reg ={CCFSM_REGISTRATION_EVENT, 0, CCFSM_REG_SUCCESS};
 	int i, ret, pos=0;
 	 
	for (i=0;i<MAX_SUPPORTED_ACCOUNTS;i++)
	{
		if (siphoneCore.siregistration[i].registrationID==rid){
 			 	m_cback_reg.accountid = i;
				m_cback_reg.status = flag;
				siphoneCore.siregistration[i].allow =0;
				break;   	
		}
 	}
	//MAY new feature
	if (flag==CCFSM_REG_SUCCESS)
	{
		// FIX BROADWORKS - MWI
		if (siphoneCore.NATMode == (int)NAT_STUN ) 
			 si_ua_init_SIPKeepAlive(pEvent->did); 

		//Subscribe for MWI
 		si_mwi_subscribe(0,0,NULL,NULL, 3600);
/*
 		if (siphoneCore.Presence){
			printf("SUBSCRIBE for Presence !!!!!!!!!!!!!!!!!\n");
  			si_pr_subscribe(0,m_cback_reg.accountid, &siphoneCore.regusername[m_cback_reg.accountid][0], &siphoneCore.RegistrarIPAddress [0], 3600 );
		}
*/		 
		for (;;){
		   ret = osip_message_get_allow (pEvent->response , pos,&allow);
		   if (allow->value)
		   {
			   if (!strcmp(allow->value, "SUBSCRIBE")){
 				   siphoneCore.siregistration[i].allow |=REG_ALLOW_SUBSCRIBE;
				   // FIX BROADWORKS
				   //si_mwi_subscribe(0,0,NULL,NULL, 3600);
				   goto _exit; 
 			   }
		   } 
    		   if (ret==-1) break;
		   pos++;
 		} 
		for (;;){
 			ret = osip_message_header_get_byname(pEvent->response, "Allow-Events", pos, &allow_events);
			if (ret>=0)
			{
 				if (strstr(allow_events->hvalue,"message-summary"))
				{
 				   siphoneCore.siregistration[i].allow |=REG_ALLOW_SUBSCRIBE;
				   // FIX BROADWORKS
				   //si_mwi_subscribe(0,0,NULL,NULL, 3600);
				   goto _exit; 
 				}
			}
  		   if (ret==-1) break;
		   pos++;
 		}  
	}else{

		 ;//si_dns_set_alternate(pevent);
	}
_exit:
 	ccfsm_callback(&m_cback_reg);
 	return 0;
}


int si_ua_registration_init_notification(int account, int flag)
{
  	ccfsm_registration_cback_data m_cback_reg ={CCFSM_REGISTRATION_EVENT, 0, CCFSM_REG_SUCCESS};
 	m_cback_reg.accountid = account;
	m_cback_reg.status = flag;
 	ccfsm_callback(&m_cback_reg);
 	return 0;
}
 
///////////////////////////////////////////////////////////////////////////////
//				SUBSCRIPTION  SECTION
//
///////////////////////////////////////////////////////////////////////////////
int si_mwi_subscribe(ccfsm_attached_entity attachedentity, 
					 int accountID, 
					 char *subAccount, 
					 char *subRealm, 
					 int expires  )
{
	osip_message_t *sreg;
	SICORE* pCore = &siphoneCore;
 	int ret;
 	char from[256];
  char to[256];
	char toHost[256];
  	//TODO the subAccount, subRealm should be altered to a address book id
	//sprintf(from,"sip:%s@%s",&(pCore->regusername[accountID][0]), &(pCore->RegistrarIPAddress [0]));
	//sprintf(to,"sip:%s",  &(pCore->RegistrarIPAddress [0]));
	//sprintf(to,"sip:%s@%s",&(pCore->regusername[accountID][0]), &(pCore->RegistrarIPAddress [0]));

  	sprintf(from,"sip:%s@%s",&(pCore->regusername[accountID][0]),&(pCore->RegistrarIPAddress [0]));//
  	sprintf(to,"sip:%s@%s",&(pCore->regusername[accountID][0]),&(pCore->RegistrarIPAddress [0]));

    /*
		//if no Realm is provided try to the same registrar
  		if ((subRealm==NULL) ||(subRealm[0]==0))
 			sprintf(to,"sip:%s@%s",subAccount, &(pCore->RegistrarIPAddress [0]));
 		else
 			sprintf(to,"sip:%s@%s",subAccount, subRealm);
	*/

 	si_print(PRINT_LEVEL_INFO, "Subscribing to %s\n",to);
 	eXosip_lock();
 	si_print(PRINT_LEVEL_DEBUG, "Expiration is %d\n",expires);

	sprintf(toHost,"sip:%s",&(pCore->RegistrarIPAddress [0]));

	ret = eXosip_subscribe_build_initial_request(&sreg,to, from, toHost, "message-summary", expires);
 	osip_message_set_allow(sreg, SipAllowHeader);
	osip_message_set_accept(sreg, SIPHONE_UA_ACCEPT_HEADER);
 	//ret=_eXosip_dialog_add_contact(sreg,NULL);
 	ret=eXosip_subscribe_send_initial_request(sreg);
 	eXosip_unlock();

   	return 0;
}


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
 * (C) Copyright SiTel Semiconductor BV, unpublished work.
 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:			si_ua_server_api.c
 * Purpose:				 		 
 * Created:		06/12/2007
 * By:		 		YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

/*========================== Include files ==================================*/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <net/if.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/ioctl.h> 
#include <arpa/inet.h> 

#include <si_print_api.h>

#include "../common/si_config_file.h"
#include "../common/si_phone_api.h"
#include "../../mcu/sc1445x_mcu_api.h"
#include "si_ua_instantmessage.h"

#include "si_ua_fax.h"
#include "si_ua_dtmf.h"
#include "si_ua_notify.h"
#include "si_ua_server_api.h"
#include "si_ua_callcontrol.h" 
#include "si_ua_hold.h"
#include "si_ua_events.h"
#include "si_ua_registrar.h"

#include "../CCFSM/ccfsm_init.h"
/*========================== Local macro definitions ========================*/


/*========================== Global definitions =============================*/
  

/*========================== Local function prototypes ======================*/


/*========================== Local data definitions =========================*/
 
/*========================== Function definitions ===========================*/
 
   
 
/*sicall_t *FindCallPerClient(int clientID, int vline, int extension)
{
	int i;
 	for (i=0;i<MAX_SUPPORTED_CALLS	;i++)
	{
	     if (((SiCall[i].cbackId==clientID) && (SiCall[i].callid==vline) && (SiCall[i].accountID==extension)  ) || 
			 ((SiCall[i].cbackId==-1) && (SiCall[i].callid==vline) && (SiCall[i].accountID==extension)))
	      {	
		    	  SiCall[i].cbackId=clientID;
			     return &SiCall[i];
		}
	}

	return NULL;
}*/
  
extern  SICORE siphoneCore;
 
void si_ua_configure(int iface, void* params, int size)
{
	//fax support	
 	sc1445x_fax_params(params,size);
}

int sip_answer_call(int callid, int codec, int portid, int accountid)
{
 	ccfsm_cback_type pCBack;
	sicall_t* pCall;
	int err=-1;
  
	pCall=sip_find_callid(callid);
	if (pCall)
	{
		pCall->port = portid;
		pCall->codectype = codec; 
		pCall->accountID = accountid;
  	err = si_ua_acceptCall(pCall);
  }
	pCBack.ccfsm_updatestatus_cback.cback_id = CCFSM_ANSWERING_CB;
 	pCBack.ccfsm_updatestatus_cback.callid = callid; 
	pCBack.ccfsm_updatestatus_cback.reasoncode = err;
	ccfsm_callback(&pCBack);
	return -1;	 
}

int sip_hold_call(int callid, int flag)
{
 	ccfsm_updatestatus_cback_data m_cback_updatestatus ; 
	sicall_t* pCall;
	int err=-1;
  
	pCall=sip_find_callid(callid);
 
	if (pCall)
	{
		err = si_ua_holdOn(pCall, flag);
   	}
 
	m_cback_updatestatus.cback_id = CCFSM_HOLDING_CB;
 	m_cback_updatestatus.callid = callid; 
 	m_cback_updatestatus.reasoncode =err;
	ccfsm_callback(&m_cback_updatestatus);
	return -1;	 
}

int sip_resume_call(int callid, int flag)
{
 	ccfsm_updatestatus_cback_data m_cback_updatestatus ; 
	sicall_t* pCall;
	int err=-1;
  
	pCall=sip_find_callid(callid);
	
	if (pCall)
	{
		err = si_ua_holdOff(pCall);//, flag);
   	}
 
	m_cback_updatestatus.cback_id = CCFSM_RESUMING_CB;
 	m_cback_updatestatus.callid = callid; 
 	m_cback_updatestatus.reasoncode =err;
	ccfsm_callback(&m_cback_updatestatus);
	return -1;	 
}

int sip_blindtransfer_call(int callid, char *number)
{
 	ccfsm_updatestatus_cback_data m_cback_updatestatus ; 
	sicall_t* pCall;
	int err ;
  
	pCall=sip_find_callid(callid);
	if (pCall==NULL) 
		err=-1; 
	else
	  	err = si_ua_blindTransfer(pCall,number); 

	m_cback_updatestatus.cback_id = CCFSM_BLINDTRANSFERRING_CB;
 	m_cback_updatestatus.callid = callid; 
 	m_cback_updatestatus.reasoncode =err;
	ccfsm_callback(&m_cback_updatestatus);
	return err;	 
}

int sip_attendedtransfer_call(int fromid, int toid)
{
 	ccfsm_updatestatus_cback_data m_cback_updatestatus ; 
	sicall_t* pCallfrom;
	sicall_t* pCallto;
	int err;
 
	pCallfrom = sip_find_callid(fromid);
	pCallto = sip_find_callid(toid);
	if ((pCallfrom==NULL) || (pCallto==NULL))
		err= -1; 
	else
	  	err = si_ua_attendedTransfer(pCallfrom,pCallto); 

	m_cback_updatestatus.cback_id = CCFSM_ATTENDEDTRANSFERRING_CB;
 	m_cback_updatestatus.callid = toid; 
 	m_cback_updatestatus.reasoncode =err;
  	ccfsm_callback(&m_cback_updatestatus);
	return err;	 
}

int sip_terminate_call(int callid)
{
	ccfsm_updatestatus_cback_data m_cback_updatestatus; 
	sicall_t* pCall;

 	pCall=sip_find_callid(callid);
	if (pCall)
	{
		m_cback_updatestatus.cback_id = CCFSM_TERMINATING_CB;
		m_cback_updatestatus.accountid = pCall->accountID; 
		m_cback_updatestatus.callid =callid; 
		m_cback_updatestatus.state = CCFSM_STATE_TERMINATING;
		m_cback_updatestatus.reasoncode =0;
		m_cback_updatestatus.info [0]=0;
		// ccfsm_callback(&m_cback_updatestatus);
 		si_ua_terminateCall(pCall);
  	// send a callback to call control to notify a call change
		return 0;
	}else {
		si_print(PRINT_LEVEL_ERR, "%s,IVALID STATUS \n", __FUNCTION__);
	}
 
 	m_cback_updatestatus.cback_id = CCFSM_TERMINATING_CB;
 	m_cback_updatestatus.callid = callid; 
	m_cback_updatestatus.state = CCFSM_STATE_TERMINATING;
	m_cback_updatestatus.reasoncode = -1;
	m_cback_updatestatus.info[0] = 0;
 	ccfsm_callback(&m_cback_updatestatus);

  return -1;	 
}
 
int sip_invite(ccfsm_outgoing_call_w_replaces_req_data *pCallData)
{
	sicall_t* pTransferedCall;
	int err;
  
	pTransferedCall = sip_find_callid(pCallData->transferred_callid);
 
 
	if (pCallData->replaces[0]!='\0')
	{
 		err = si_ua_Invite_w_Replace(pCallData->accountid, pCallData->portid , pCallData->callid, (int)(pCallData->codec ), NULL, pCallData->transfer_to  ,pCallData->calltype , pTransferedCall,pCallData->transfer_from , pCallData->replaces);
	}
	else{
   		err= si_ua_callEstablish(pCallData->accountid, pCallData->portid , pCallData->callid,  (int)(pCallData->codec) ,NULL, pCallData->transfer_to  ,pCallData->calltype , pTransferedCall);
 	}

	return err;
}

void sip_reset(void)
{
	si_ua_reset();
}
void sip_dtmf_start(char key)
{
	int i;
	for (i=0;i<MAX_SUPPORTED_CALLS;i++)
	{
		if (siphoneCore.DtmfMode[(int)SiCall[i].accountID]==DTMF_MODE_INFO)
		{
			si_ua_sendDtmfInfo(&SiCall[i], (unsigned char)key, (unsigned char)200);
		}
  	}
}
int sip_send_instant_message(char  *pIMtext, int accountid,char *dialled_num, int callType )
{

	return sc1445x_im_send(pIMtext, accountid, dialled_num, callType);//fix April 29, 2009

}

void sip_fax_start(int callid, int faxevent) 
{
  sicall_t* pCall;
  pCall=sip_find_callid(callid);
  if (pCall==NULL) return ;
	
  if( faxevent == 0x16 )
  {
    // CNG tone detected. Fax call origination. Make sure we answer 
    // the Re-Invite with T38 SDP params.
    pCall->faxsupport = 3; 
  }
  else if( faxevent == 0x15 )
  {
    // CED tone detected. Fax call termination. Send Re-Invite
	sc1445x_fax_open_connection(pCall);
  }else
  {
    si_print(PRINT_LEVEL_ERR,"%s IVALID FAX EVENT %x \n",__FUNCTION__, faxevent);
  }
}

void sip_incall_codec_change(int callid, int codec) 
{
	sicall_t* pCall;
	int i;
	pCall=sip_find_callid(callid);
	if (pCall==NULL) return ;
 
	//if current call is a narrowband one, place G.722 in the first position of codel list
	// otherwise replace the G722 in the first position with a narrowband coded
 
	if (siphoneCore.Codecs[pCall->accountID][0]==9)//G722
	{		
		siphoneCore.Codecs[pCall->accountID][0]=siphoneCore.Codecs[pCall->accountID][1];
		siphoneCore.Codecs[pCall->accountID][1]=9;
		si_ua_codecChange(pCall);
		return ;
	}

	for (i=0;i<MAX_SUPPORTED_CODECS;i++)
	{
		if (siphoneCore.Codecs[pCall->accountID][i]==9)//G722
		{	
			siphoneCore.Codecs[pCall->accountID][i]=siphoneCore.Codecs[pCall->accountID][0];
			siphoneCore.Codecs[pCall->accountID][0]=9;
			si_ua_codecChange(pCall);
			return ;
		}
	}
}

sicall_t* sip_find_callid(int index)
{
	int i;
	for (i=0;i<MAX_SUPPORTED_CALLS;i++)
		{
		  if (SiCall[i].state && SiCall[i].callid==index)  
		  {	
				return &SiCall[i];
		  }
 	}
	return NULL;
}
 
int GetUserAgentStatus(int clientID)
{
	int i;
	UserAgentStatus uaStatus;
	for (i=0;i<MAX_SUPPORTED_ACCOUNTS;i++)
		uaStatus.isregistered[i] = siphoneCore.siregistration[i].state; 

	strncpy((char*)uaStatus.IPAddress, (char*)siphoneCore.IPAddress, 16);
 // si_cb_callback(clientID, -1, SI_SYSTEM_GETSTATUS, -1, -1, (int)&uaStatus, "getStatus") ;

  return 0;
}
  
int sip_presence_subscribe (ccfsm_attached_entity attachedentity, int accountID, char *subAccount, char *subRealm, int expires)
{
 	return si_pr_subscribe(attachedentity, accountID, subAccount, subRealm, expires);
}

int sip_presence_publish (int accountID, pr_status_basic pres, pr_note note, int expires)
{
  return sc1445x_pr_send(accountID, pres, note, expires);
} 
  
int sip_registration(int account, int status)
{
	return registration_start(account, status);
}
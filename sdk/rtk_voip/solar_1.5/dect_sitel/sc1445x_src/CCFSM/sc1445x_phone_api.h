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
 * File:		sc1445x_phone_api.h
 * Purpose:		
 * Created:		Sep 2008
 * By:			KF
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SC1445x_PHONE_API_H
#define SC1445x_PHONE_API_H

#define GUI_MAX_CALLS_SUPPORTED 16
#include "ccfsm_api.h" 

typedef struct _sc1445x_phone_call_info
{
	ccfsm_attached_entity attached;
 	ccfsm_call_type		calltype;
	ccfsm_call_states		state;
	int flags;
	int parkid;
	int lineid;
 	int reference;
  int callid;
	int accountid;
	int portid;
  char user_name[64];
 	char display_name[64];
	char host_name[64];
	int codec_id;
	char info[64];
	int CallRecord;
	int inConference;
	int encryption;
}sc1445x_phone_call_info;

 typedef struct _sc1445x_phone_active_calls_list
{
	char display[100];
	sc1445x_phone_call_info call_info;
}sc1445x_phone_active_calls_list;

#define GUI_CONFERENCE_MAX_MEMBERS 4

typedef struct _gui_conference_data_t
{
	int state;
	int callid;
	int numofmembers;
 	char members[GUI_CONFERENCE_MAX_MEMBERS][64];
}gui_conference_data_t;

extern gui_conference_data_t conference_data;
extern sc1445x_phone_active_calls_list m_active_calls_list[ ];
 
// GUI to CCFSM Interface
typedef struct _phone_api
{
	int (*init)(void);
	void (*phoneevents)(int var1,int var2,int var3, void* var4);	
	void (*ring)(int mode, int port, int accountid, int codec, int numofcalls, char* displayname);	
 	void (*terminatecall)(int port, int callid, int numofcalls, int term_reason );	
	
	void (*processcalls)(void);
	void (*retransmit)(void* numofparams, int size);
	int (*assignedline)(int);
	void (*improcess) ( char *host, char *username, char * display, void *im_text, char *imsize, int im_type);

}phone_api;

extern phone_api phone_function[ ];
int sc1445x_phoneapi_init(void);
 
int sc1445x_phoneapi_reset(void);
int sc1445x_phoneapi_configure(ccfsm_attached_entity attachedentity, void* params, int size); 
int sc1445x_phoneapi_register(ccfsm_attached_entity attachedentity, int account, int flag);

void sc1445x_phoneapi_application_callback(ccfsm_cback_type* pCBack);
 int sc1445x_phoneapi_send_im(ccfsm_attached_entity attachedentity, unsigned char * dialnumber, char *msg,int portid, int account);
 
int sc1445x_phoneapi_create_newcall(ccfsm_attached_entity attachedentity, unsigned char * dialnumber,  int portid, int account,ccfsm_codec_type codec, int reference);
int sc1445x_phoneapi_create_internalcall(ccfsm_attached_entity attachedentity, unsigned char * dialnumber, int portid, int account, ccfsm_codec_type codec, int reference);

int	sc1445x_phoneapi_blindtransfer(int callid,  char *number); 
int	sc1445x_phoneapi_attendedtransfer(  int fromid,  int toid); 

int sc1445x_phoneapi_call_answer(int callid, ccfsm_attached_entity attached, int portid, int codec, int accountid);	 
int sc1445x_phoneapi_call_terminate(int callid, ccfsm_attached_entity attached);
int sc1445x_phoneapi_incall_codec_change(int callid, int codec);

int sc1445x_phoneapi_call_hold(int callid);	 
int sc1445x_phoneapi_call_resume(int callid);	 
int sc1445x_phoneapi_dtmf_start(char key, int port);
int sc1445x_phoneapi_dtmf_stop(void);
  
void sc1445x_phoneapi_start_fax(ccfsm_attached_entity attached,int callid, int port, int faxevent);
void  sc1445x_phoneapi_add_call(ccfsm_cback_type* pData);
sc1445x_phone_call_info* sc1445x_phoneapi_modify_call_state(ccfsm_cback_type* pData) ;
void sc1445x_phoneapi_del_call(ccfsm_cback_type* pData) ;		 
void sc1445x_phoneapi_replace_call(ccfsm_cback_type* pData); 		 

void sc1445x_phoneapi_show_registration(ccfsm_cback_type* pCBack); 		 
 

char* sc1445x_phoneapi_getDiplayStatusString( ccfsm_call_states status) ;
int sc1445x_phoneapi_getAllowedUserActions(ccfsm_call_states status); 
char*  sc1445x_phoneapi_getUsername(char *userstring);

int sc1445x_phoneapi_find_numof_calls(int portid);
int sc1445x_phoneapi_find_numof_calls_notinconference(int portid);
int sc1445x_phoneapi_find_numof_calls_perline(int line);
int sc1445x_phoneapi_find_numof_calls_peraccountid(int accountid);
int sc1445x_phoneapi_get_call_flags(int portid, int account);
int sc1445x_phoneapi_find_numof_calls_per_port(ccfsm_call_states state, int port);
int sc1445x_phoneapi_update_port(int callid, int port);
int sc1445x_phoneapi_update_accountid(int callid, int accountid);
int sc1445x_phoneapi_find_numof_calls_per_state(ccfsm_call_states state, int portid);

sc1445x_phone_call_info * sc1445x_phoneapi_find_call_per_state_ofportid(ccfsm_call_states state, int port);

sc1445x_phone_call_info * sc1445x_phoneapi_find_call_per_flag(int flag, int portid, int account);
sc1445x_phone_call_info * sc1445x_phoneapi_find_call_per_state_ofaccountid(ccfsm_call_states state, int accountid);
sc1445x_phone_call_info * sc1445x_phoneapi_find_call_per_accountid(int accountid);

ccfsm_call_states sc1445x_phoneapi_find_state_per_callid(int callid);

sc1445x_phone_call_info * sc1445x_phoneapi_find_call_per_state(ccfsm_call_states state,int portid);
sc1445x_phone_call_info * sc1445x_phoneapi_find_call(int portid);
sc1445x_phone_call_info * sc1445x_phoneapi_find_call_withID(int callid);
sc1445x_phone_call_info * sc1445x_phoneapi_find_call_withline(int lineid);

sc1445x_phone_call_info * sc1445x_phoneapi_get_next_call_per_state(sc1445x_phone_call_info *pCall, ccfsm_call_states state, int portid);
sc1445x_phone_call_info * sc1445x_phoneapi_get_next_call(sc1445x_phone_call_info *pCall,int portid);

int sc1445x_phoneapi_CheckNumber(char *URI, char *number, char *registrar);
int sc1445x_phoneapi_extract_address(unsigned char *IP_address, char *string);
int sc1445x_phoneapi_assignedline(ccfsm_attached_entity attachedentity, int callid)  ;

void sc1445x_phoneapi_retransmit(ccfsm_attached_entity attachedentity, void* pData, int size) ;
void sc1445x_phoneapi_processcalls(ccfsm_attached_entity attachedentity); 
void sc1445x_phoneapi_terminatecall(ccfsm_attached_entity attachedentity, int port, int callid, int numofcalls, int term_reason);
void sc1445x_phoneapi_ringmode(ccfsm_attached_entity attachedentity, ring_mode mode, int port, int accountid, int codec, int numofcalls, char *display);
 
void sc1445x_phoneapi_setled(int callid, int flag);
 
void sc1445x_phoneapi_fix_calling_username(unsigned char *newnumber, unsigned char *dialnumber);
char *returnCodecName(int cnum);

int sc1445x_phoneapi_presence_notify_process(ccfsm_cback_type* pData);
int sc1445x_phoneapi_presence_subscribe_process (ccfsm_cback_type* pData);
int sc1445x_phoneapi_im_process(ccfsm_cback_type* pData);
int sc1445x_phoneapi_presence_subscribe (ccfsm_attached_entity attachedentity, int portid, int account, int expires, char * subAccount, char * subRealm);
int sc1445x_phoneapi_prerence_unsubscribe (ccfsm_attached_entity attachedentity, int sid, int portid);
int sc1445x_phoneapi_presence_publish(ccfsm_attached_entity attachedentity, pr_status_basic pres, pr_note note, int portid, int account, int expires);
#endif  

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
 * File:		si_dect_callcontrol.c
 * Purpose:		
 * Created:	26/1/2009
 * By:			EF
 * Country:	Greece
 *
 *-----------------------------------------------------------------------------
 */

/*========================== Include files ==================================*/
#include <stdio.h>
#include <stdlib.h>

#ifdef SA_CVM
#include <stdarg.h>
#else
#include <si_print_api.h>
#endif

#include "../../mcu/sc1445x_mcu_api.h"
#include "../../mcu/sc1445x_amb_api.h"
#include "../../mcu/sc1445x_alsa.h"
#include "../CCFSM/sc1445x_phone_api.h"
/* libUASetNonCriticalCoreValues */
#include "../SiPhoneUA/siUAFunctions.h"

#include "cvmcon.h"
#include "si_dect.h"

#if (!(defined ATA_ENABLED))
	#include "std-def.h"
#endif

#ifdef NATALIE
	#include "ISip450Api.h"
#else
	#include "primitiv.h"
	#include "api-intf.h"
#endif

/* sc_fileapi_LoadStarCodes */
#include "../common/si_configfiles_api.h"
#include "si_dect_callcontrol.h"
/* MAX_ALLOWED_ACCOUNTS */
#include "../common/si_config_file.h"

#ifdef SA_CVM
#include "si_dect_api.h"
#endif

#ifdef SA_CVM
/* enumaration for print debug level (copy from si_print_api.h) */
typedef enum _PRINT_DEBUG_LEVEL
{
	PRINT_LEVEL_EMERG=0,
	PRINT_LEVEL_ALERT,
	PRINT_LEVEL_CRIT,
	PRINT_LEVEL_ERR,
	PRINT_LEVEL_WARNING,
	PRINT_LEVEL_NOTICE,
	PRINT_LEVEL_INFO,
	PRINT_LEVEL_DEBUG,
	PRINT_DEBUG_LEVEL_INVALID
}PRINT_DEBUG_LEVEL;

static inline int si_print(PRINT_DEBUG_LEVEL print_level, char *fmt, ...)
{
	va_list args;
	
	va_start( args, fmt );
	
	return vprintf( fmt, args );
}
#endif

/*========================== Local macro definitions ========================*/
/* Star Codes for internal use only. Not shown on web configuration page (BOA) */
#define DECT_INTERN_STAR_CODES_NUMBER 6
#define DECT_STAR_CODE_DIGITS_MAX 4		//*xx#
#define DECT_MAX_KEY_SEQ 64		
#define FALSE 0
#define TRUE 1

#ifdef SA_CVM
#define NO_STAR_CODE
#endif

/*========================== Global definitions =============================*/
/* Change Base Statiob RF ID according to MAC Address */
#ifdef SET_RFPI
	extern unsigned char CC_RFPI[5];//requested rfpi by the CC
#endif

/* Structure to store changed config settings after star code */
ConfigSettings m_ConfigFile;

/*========================== Local function prototypes ======================*/
#ifndef SA_CVM
static void dect_callprogress_tone_handling(sc1445x_mcu_tone_bmp std_tone, sc1445x_ae_tone dynamic_tone, sc1445x_ae_tone periodic_tone, int reason, int port);
#endif
#ifndef NO_STAR_CODE
static void dect_process_star_code(unsigned char portid, unsigned char accountid, char *star_code, char *dialnumber);
static void dect_select_codec_scode(unsigned char portid, unsigned char accountid, void* code);
static void dect_add_to_conf_scode(unsigned char portid, unsigned char accountid, void *dummy);
static void dect_blind_transfer_scode(unsigned char portid, unsigned char accountid, char *number);
static void dect_callerID_block_scode(unsigned char portid, unsigned char accountid, void *dummy);
static void dect_callerID_send_scode(unsigned char portid, unsigned char accountid, void *dummy);
static void dect_call_waiting_off_scode(unsigned char portid, unsigned char accountid, void *dummy);
static void dect_call_waiting_on_scode(unsigned char portid, unsigned char accountid, void *dummy);
static void dect_call_forward_uncond_on_scode(unsigned char portid, unsigned char accountid, char *number);
static void dect_call_forward_uncond_off_scode(unsigned char portid, unsigned char accountid, void *dummy);
static void dect_call_forward_busy_on_scode(unsigned char portid, unsigned char accountid, char *number);
static void dect_call_forward_busy_off_scode(unsigned char portid, unsigned char accountid, void *dummy);
static void dect_call_forward_delayed_on_scode(unsigned char portid, unsigned char accountid, char *number);
static void dect_call_forward_delayed_off_scode(unsigned char portid, unsigned char accountid, void *dummy);
static void dect_internal_call_scode(unsigned char initiator_portid, unsigned char initiator_accountid, char *to_accountid);
static void dect_unpark_call_scode(unsigned char portid, unsigned char accountid, void *dummy);
static void dect_park_call_scode(unsigned char portid, unsigned char accountid, void *dummy);
static void dect_dnd_scode_off(unsigned char portid, unsigned char accountid, void *dummy);
static void dect_dnd_scode_on(unsigned char portid, unsigned char accountid, void *dummy);
#endif
#ifndef SA_CVM
static void sc1445x_dect_terminal_hold(ccfsm_attached_entity attachedentity, unsigned char portid, unsigned char accountid, unsigned char hold_state);
static void sc1445x_dect_blind_transfer(unsigned char portid, char* number);
static void sc1445x_dect_park_call(unsigned char portid, unsigned char acccountid, int toline);
static void sc1445x_dect_unpark_call(unsigned char portid, unsigned char accountid, int toline);
static void sc1445x_dect_terminal_internal_connect_lines(unsigned short initiator_portid, unsigned short initiator_accountid, char *to_accountid);
#endif

/*========================== Local data definitions =========================*/
int internalDECTCallTable[MAX_ALLOWED_DECT_HANDSETS] = {-1, -1, -1, -1, -1, -1, -1};  //+1 for broadcast
char dialno[MAX_ALLOWED_DECT_HANDSETS][DECT_MAX_KEY_SEQ];
unsigned char key_seq_indx[MAX_ALLOWED_DECT_HANDSETS] = {0, 0, 0, 0, 0, 0, 0};
unsigned char needs_number_info[MAX_ALLOWED_DECT_HANDSETS] = {0, 0, 0, 0, 0, 0, 0};
unsigned char dect_terminal_audio_peripheral_status[MAX_ALLOWED_DECT_HANDSETS] = {0, 0, 0, 0, 0, 0, 0};

#ifndef NO_STAR_CODE
typedef struct _dect_star_code {
	char name[30];
	char local;
	char code[3];
	void (*star_code_func)(unsigned char, unsigned char, void *);
} dect_star_code_t;

typedef void (*star_code_func_t)(unsigned char, unsigned char, void *);

/* Table of supported Star Codes */
dect_star_code_t dect_star_code_table [] =
{
  {"", TRUE, "", ( star_code_func_t )dect_add_to_conf_scode},
  {"", TRUE, "", ( star_code_func_t )dect_blind_transfer_scode},
  {"", TRUE, "", ( star_code_func_t )dect_callerID_block_scode},
  {"", TRUE, "", ( star_code_func_t )dect_callerID_send_scode},
  {"", TRUE, "", ( star_code_func_t )dect_call_waiting_off_scode},
  {"", TRUE, "", ( star_code_func_t )dect_call_waiting_on_scode},
  {"", TRUE, "", ( star_code_func_t )dect_call_forward_uncond_on_scode},
  {"", TRUE, "", ( star_code_func_t )dect_call_forward_uncond_off_scode},
  {"", TRUE, "", ( star_code_func_t )dect_call_forward_busy_on_scode},
  {"", TRUE, "", ( star_code_func_t )dect_call_forward_busy_off_scode},
  {"", TRUE, "", ( star_code_func_t )dect_call_forward_delayed_on_scode},
  {"", TRUE, "", ( star_code_func_t )dect_call_forward_delayed_off_scode},
  {"", TRUE, "", ( star_code_func_t )dect_internal_call_scode},
  {"", TRUE, "", ( star_code_func_t )dect_unpark_call_scode},
  {"", TRUE, "", ( star_code_func_t )dect_park_call_scode},
  {"", TRUE, "", ( star_code_func_t )dect_dnd_scode_off},
  {"", TRUE, "", ( star_code_func_t )dect_dnd_scode_on}
};

typedef struct _dect_intern_star_code {
	char name[30];
	char local;
	char code[3];
	void (*star_code_func)(unsigned char, unsigned char, void *);
} dect_intern_star_code_t;

/* Table of supported internal Star Codes */
dect_intern_star_code_t dect_intern_star_code_table [] =
{
  {"PCMU",  TRUE, "00", dect_select_codec_scode},  /* PCMU */
  {"G.726", TRUE, "01", dect_select_codec_scode},  /* G.726 */
  {"G.723", TRUE, "02", dect_select_codec_scode},  /* G.723 */
  {"PCMA",  TRUE, "03", dect_select_codec_scode},  /* PCMA */
  {"G.722", TRUE, "04", dect_select_codec_scode},  /* G.722 */
  {"G.729", TRUE, "05", dect_select_codec_scode}   /* G.729 */
};
#endif // !NO_STAR_CODE

/*========================== Function definitions ===========================*/

#ifndef SA_CVM
void _sc1445x_dect_terminal_outgoing_req(ccfsm_attached_entity attachedentity, unsigned char *dialnumber, int portid, int accountid, ccfsm_codec_type codec)
{
	sc1445x_phone_call_info *pCallInfo;
	int numofcalls;
	int allownewcall=0;
	int i=0;

	if ((dialnumber == NULL) || strlen((char*)&dialnumber) < 0) return;

 	while (i < strlen((char *)dialnumber)){
		if (dialnumber[i]=='*') dialnumber[i]='.';	
 		i++;
	} 

  sc1445x_phoneapi_fix_calling_username(dialnumber, dialnumber);

  si_print(PRINT_LEVEL_DEBUG, "\n DECT Dial %s at port %d and handid %d \n", dialnumber, portid, accountid);

	pCallInfo = sc1445x_phoneapi_find_call_per_accountid(accountid);
 
  if (pCallInfo == NULL)
  {
 		sc1445x_phoneapi_create_newcall(attachedentity, (unsigned char*)dialnumber, portid, accountid, codec, 0);	  
  }
  else
  {
  	//get the number of calls in the same portid/accountid
		allownewcall = numofcalls = sc1445x_phoneapi_find_numof_calls(portid);

 		//verify that there is no call in talking state or in any other active state (eg sending a request)
		while (numofcalls && pCallInfo)
		{
			if ((pCallInfo->state==CCFSM_STATE_HOLD) || (pCallInfo->state==CCFSM_STATE_INACTIVE) || (pCallInfo->state==CCFSM_STATE_DIALING) || (pCallInfo->state==CCFSM_STATE_IDLE))
			{
				pCallInfo = sc1445x_phoneapi_get_next_call(pCallInfo, accountid);
				numofcalls--;
			}
			else return ;
 		}

 		if (allownewcall < MAX_ALLOWED_CALLS_PER_DECT_PORT) // all calls are in HOLD or INACTIVE states	
		{
			sc1445x_phoneapi_create_newcall(attachedentity, (unsigned char*)dialnumber, portid, accountid, codec, 0);	  
		}
 	}
}
#endif // SA_CVM


void _sc1445x_dect_terminal_audio_peripheral_change(ccfsm_attached_entity attachedentity, int port, int accountid, ccfsm_audio_peripheral_status hookstatus, int codec, unsigned char appliedtocall) 
{
#ifdef SA_CVM
	// In our case, paramters are: 
	//  - attachedentity = CCFSM_ATTACHED_DECT (2)
	//  - port = 0 ~ n (pcm port; it is dynamic pair with accountid)
	//  - accountid = 0 ~ n 
	//  - hookstatus = CCFSM_HOOK_OFF (2) or CCFSM_HOOK_ON (1)
	//  - codec = 0 or CCFSM_NARROWBAND_CODEC (1) (pcm codec)
	//  - appliedtocall = 1 ~ n + 1 (handset_id)
	//
	char input;
	
	switch( hookstatus ) {
	case CCFSM_HOOK_OFF:
		if( codec == CCFSM_WIDEBAND_CODEC )
			input = 2;
		else
			input = 1;
		break;
	case CCFSM_HOOK_ON:
		input = 0;
		break;
	default:
		goto label_not_expected_key;
		break;
	}
	
	dect_api_R2S_pressed_key( accountid, port, input );
			
label_not_expected_key:	
	
	printf( "TODO: _sc1445x_dect_terminal_audio_peripheral_change\n" );
#else
	unsigned char oldstatus = 0;
	int i = 0;

	si_print(PRINT_LEVEL_DEBUG, "DECT hook Rcvd at port %d with accountid %d and codec %d\n\n", port, accountid, codec);

  oldstatus = dect_terminal_audio_peripheral_status[accountid];
	dialno[accountid][0] = '\0';
	key_seq_indx[accountid] = 0;
	needs_number_info[accountid] = 0;

	if ((hookstatus==CCFSM_HOOK_OFF) && !oldstatus)
	{
		sc1445x_phone_call_info * pCall;
    sc1445x_phone_call_info * pCall_br;

    sc1445x_dect_set_pcm_line_type(port, codec);

 		//check if there is pending incoming call in ringing state
    if (internalDECTCallTable[accountid]==-1)
		{
			si_print(PRINT_LEVEL_DEBUG, "DECT Hook Off NO INT CALL \n\n");
 			pCall = sc1445x_phoneapi_find_call_per_state_ofaccountid(CCFSM_STATE_INCOMING, accountid);
      pCall_br = sc1445x_phoneapi_find_call_per_state_ofaccountid(CCFSM_STATE_INCOMING, CCFSM_DECT_BROADCAST_ID);
	
			if (pCall && pCall->accountid == accountid)
			{
        si_print(PRINT_LEVEL_DEBUG, "DECT Hook Off to answer incoming with port %d\n\n", port);
				sc1445x_amb_cid_hookoff_ind(port);
  			sc1445x_phoneapi_call_answer(pCall->callid, attachedentity, port, codec, accountid);

			}else if ((pCall_br) && (appliedtocall == 255))//appliedtocall is needed to distinguish between br and another call ringing
			{
				si_print(PRINT_LEVEL_DEBUG, "DECT Hook Off to answer incoming Broadcast \n\n");
				sc1445x_amb_cid_hookoff_ind(port);
				sc1445x_phoneapi_call_answer(pCall_br->callid, attachedentity, port, codec, accountid);
        
      }else{
        si_print(PRINT_LEVEL_DEBUG, "DECT Hook Off to make new outgoing \n\n");
 				if (!sc1445x_phoneapi_find_numof_calls(port))
				{
					dect_callprogress_tone_handling(MCU_TONE_DL_BT, SC1445x_AE_TONE_INVALID, SC1445x_AE_TONE_INVALID, 0, port);
				} 
 			}
		}else{
			 si_print(PRINT_LEVEL_DEBUG, "DECT Hook Off to answer internal call \n\n");
			/* During connect the first handset does not know which port to connect yet */
			/* Does not know the second port */
			/* Therefore the table internalDECTCallTable of the first accountid stores the port of the first device */
			/* instead of the second */
			/* Here, at hook-off, from the second handset to answer the internal call, the port is known because it was */
			/* stored to the account previous holding the port of the first account */
			for (i=0; i<MAX_ALLOWED_DECT_HANDSETS; i++)
			{
				/* the second account is found here */
				if ((internalDECTCallTable[accountid] == internalDECTCallTable[i]) && (i != accountid))
				{
					internalDECTCallTable[i] = port;
					break;
				}
			}		
      dect_callprogress_tone_handling(MCU_TONE_NA_BT, SC1445x_AE_TONE_INVALID, SC1445x_AE_TONE_INVALID, 0, port);
      dect_callprogress_tone_handling(MCU_TONE_NA_BT, SC1445x_AE_TONE_INVALID, SC1445x_AE_TONE_INVALID, 0, internalDECTCallTable[accountid]);
			si_print(PRINT_LEVEL_DEBUG, "!!!!!!! Connect line1 %d line2 %d \n\n", internalDECTCallTable[accountid], port);
			sc1445x_amb_internal_connect_lines((unsigned short)internalDECTCallTable[accountid], (unsigned short)port);
 		}
		dect_terminal_audio_peripheral_status[accountid] = 1;			
	}
	else if ((hookstatus==CCFSM_HOOK_ON) &&  oldstatus)
	{
    si_print(PRINT_LEVEL_DEBUG, "DECT Hook ON with accountid %d \n\n", accountid);
 		dect_terminal_audio_peripheral_status[accountid] = 0;	 
    dect_callprogress_tone_handling(MCU_TONE_NA_BT, SC1445x_AE_TONE_INVALID, SC1445x_AE_TONE_INVALID, 0, port);
		sc1445x_dect_terminal_terminate_allcalls(attachedentity, port, accountid);
 	}
#endif // SA_CVM
}


int _sc1445x_dect_terminal_get_key_seq(ccfsm_attached_entity attachedentity, char *key_seq, int portid, int accountid, ccfsm_codec_type codec)
{
#ifdef SA_CVM
	// In our case, paramters are 
	//  - attachedentity = CCFSM_ATTACHED_DECT (2)
	//  - key_seq = an key array with '\0' terminator 
	//  - portid = 0 ~ n (pcm port)
	//  - accountid = 0 ~ n (it is dynamic pair with port)
	//  - codec = CCFSM_NARROWBAND_CODEC (1) or CCFSM_WIDEBAND_CODEC (2)
	//
	char ch;
	char *ch_ptr = key_seq;
	
	while( ( ch = *ch_ptr ++ ) ) 
		dect_api_R2S_pressed_key( accountid, portid, ch );
	
	printf( "TODO: _sc1445x_dect_terminal_get_key_seq: %s\n", key_seq );
	
	return 0;
#else
  #ifndef NO_STAR_CODE
	static unsigned char scodes_loaded = 0;
  #endif
	char *tmp_strcode = NULL, *tmp_dialno = NULL;
	char star_code[MAX_ALLOWED_DECT_HANDSETS][DECT_MAX_KEY_SEQ];
	int i = 0;

	if ((key_seq == NULL) || (strlen(key_seq) <= 0) || (strlen(key_seq) >= DECT_MAX_KEY_SEQ))
	{
		si_print(PRINT_LEVEL_ERR, "Wrong key sequence length \n");
		return -1;
	}

	/* Scan key sequence */
	while (i < strlen(key_seq))
	{
		/* Copy sequence to dialno buffer */
		strcpy(&dialno[accountid][key_seq_indx[accountid]], &key_seq[i]);
		/* Play tones only if key sequence is coming one by one */
		/* this should be improved to play tones when we get an entire string from dect */
		if (strlen(key_seq) == 1){
			sc1445x_phoneapi_dtmf_start(dialno[accountid][key_seq_indx[accountid]], portid);
		}
		/* check if an entire star code sequence *xx# exists */
		if ((dialno[accountid][0] == '*') && (dialno[accountid][DECT_STAR_CODE_DIGITS_MAX - 1] == '#') && (strlen(dialno[accountid]) >= DECT_STAR_CODE_DIGITS_MAX))
		{
			/* load star code table from config file, once */
  #ifndef NO_STAR_CODE
			if (!scodes_loaded){
				sc_fileapi_LoadStarCodes(dect_star_code_table);
				scodes_loaded = 1;
			}
  #endif
  
			/* check if after a star code a number exists (used for frwd-trnsfr-etc) */
			/* If exists, separate star code with number and process it */
			if ((strlen(dialno[accountid]) > DECT_STAR_CODE_DIGITS_MAX) && (dialno[accountid][strlen(dialno[accountid]) - 1] == '#'))
			{
				needs_number_info[accountid] = 1;
				/* get star code */
				tmp_strcode = strtok(key_seq, "#");
				/* get number after star code */
				tmp_dialno = strtok(NULL, "#");
				strcpy(dialno[accountid], tmp_dialno);
				/* copy to leave '*' behind */
				strcpy(star_code[accountid], &tmp_strcode[1]);
				/* change all '*' to '.' */
				for (i=0; i<strlen(dialno[accountid]); i++)
					if (dialno[accountid][i] == '*') dialno[accountid][i] = '.';	
				goto process_star_code;
			}
			else
			{
				/* Current star code has the form *xx#. We move first three one place to the left */
				for (i=0; i<DECT_STAR_CODE_DIGITS_MAX-1; i++)
					dialno[accountid][i] = dialno[accountid][i+1];
				/* Set third digit to terminate char */ 
				dialno[accountid][DECT_STAR_CODE_DIGITS_MAX-2]='\0';			
				strcpy(star_code[accountid], dialno[accountid]);					
				goto process_star_code;
			}
		}
		else
		{
			/* If end of string */
			if (dialno[accountid][key_seq_indx[accountid]] == '#') 
			{
				dialno[accountid][key_seq_indx[accountid]] = '\0';
				if (needs_number_info[accountid] == 1)
				{
					/* this is needed here only because at _sc1445x_dect_terminal_outgoing_req is already inlcuded */
					for (i=0; i<strlen(dialno[accountid]); i++)
						if (dialno[accountid][i] == '*') dialno[accountid][i] = '.';	
					goto process_star_code;
				}
				else
					goto makenewcall;
			}
			key_seq_indx[accountid]++;
			i++;
			if (key_seq_indx[accountid] > DECT_MAX_KEY_SEQ)
				key_seq_indx[accountid] = 0;
		}
	}
/*
		else if ((dialno[accountid][0] == '*') && strlen(dialno[accountid] > DECT_STAR_CODE_DIGITS_MAX))
		{
			si_print(PRINT_LEVEL_ERR,"%s, Wrong star code length\n", __FUNCTION__);
			goto clear_and_exit;
		}
*/
	return 0;

process_star_code:
#ifndef NO_STAR_CODE
	dect_process_star_code((unsigned char)portid, (unsigned char)accountid, star_code[accountid], dialno[accountid]);
#endif
	if (needs_number_info[accountid] == 1)
		strcpy(star_code[accountid], dialno[accountid]);
	else star_code[accountid][0] = '\0';
	key_seq_indx[accountid] = 0;
	dialno[accountid][0] = '\0';
	return 0;

clear_and_exit:
	star_code[accountid][0] = '\0';
	key_seq_indx[accountid] = 0;
	dialno[accountid][0] = '\0';
	needs_number_info[accountid] = 0;
	return 0;

makenewcall:
	key_seq_indx[accountid] = 0;
	/* Find if there are 4 dots '.'. Then change first to @. e.g 200.10.4.0.100 becomes 200@10.4.0.100 */
	sc1445x_phoneapi_fix_calling_username((unsigned char *)dialno[accountid], (unsigned char *)dialno[accountid]);
	/* Make the new call */
	_sc1445x_dect_terminal_outgoing_req(attachedentity, (unsigned char *)dialno[accountid], portid, accountid, codec);
	dialno[accountid][0] = '\0';
	needs_number_info[accountid] = 0;
	star_code[accountid][0] = '\0';
	return 0;
#endif // SA_CVM
}


/* This function processes a star code after it has been parsed succesfully */
#ifndef NO_STAR_CODE
static void dect_process_star_code(unsigned char portid, unsigned char accountid, char *star_code, char *dialnumber)
{
  unsigned char idx;
 
  for(idx = 0; idx < STAR_CODES_NUMBER; idx++)
  {
		si_print(PRINT_LEVEL_DEBUG, "%s %d [%s] [%s] \n", __FUNCTION__, idx, dect_star_code_table[idx].code, star_code);

    if(!strncmp(star_code, dect_star_code_table[idx].code, strlen(dect_star_code_table[idx].code)))
    {
			dect_star_code_table[idx].star_code_func(portid, accountid, dialnumber);
      break;
    }    
  }

	for(idx = 0; idx < DECT_INTERN_STAR_CODES_NUMBER; idx++)
  {
		si_print(PRINT_LEVEL_DEBUG, "%s %d [%s] [%s] \n", __FUNCTION__, idx, dect_star_code_table[idx].code, star_code);

    if(!strncmp(star_code, dect_intern_star_code_table[idx].code, strlen(dect_intern_star_code_table[idx].code)))
    {
			dect_intern_star_code_table[idx].star_code_func(portid, accountid, (char*)&dect_intern_star_code_table[idx].code);
      break;
    }    
  }
}
#endif // NO_STAR_CODE


#ifndef NO_STAR_CODE
static void dect_unpark_call_scode(unsigned char portid, unsigned char accountid, void *dummy)
{
	si_print(PRINT_LEVEL_DEBUG, "[%s]: portid %d\n", __FUNCTION__, portid);
	sc1445x_dect_terminal_hold(CCFSM_ATTACHED_DECT, portid, accountid, 0);
}
#endif

#ifndef NO_STAR_CODE
static void dect_park_call_scode(unsigned char portid, unsigned char accountid, void *dummy)
{
	si_print(PRINT_LEVEL_DEBUG, "[%s]: portid %d\n", __FUNCTION__, portid);
	sc1445x_dect_terminal_hold(CCFSM_ATTACHED_DECT, portid, accountid, 1);
}
#endif

#ifndef NO_STAR_CODE
static void dect_add_to_conf_scode(unsigned char portid, unsigned char accountid, void *dummy)
{
	si_print(PRINT_LEVEL_DEBUG, "[%s]: portid %d\n", __FUNCTION__, portid);
	sc1445x_dect_terminal_hold(CCFSM_ATTACHED_DECT, portid, accountid, 2);
}
#endif

#ifndef NO_STAR_CODE
static void dect_blind_transfer_scode(unsigned char portid, unsigned char accountid, char *number)
{
	si_print(PRINT_LEVEL_DEBUG, "[%s]: portid %d\n", __FUNCTION__, portid);
	sc1445x_phone_call_info *pCurCall;
	pCurCall = sc1445x_phoneapi_find_call_per_state(CCFSM_STATE_TALK, (int)portid);

	if (needs_number_info[accountid] == 0)
	{
		needs_number_info[accountid] = 1;
		sc1445x_phoneapi_call_hold(pCurCall->callid);
	}
	else
	{
		si_print(PRINT_LEVEL_DEBUG, "trnsfr number [%s] \n", number);
		needs_number_info[accountid] = 0;
		sc1445x_dect_blind_transfer((int)portid, number);
	}
}
#endif

#ifndef NO_STAR_CODE
static void dect_callerID_block_scode(unsigned char portid, unsigned char accountid, void *dummy)
{
	si_print(PRINT_LEVEL_DEBUG, "[%s]\n", __FUNCTION__);

  sc_fileapi_LoadCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);
 	m_ConfigFile.m_CallSettings[accountid].hide_id = (char) 1;
  sc_fileapi_StoreCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);
  
	si_ua_read_nonCriticalsettings(&m_ConfigFile) ;
 	libUASetNonCriticalCoreValues(&m_ConfigFile);
}
#endif

#ifndef NO_STAR_CODE
static void dect_callerID_send_scode(unsigned char portid, unsigned char accountid, void *dummy)
{
	si_print(PRINT_LEVEL_DEBUG, "[%s]\n", __FUNCTION__);

  sc_fileapi_LoadCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);
 	m_ConfigFile.m_CallSettings[accountid].hide_id = (char) 0;
  sc_fileapi_StoreCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);
  
	si_ua_read_nonCriticalsettings(&m_ConfigFile) ;
 	libUASetNonCriticalCoreValues(&m_ConfigFile);
}
#endif

#ifndef NO_STAR_CODE
static void dect_call_waiting_off_scode(unsigned char portid, unsigned char accountid, void *dummy)
{
	si_print(PRINT_LEVEL_DEBUG, "[%s]\n", __FUNCTION__);

  sc_fileapi_LoadCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);
 	m_ConfigFile.m_CallSettings[accountid].call_waiting = (char) 0;
  sc_fileapi_StoreCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);
  
	si_ua_read_nonCriticalsettings(&m_ConfigFile) ;
 	libUASetNonCriticalCoreValues(&m_ConfigFile);
}
#endif

#ifndef NO_STAR_CODE
static void dect_call_waiting_on_scode(unsigned char portid, unsigned char accountid, void *dummy)
{
	si_print(PRINT_LEVEL_DEBUG, "[%s]\n", __FUNCTION__);

  sc_fileapi_LoadCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);
 	m_ConfigFile.m_CallSettings[accountid].call_waiting = (char) 1;
  sc_fileapi_StoreCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);
  
	si_ua_read_nonCriticalsettings(&m_ConfigFile) ;
 	libUASetNonCriticalCoreValues(&m_ConfigFile);
}
#endif

#ifndef NO_STAR_CODE
static void dect_call_forward_uncond_on_scode(unsigned char portid, unsigned char accountid, char *number)
{
	si_print(PRINT_LEVEL_DEBUG, "[%s]\n", __FUNCTION__);

	if (needs_number_info[accountid] == 0)
		needs_number_info[accountid] = 1;
	else
	{
		si_print(PRINT_LEVEL_DEBUG, "[%s] frwrd number: [%s]\n", __FUNCTION__, number);
		needs_number_info[accountid] = 0;
		sc_fileapi_LoadCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);
 		m_ConfigFile.m_CallSettings[accountid].call_forward_mode = (char) 3;
		strcpy(m_ConfigFile.m_CallSettings[accountid].call_forward_number, number);
		sc_fileapi_StoreCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);
  
		si_ua_read_nonCriticalsettings(&m_ConfigFile) ;
 		libUASetNonCriticalCoreValues( &m_ConfigFile);
	}
}
#endif

#ifndef NO_STAR_CODE
static void dect_call_forward_uncond_off_scode(unsigned char portid, unsigned char accountid, void *dummy)
{
	si_print(PRINT_LEVEL_DEBUG, "[%s]\n", __FUNCTION__);

	sc_fileapi_LoadCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);
	m_ConfigFile.m_CallSettings[accountid].call_forward_mode = (char) 0;
	sc_fileapi_StoreCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);

	si_ua_read_nonCriticalsettings(&m_ConfigFile) ;
	libUASetNonCriticalCoreValues( &m_ConfigFile);
}
#endif

#ifndef NO_STAR_CODE
static void dect_call_forward_busy_on_scode(unsigned char portid, unsigned char accountid, char *number)
{
	si_print(PRINT_LEVEL_DEBUG, "[%s]\n", __FUNCTION__);

	if (needs_number_info[accountid] == 0)
		needs_number_info[accountid] = 1;
	else
	{
		si_print(PRINT_LEVEL_DEBUG, "[%s] frwrd number: [%s]\n", __FUNCTION__, number);
		needs_number_info[accountid] = 0;
		sc_fileapi_LoadCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);
 		m_ConfigFile.m_CallSettings[accountid].call_forward_mode = (char) 2;
		strcpy(m_ConfigFile.m_CallSettings[accountid].call_forward_number, number);
		sc_fileapi_StoreCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);
  
		si_ua_read_nonCriticalsettings(&m_ConfigFile) ;
 		libUASetNonCriticalCoreValues( &m_ConfigFile);
	}
}
#endif

#ifndef NO_STAR_CODE
static void dect_call_forward_busy_off_scode(unsigned char portid, unsigned char accountid, void *dummy)
{
	si_print(PRINT_LEVEL_DEBUG, "[%s]\n", __FUNCTION__);

	sc_fileapi_LoadCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);
	m_ConfigFile.m_CallSettings[accountid].call_forward_mode = (char) 0;
	sc_fileapi_StoreCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);

	si_ua_read_nonCriticalsettings(&m_ConfigFile) ;
	libUASetNonCriticalCoreValues( &m_ConfigFile);
}
#endif

#ifndef NO_STAR_CODE
static void dect_call_forward_delayed_on_scode(unsigned char portid, unsigned char accountid, char *number)
{
	si_print(PRINT_LEVEL_DEBUG, "[%s]\n", __FUNCTION__);

	if (needs_number_info[accountid] == 0){
		needs_number_info[accountid] = 1;
	}
	else
	{
		si_print(PRINT_LEVEL_DEBUG, "[%s] frwrd number: [%s]\n", __FUNCTION__, number);
		needs_number_info[accountid] = 0;
		sc_fileapi_LoadCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);
 		m_ConfigFile.m_CallSettings[accountid].call_forward_mode = (char) 1;
		strcpy(m_ConfigFile.m_CallSettings[accountid].call_forward_number, number);
		sc_fileapi_StoreCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);
  
		si_ua_read_nonCriticalsettings(&m_ConfigFile) ;
 		libUASetNonCriticalCoreValues(&m_ConfigFile);
	}
}
#endif

#ifndef NO_STAR_CODE
static void dect_call_forward_delayed_off_scode(unsigned char portid, unsigned char accountid, void *dummy)
{
	si_print(PRINT_LEVEL_DEBUG, "[%s]\n", __FUNCTION__);

	sc_fileapi_LoadCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);
	m_ConfigFile.m_CallSettings[accountid].call_forward_mode = (char) 0;
	sc_fileapi_StoreCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);

	si_ua_read_nonCriticalsettings(&m_ConfigFile) ;
	libUASetNonCriticalCoreValues(&m_ConfigFile);
}
#endif

#ifndef NO_STAR_CODE
static void dect_internal_call_scode(unsigned char initiator_portid, unsigned char initiator_accountid, char *to_accountid)
{
	si_print(PRINT_LEVEL_DEBUG, "[%s]\n", __FUNCTION__);

	if (needs_number_info[initiator_accountid] == 0)
	{
		needs_number_info[initiator_accountid] = 1;
	}
	else
	{
		needs_number_info[initiator_accountid] = 0;
		sc1445x_dect_terminal_internal_connect_lines((unsigned short)initiator_portid, (unsigned short)initiator_accountid, to_accountid);
	}
}
#endif

#ifndef NO_STAR_CODE
static void dect_dnd_scode_off(unsigned char portid, unsigned char accountid, void *dummy)
{
	si_print(PRINT_LEVEL_DEBUG, "[%s] accountid %d\n", __FUNCTION__, accountid);

  sc_fileapi_LoadCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);
 	m_ConfigFile.m_CallSettings[accountid].DND = (char) 0;
  sc_fileapi_StoreCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);
  
	si_ua_read_nonCriticalsettings(&m_ConfigFile) ;
 	libUASetNonCriticalCoreValues(&m_ConfigFile);
}
#endif

#ifndef NO_STAR_CODE
static void dect_dnd_scode_on(unsigned char portid, unsigned char accountid, void *dummy)
{
	si_print(PRINT_LEVEL_DEBUG, "[%s] accountid %d\n", __FUNCTION__, accountid);

  sc_fileapi_LoadCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);
 	m_ConfigFile.m_CallSettings[accountid].DND = (char) 1;
  sc_fileapi_StoreCallSettings(&m_ConfigFile.m_CallSettings[accountid], accountid + 1);
  
	si_ua_read_nonCriticalsettings(&m_ConfigFile) ;
 	libUASetNonCriticalCoreValues(&m_ConfigFile);
}
#endif

/* Select/change voice codec */
#ifndef NO_STAR_CODE
static void dect_select_codec_scode(unsigned char portid, unsigned char accountid, void* code)
{
  unsigned char star_code = atoi((char*)code);

  si_print(PRINT_LEVEL_DEBUG, "Portid: %d Codec Selection Req %s \n", portid, dect_intern_star_code_table[star_code].name);
	sc1445x_dect_change_configuration_codec_selection((char*)code , portid);
}
#endif


#ifndef SA_CVM
static void sc1445x_dect_blind_transfer(unsigned char portid, char* number)
{
	int i=0;
	sc1445x_phone_call_info *pCurCall;
	pCurCall = sc1445x_phoneapi_find_call_per_state(CCFSM_STATE_HOLD, (int)portid);
	if (pCurCall)
	{
	 	while (i < strlen(number)){
			if (number[i]==(unsigned char)'*') number[i]='.';	
 			i++;
		} 
		si_print(PRINT_LEVEL_DEBUG,"[%s], trnsfr to: [%s]\n", __FUNCTION__, number);
 		sc1445x_phoneapi_blindtransfer(pCurCall->callid, number);
	} 
}
#endif // SA_CVM


///////////////////////////////////////////////////////////////////////////////// 
///////////////////////////////////////////////////////////////////////////////// 
// Function: sc1445x_dect_terminal_hold
// If one active call in talk State , put it on hold
// If one active call in hold State , resume it
// If one active call is in talk state and the second in hold mode then put them in conference mode 
///////////////////////////////////////////////////////////////////////////////// 
#ifndef SA_CVM
static void sc1445x_dect_terminal_hold(ccfsm_attached_entity attachedentity, unsigned char portid, unsigned char accountid, unsigned char hold_state)
{
	sc1445x_phone_call_info *pIncomingCall;
	sc1445x_phone_call_info *pCurrentCall;
 	sc1445x_phone_call_info *pCall1, *pCall2;

	int numOfCalls= 0;
 
	pIncomingCall = sc1445x_phoneapi_find_call_per_state(CCFSM_STATE_INCOMING, (int)portid);
	pCurrentCall = sc1445x_phoneapi_find_call((int)portid);
 	numOfCalls = sc1445x_phoneapi_find_numof_calls((int)portid);
	si_print(PRINT_LEVEL_DEBUG, "DECT HOLD REQ port=[%d] accountid=[%d], numOfCalls=[%d] \n", (int)portid, accountid, numOfCalls);

	switch (numOfCalls)
	{
		case 0:
			/* check if account is having an internal call */
			if ((internalDECTCallTable[accountid] != -1) && (hold_state == 1))
			{
				sc1445x_amb_internal_disconnect_line((int)portid);
				dect_callprogress_tone_handling(MCU_TONE_DL_BT, SC1445x_AE_TONE_INVALID, SC1445x_AE_TONE_INVALID, 0, (int)portid);
			}
			else if ((internalDECTCallTable[accountid] != -1) && (hold_state == 0))
			{
				dect_callprogress_tone_handling(MCU_TONE_NA_BT, SC1445x_AE_TONE_INVALID, SC1445x_AE_TONE_INVALID, 0, (int)portid);
				sc1445x_amb_internal_connect_lines((unsigned short)portid, (unsigned short)internalDECTCallTable[accountid]);
			}
			
 			return ;
		case 1:
      if (pIncomingCall) return;
			si_print(PRINT_LEVEL_DEBUG, "\n\nENTER CONFERENCE 0\n\n");
 			if (hold_state == 1)
			{
				if (pCurrentCall && ((pCurrentCall->state  == CCFSM_STATE_TALK) || (pCurrentCall->state == CCFSM_STATE_HOLDING))){
				{
					int ret=sc1445x_phoneapi_call_hold(pCurrentCall->callid);
					if (!ret)
						dect_callprogress_tone_handling(MCU_TONE_DL_BT, SC1445x_AE_TONE_INVALID, SC1445x_AE_TONE_INVALID, 0, (int)portid);
				}
			}
			}else if (hold_state == 0)
			{
				if (pCurrentCall && ((pCurrentCall->state  == CCFSM_STATE_HOLD) || (pCurrentCall->state == CCFSM_STATE_HELD) || (pCurrentCall->state == CCFSM_STATE_INACTIVE)))
				{
					dect_callprogress_tone_handling(MCU_TONE_NA_BT, SC1445x_AE_TONE_INVALID, SC1445x_AE_TONE_INVALID, 0, (int)portid);
 					sc1445x_phoneapi_call_resume(pCurrentCall->callid);
				}
			}else if (hold_state == 2)/* conference one internal and one external */
			{
					si_print(PRINT_LEVEL_DEBUG, "\n\nENTER CONFERENCE 1\n\n");
				pCall1= sc1445x_phoneapi_find_call_per_state(CCFSM_STATE_TALK, (int)portid);
				pCall2= sc1445x_phoneapi_find_call_per_state(CCFSM_STATE_HOLD, (int)portid);
				if (pCall1)
					sc1445x_phoneapi_call_resume(pCall1->callid);	
				else if (pCall2)
					sc1445x_phoneapi_call_resume(pCall2->callid);	

				if (internalDECTCallTable[accountid] != -1)
				{
					dect_callprogress_tone_handling(MCU_TONE_NA_BT, SC1445x_AE_TONE_INVALID, SC1445x_AE_TONE_INVALID, 0, (int)portid);
					sc1445x_amb_internal_connect_lines((unsigned short)portid, (unsigned short)internalDECTCallTable[accountid]);
				}
			}
  		break;
		case 2:
			si_print(PRINT_LEVEL_DEBUG, "\n\nENTER CONFERENCE \n\n");
			if ((!(pIncomingCall)) && (hold_state == 2))
      {
				pCall1= sc1445x_phoneapi_find_call_per_state(CCFSM_STATE_TALK, (int)portid);
				pCall2= sc1445x_phoneapi_find_call_per_state(CCFSM_STATE_HOLD, (int)portid);

				if (internalDECTCallTable[accountid] != -1)
				{
					dect_callprogress_tone_handling(MCU_TONE_NA_BT, SC1445x_AE_TONE_INVALID, SC1445x_AE_TONE_INVALID, 0, (int)portid);
					sc1445x_amb_internal_connect_lines((unsigned short)portid, (unsigned short)internalDECTCallTable[accountid]);
				}

				if (pCall1 && pCall2){
          dect_callprogress_tone_handling(MCU_TONE_NA_BT, SC1445x_AE_TONE_INVALID, SC1445x_AE_TONE_INVALID, 0, pCall2->portid);
					sc1445x_phoneapi_call_resume(pCall2->callid);	
				}
  		}
  		break;
		default:
			break;
 	}
}
#endif // SA_CVM


#ifndef SA_CVM
#define CURRENT_REG_DEVICES 3
void sc1445x_dect_terminal_terminate_allcalls(ccfsm_attached_entity attachedentity, int port, int accountid)
{
	sc1445x_phone_call_info *pCallInfo;
	sc1445x_phone_active_calls_list *pCallItem;
  int numofcalls;
	int i;
	static int noofhandsets=0;
	sc1445x_phone_call_info *pCall_br;
	
	/* If broadcast is notified by the cvmcon that a handset is not active. Broadcast call should exist only at incoming state*/
	pCall_br = sc1445x_phoneapi_find_call_per_accountid(CCFSM_DECT_BROADCAST_ID);

	if ((pCall_br) && (!(noofhandsets == CURRENT_REG_DEVICES)) && (pCall_br->state==CCFSM_STATE_INCOMING)) 
	{
		noofhandsets ++;	
	}
	if (noofhandsets == CURRENT_REG_DEVICES){
		noofhandsets=0;
		return;
	}
	// if call was internal call, play dial tone and clear the table
	if (internalDECTCallTable[accountid] != -1)
	{
		/* We came here due to a hook-off from one handset */
		/* We have to find the account and port of the second handset */
		/* The first and second handset ports are stored at internalDECTCallTable[accountid] during connect */

		/* find the accountid of the second device */
		for (i=0; i<MAX_ALLOWED_DECT_HANDSETS; i++)
		{
			/* the second account is found here */
			/* they may have the same port during ringing state because second port is not known yet */
			if ((internalDECTCallTable[i] == port) && (i != accountid))
			{
				// is needed to stop ringing if cancelled during ringing state. check if other call exists
				numofcalls = sc1445x_phoneapi_find_numof_calls_peraccountid(i);
				if (numofcalls==0)
				{
					HandleSendReleaseReq((unsigned char)i);
					dect_callprogress_tone_handling(MCU_TONE_DL_BT, SC1445x_AE_TONE_INVALID, SC1445x_AE_TONE_INVALID, 0, internalDECTCallTable[i]);
				}
				dect_terminal_audio_peripheral_status[i] = 0;
				internalDECTCallTable[i] = -1;
				si_print(PRINT_LEVEL_DEBUG, "!!!!! Release account1 %d account2 %d numofcalls %d \n", i, accountid, numofcalls);
				break;
			}
		}

		dect_terminal_audio_peripheral_status[accountid] = 0;
//	dect_callprogress_tone_handling(MCU_TONE_DL_BT, SC1445x_AE_TONE_INVALID, SC1445x_AE_TONE_INVALID, 0, port);
//	dect_callprogress_tone_handling(MCU_TONE_DL_BT, SC1445x_AE_TONE_INVALID, SC1445x_AE_TONE_INVALID, 0, internalDECTCallTable[accountid]);
		internalDECTCallTable[accountid] = -1;
		sc1445x_amb_internal_disconnect_line(port);
		return ;
	}

	numofcalls = sc1445x_phoneapi_find_numof_calls_peraccountid(accountid);
	if (numofcalls==0) return ;
	pCallInfo = sc1445x_phoneapi_find_call_per_accountid(accountid);
	if (pCallInfo==NULL) return ;

 	// verify that there is no call in talking state or in any other active state (eg sending a request)
 	for(pCallItem = m_active_calls_list; pCallItem<(m_active_calls_list+(sizeof(&m_active_calls_list))); pCallItem++)  
	{
		if ((pCallItem->call_info.accountid==accountid) && (pCallItem->call_info.state !=CCFSM_STATE_IDLE))
		{
  		sc1445x_phoneapi_call_terminate(pCallItem->call_info.callid, attachedentity);
			numofcalls--;
		}
		if (numofcalls==0) break;
 	}
}
#endif // SA_CVM


#ifndef SA_CVM
static void sc1445x_dect_terminal_internal_connect_lines(unsigned short initiator_portid, unsigned short initiator_accountid, char *to_accountid)
{
	/* store account id to call */
	int to_number;
	char initiator_number[4];

	to_number = atoi(to_accountid);
	sprintf(initiator_number, "%d", initiator_accountid);
	/* second handset ringing */
	sc1445x_dect_terminal_start_ring(DECT_WIDEBAND_CODEC, 0, to_number, initiator_number); //SOS Codec should be found here..... to_port not known yet so portid=0
	/* first handset dial tone */
	dect_callprogress_tone_handling(MCU_TONE_INVALID, SC1445x_AE_TONE_INVALID, SC1445x_AE_TONE_F440, 0, initiator_portid); 

	/* store the initiator_portid to the second account */
	/* the second portid is not yet known */
	/* so we set it to a default 1 just not to be -1 */
	/* The last will change at hook off, when the second handset will answer the call */
	internalDECTCallTable[to_number] = initiator_portid;
	internalDECTCallTable[initiator_accountid] = initiator_portid;

	//sc1445x_phoneapi_create_internalcall(CCFSM_ATTACHED_DECT, to_accountid, (int)initiator_portid, (int)initiator_accountid, 0, 0);//codec ? reference = 0
}
#endif // SA_CVM
 

void sc1445x_dect_terminal_ring(ring_mode mode, int port, int accountid, int codec, int numofcalls, char* displayname)
{
#ifdef SA_CVM
	printf( "TODO: sc1445x_dect_terminal_ring\n" );
#else
	// at a 2nd broadcast call we send ring again
	// if (accountid==CCFSM_DECT_BROADCAST_ID) mode=1;
  if (mode==1){

		sc1445x_dect_terminal_start_ring(DECT_WIDEBAND_CODEC, port, accountid, displayname);

  }else if(mode==2){
      
    dect_callprogress_tone_handling(MCU_TONE_NA_BT, SC1445x_AE_TONE_INVALID, SC1445x_AE_TONE_INVALID, 0, port);
  }
	else
		sc1445x_dect_terminal_stop_ring(port, accountid);
#endif // SA_CVM
}

#ifndef SA_CVM
void sc1445x_dect_terminal_start_ring(int codec, int port, int accountid, char* displayname)
{
	int numOfCalls = sc1445x_phoneapi_find_numof_calls_peraccountid(accountid);
  sc1445x_phone_call_info * pCall;

  pCall = sc1445x_phoneapi_find_call_per_state_ofaccountid(CCFSM_STATE_INCOMING, accountid);
	si_print(PRINT_LEVEL_DEBUG, "[%s], pCall->reference %d port %d accountid %d \n", __FUNCTION__, pCall->reference, port, accountid);
  if ((numOfCalls<2) && (internalDECTCallTable[accountid] == -1) && (!(accountid==CCFSM_DECT_BROADCAST_ID))){
  
    pCall->reference = accountid;
    if (pCall->accountid == CCFSM_DECT_BROADCAST_ID){
       pCall->reference = -1;//Broacast ID to DECT
    }
	 	if(HandleSendConnectReq(codec, pCall->reference, displayname, displayname) == -1)
		{
			sc1445x_dect_terminal_terminate_allcalls(CCFSM_ATTACHED_DECT, port, accountid);
		}

	}else if ((accountid==CCFSM_DECT_BROADCAST_ID) && (numOfCalls < 4)){
		//si_print
		pCall->reference = -1;//Broacast ID to DECT
		if(HandleSendConnectReq(codec, pCall->reference, displayname, displayname) == -1)
		{
			sc1445x_dect_terminal_terminate_allcalls(CCFSM_ATTACHED_DECT, port, accountid);
		}
	}

	else{
		si_print(PRINT_LEVEL_DEBUG, "[%s] \n", __FUNCTION__);
		dect_callprogress_tone_handling(MCU_TONE_INVALID, SC1445x_AE_TONE_INVALID, SC1445x_AE_TONE_F440, 0, port);
	}
}
#endif // SA_CVM
 
#ifndef SA_CVM
void sc1445x_dect_terminal_stop_ring(int port, int accountid)
{
	dect_callprogress_tone_handling(MCU_TONE_NA_BT, SC1445x_AE_TONE_INVALID, SC1445x_AE_TONE_INVALID, 0, port);
	//dect_stop_ringing(port);
}
#endif  

void sc1445x_dect_terminate_call(int port, int callid, int numOfCalls, int term_reason)
{
#ifdef SA_CVM
	printf( "TODO: sc1445x_dect_terminate_call\n" );
#else
  int tone=1;
	ccfsm_call_states state;

  sc1445x_phone_call_info *pCallInfo=NULL;
	sc1445x_phone_call_info *pCall_br=NULL;

	/* If incoming broadcast is released */
	pCall_br = sc1445x_phoneapi_find_call_per_state_ofaccountid(CCFSM_STATE_INCOMING, CCFSM_DECT_BROADCAST_ID);
	pCallInfo = sc1445x_phoneapi_find_call_withID(callid);
	if(pCall_br) 
	{
		if ((pCall_br->accountid == CCFSM_DECT_BROADCAST_ID) && (pCallInfo->accountid == CCFSM_DECT_BROADCAST_ID))
		{
			pCall_br->reference = -1;//Broacast ID to DECT
			HandleSendReleaseReq((unsigned char)pCall_br->reference);
			return;
		}
	}

  if ((numOfCalls<2) && (internalDECTCallTable[pCallInfo->accountid]==-1))
  {
    dect_callprogress_tone_handling(MCU_TONE_INVALID, SC1445x_AE_TONE_INVALID, SC1445x_AE_TONE_INVALID, term_reason, port);
		state = sc1445x_phoneapi_find_state_per_callid(callid);
	
		if (sc1445x_phoneapi_find_call_per_state_ofportid(CCFSM_STATE_INCOMING, port)){
			pCallInfo = sc1445x_phoneapi_find_call(port);
			if (pCallInfo->accountid == CCFSM_DECT_BROADCAST_ID){
				pCallInfo->reference = -1;//Broacast ID to DECT
			}
			HandleSendReleaseReq((unsigned char)pCallInfo->reference);
		}

  }else if ((numOfCalls>1)||(internalDECTCallTable[pCallInfo->accountid]!=-1))
  {
    pCallInfo = sc1445x_phoneapi_find_call(port);
    while (numOfCalls && pCallInfo)
    {
     if (pCallInfo->callid != callid)
     {
      if ((pCallInfo->state == CCFSM_STATE_TALK) || (internalDECTCallTable[pCallInfo->accountid]!=-1))
      {
        tone = 1;
        break;
      }
			if ((pCallInfo->state == CCFSM_STATE_HOLD) || (pCallInfo->state == CCFSM_STATE_INACTIVE)){
       tone = 2;
			}
     }

     pCallInfo = sc1445x_phoneapi_get_next_call(pCallInfo,port);
     numOfCalls--;
    }
 
   if (tone==1)
   {
    dect_callprogress_tone_handling(MCU_TONE_INVALID, SC1445x_AE_TONE_F440, SC1445x_AE_TONE_INVALID, 0, port);
   }
   else
   {
		dect_callprogress_tone_handling(MCU_TONE_DL_BT, SC1445x_AE_TONE_INVALID, SC1445x_AE_TONE_INVALID, 0, port);
   }
  }
#endif
}

#ifndef SA_CVM
static void sc1445x_dect_park_call(unsigned char portid, unsigned char accountid, int toline)
{
	sc1445x_phone_call_info *pCurCall;

	pCurCall = sc1445x_phoneapi_find_call((int)portid);	
	
	if ((pCurCall->state==CCFSM_STATE_TALK) || (pCurCall->state==CCFSM_STATE_HELD)  )
	{ 
		sc1445x_phoneapi_call_hold(pCurCall->callid);
		pCurCall->parkid = toline;
    dect_callprogress_tone_handling(MCU_TONE_DL_BT, SC1445x_AE_TONE_INVALID, SC1445x_AE_TONE_INVALID, 0, (int)portid);
  }
}
#endif // SA_CVM
 

#ifndef SA_CVM
static void sc1445x_dect_unpark_call(unsigned char portid, unsigned char accountid, int toline)
{
	int numofcalls;
	int found = 0;
	sc1445x_phone_call_info *pCurCall;

	pCurCall = sc1445x_phoneapi_find_call((int)portid);		
 	numofcalls = sc1445x_phoneapi_find_numof_calls((int)portid);
 
	while (pCurCall)
	{
		if (toline && pCurCall->parkid == toline) {
			found=1;
 			break;
		}
 		pCurCall = sc1445x_phoneapi_get_next_call(pCurCall, (int)portid);
   	}
	if (!found) return ;
	
	sc1445x_phoneapi_call_resume(pCurCall->callid);
	pCurCall->parkid = 0;
  dect_callprogress_tone_handling(MCU_TONE_NA_BT, SC1445x_AE_TONE_INVALID, SC1445x_AE_TONE_INVALID, 0, (int)portid);
}
#endif // SA_CVM


int sc1445x_dect_change_rfpi(unsigned char *MACAddress)
{
#ifdef SET_RFPI
	unsigned short emc_rfpi=0;
	unsigned short fpn_rfpi=0;

	emc_rfpi=MACAddress[2]<<8 | MACAddress[3];
	fpn_rfpi=MACAddress[4]<<8 | MACAddress[5];

	si_print(PRINT_LEVEL_INFO, "\n emc_rfpi %x, fpn_rfpi %x \n", emc_rfpi, fpn_rfpi);

  SET_CC_RFPI_EMC(CC_RFPI, emc_rfpi);
  SET_CC_RFPI_FPN(CC_RFPI, fpn_rfpi<<1);
#endif
	
	return 0;
}

#ifndef SA_CVM
int sc1445x_dect_set_pcm_line_type(int line, int codec)
{
	int err = 0 ;
  snd_hwdep_t *hwdephandle;
  sc1445x_line_type_t line_type;

#ifndef USE_PCM
  return 1; //NO_MMI_USE_PCM is included in all designs except VoIP and DECT headset
#endif

  if ((line<0) || (line > MAX_ADPCM_PORTS_SUPPORTED-1)) return -1;

	if ((err = snd_hwdep_open(&hwdephandle, "hw" , O_RDWR)) < 0) {
  		si_print(PRINT_LEVEL_ERR, "[%s] couldn't open alsa driver %s\n", __FUNCTION__ , snd_strerror(err) );
  		return err;
	}

  line_type.line = (unsigned short) line;

//CVM with ATA
#ifdef CONFIG_ATA_1_FXS_NO_FXO_1_CVM 
  if (line>0){// line 0 is for ATA only and should be given by the ata handler
    if(codec == 2){//Wideband 
      line_type.type = SC1445x_AE_LINE_TYPE_CVM_DECT_WIDE_8KHZ_ULAW;
    }else{ //Narrowband
      line_type.type = SC1445x_AE_LINE_TYPE_CVM_DECT_NARROW; 
    }
  }else return -1;
#elif defined CONFIG_ATA_2_FXS_NO_FXO_1_CVM
 if (line>1){// line 0 and 1 are for ATA only and should be given by the ata handler
    if(codec == 2)//Wideband 
      line_type.type = SC1445x_AE_LINE_TYPE_CVM_DECT_WIDE_8KHZ_ULAW;
    else //Narrowband
      line_type.type = SC1445x_AE_LINE_TYPE_CVM_DECT_NARROW;  
 }else return -1;
//CVM only
#elif (!(defined CONFIG_SC1445x_LEGERITY_890_SUPPORT) && (defined CONFIG_CVM480_DECT_SUPPORT))
  if(codec == 2)//Wideband 
    line_type.type = SC1445x_AE_LINE_TYPE_CVM_DECT_WIDE_16KHZ;
  else //Narrowband
    line_type.type = SC1445x_AE_LINE_TYPE_CVM_DECT_NARROW;
//Natalie only or Natalie with ATA
#elif (defined CONFIG_LMX4180_DECT_SUPPORT)
  if(codec == 2){//Wideband 
    line_type.type = SC1445x_AE_LINE_TYPE_NATIVE_DECT_WIDE;
  }else{ //Narrowband
    line_type.type = SC1445x_AE_LINE_TYPE_NATIVE_DECT_NARROW;
  }
#endif

  if ((err = snd_hwdep_ioctl(hwdephandle, SNDRV_SC1445x_SET_LINE_TYPE, &line_type ))<0){
	  si_print(PRINT_LEVEL_ERR, "[%s] Could not set pcm port [%d] with line type [%d] : %s\n", __FUNCTION__, line, line_type.type, snd_strerror( err ) );
	  snd_hwdep_close(hwdephandle);
	  return err ;
  }else {
    si_print(PRINT_LEVEL_INFO,  "[%s] PCM [%d] line type successfully set to [%d] \n", __FUNCTION__, line, line_type.type );
  }
   return err;
}
#endif // SA_CVM

#ifndef NO_STAR_CODE
void sc1445x_dect_change_configuration_codec_selection(char* codec ,int portid)
{
  si_ua_read_nonCriticalsettings(&m_ConfigFile) ;
 	libUASetNonCriticalCoreValues(&m_ConfigFile);
}	
#endif

#ifndef SA_CVM
static void dect_callprogress_tone_handling(sc1445x_mcu_tone_bmp std_tone, sc1445x_ae_tone dynamic_tone, sc1445x_ae_tone periodic_tone, int reason, int port)
{
  si_print(PRINT_LEVEL_DEBUG, "[%s] port=%d\n", __FUNCTION__, port);
  // Check and set playing tone. If any tone is playing, stop it 
  // in order to switch it to a new one
  if(sc1445x_amb_status_callprogress_tone(port))
    sc1445x_mcu_set_tone(MCU_TONE_NA_BT, 0,(unsigned short)port);

  if ((std_tone == MCU_TONE_INVALID) && (dynamic_tone == SC1445x_AE_TONE_INVALID) && (!(periodic_tone == SC1445x_AE_TONE_INVALID))){
    sc1445x_amb_start_periodic_tone(periodic_tone, (unsigned short)port);
    si_print(PRINT_LEVEL_DEBUG, "\n PLAY PERIODIC TONE \n\n");
  }else if ((!(std_tone == MCU_TONE_INVALID)) && (dynamic_tone == SC1445x_AE_TONE_INVALID) && (periodic_tone == SC1445x_AE_TONE_INVALID)){
    sc1445x_mcu_set_tone(std_tone, 0,(unsigned short)port);
    si_print(PRINT_LEVEL_DEBUG, "\n PLAY STANDARD TONE \n\n");
  }else if ((std_tone == MCU_TONE_INVALID) && (!(dynamic_tone == SC1445x_AE_TONE_INVALID)) && (periodic_tone == SC1445x_AE_TONE_INVALID)){
    sc1445x_amb_start_dynamic_tone(dynamic_tone, (unsigned short)port);
    si_print(PRINT_LEVEL_DEBUG, "\n PLAY DYNAMIC TONE \n\n");
  }else if ((std_tone == MCU_TONE_INVALID) && (dynamic_tone == SC1445x_AE_TONE_INVALID) && (periodic_tone == SC1445x_AE_TONE_INVALID))
  {
    si_print(PRINT_LEVEL_DEBUG, "\n PLAY SPECIAL TONE \n\n");
    switch (reason)
    { 

  /* PROVISIONAL 1xx */
  /* Trying */
    case 100:
      break;

  /* SUCCESFUL 2xx */
  /* Ok - Normal call release*/
    case 200:
      sc1445x_mcu_set_tone(MCU_TONE_DL_BT, 0,(unsigned short)port);
      break;

  /* REDIRECTION 3xx */
  /* Multiple Choices */
    case 300:
      break;
  /* Moved Permantly */
    case 301:
      break;
  /* Moved Temporarily */
    case 302:
      break;
  /* Use proxy */
    case 305:
      break;
  /* Alternative Service */
    case 380:
      break;

  /* REQUEST FAILURE 4xx */
    case 400:
    case 401:
    case 402:
    case 403:
    case 404:
    case 405:
    case 406:
    case 407:
    case 408:
    case 410:
    case 413:
    case 414:
    case 415:
    case 416:
    case 420:
    case 421:
    case 423:
    case 480:
    case 481:
    case 482:
    case 483:
    case 484:
    case 485:
  /* Busy here */
    case 486:
    case 487:
    case 488:
    case 491:
    case 493:
       sc1445x_mcu_set_tone(MCU_TONE_CNG_BT, 0,(unsigned short)port);
     break;
    case 500:

  /* SERVER FAILURE 5xx */
    case 501:
    case 502:
    case 503:
    case 504:
    case 505:
    case 513:
      sc1445x_mcu_set_tone(MCU_TONE_CNG_BT, 0,(unsigned short)port);
  /* GLOBAL FAILURES 6xx */
    case 600: 
    case 603: 
    case 604: 
    case 606: 
      sc1445x_mcu_set_tone(MCU_TONE_CNG_BT, 0,(unsigned short)port);
    default:
      break;
     }
    }
  else si_print(PRINT_LEVEL_INFO, "Please select a single type of tone \n");
}
#endif // !SA_CVM


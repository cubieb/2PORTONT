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
 * File:		si_ua_instantmessage.h
 * Purpose:
 * Created:		Mar 2009
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_UA_INSTANTMESSAGE_H
#define SI_UA_INSTANTMESSAGE_H

#include "si_ua_init.h"
#include "../CCFSM/ccfsm_api.h"

/*
typedef enum
{
	STATUS_OPN,
	STATUS_CLS
}pr_status_basic;

typedef enum
{
	STATUS_ONLINE,
	STATUS_AWAY,
	STATUS_DND
}pr_note;*/

int sc1445x_im_send ( char* text, int accountID,char*toURI, int IPCall);
int sc1445x_im_recv ( 	eXosip_event_t *pevent);


#define PR_PUBLISH_EXPIRE "300"
#define PR_SUB_EXPIRE 120

int sc1445x_pr_send ( int accountID, pr_status_basic pres, pr_note note, int expires);
int si_pr_subscribe(ccfsm_attached_entity attachedentity, int accountID, char *subAccount, char *subRealm, int expires  );
int sc1445x_pr_recv (eXosip_event_t *pevent);
int si_pr_unsubscribe(ccfsm_attached_entity attachedentity,int sid);

#endif //SI_UA_INSTANTMESSAGE_H

		
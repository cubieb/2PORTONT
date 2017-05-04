#ifndef __DIALPLAN_HELP_H__
#define __DIALPLAN_HELP_H__

#include "dialplan.h"

#include "linphonecore.h"

/* Initialize dial plan module */
extern void Init_DialPlan( int chid, const dpInitParam_t *pdpInitParam);

/* Uninitialize dial plan module */
extern void Uninit_DialPlan( int chid );

/* ======================================================
 * Dial plan string catenation and call out help functions
 * (implement in dialplanhelp.c)
 * ====================================================== */
/* catenate string for INVITE */
extern void CallInviteByDialPlan(LinphoneCore *lc, guint32 ssid, char *src_ptr);

/* catenate string for REFER (call transfer) */
extern void CallTransferByDialPlan( LinphoneCore *lc, guint32 ssid );

/* catenate string for MOVE (call forward) */
extern void CallForwardByDialPlan( const LinphoneCore *lc,
							const unsigned char *pUserUrl,
							unsigned char *pSipUrl,
							const unsigned char *pszDot );

/* catenate string for hotline's INVITE (It will NOT do speed-dial translation.) */
extern void HotlineCallInviteByDialPlan(LinphoneCore *lc, guint32 ssid, char *src_ptr,
								 const char *pszDirectDial );

extern void CallTestByDialPlan(LinphoneCore *lc, guint32 ssid, char *src_ptr);

/* Try to test whether match dial, and do call invite if match. */
extern int DialPlanTryCallInvite( LinphoneCore *lc, guint32 ssid, char *src_ptr);

/* Try to test whether match dial, and do call transfer if match. */
extern int DialPlanTryCallTransfer( LinphoneCore *lc, guint32 ssid );

#endif /* __DIALPLAN_HELP_H__ */


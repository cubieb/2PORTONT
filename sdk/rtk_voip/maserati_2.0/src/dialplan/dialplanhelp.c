#include <string.h>
#include "dialplanhelp.h"
#include "abbreviated.h"

static dialplan_vars_t dialplan_internal_vars[ MAX_VOIP_PORTS ] = {
	{ 0, 0, 0, { 0 }, { 0 }, { 0 }, 0 },
#if MAX_VOIP_PORTS == 1
#elif MAX_VOIP_PORTS == 2 
	{ 0, 0, 0, { 0 }, { 0 }, { 0 }, 0 },
#else
	{0}
// #error "give more initial variables."
#endif
};


void Init_DialPlan( int chid, const dpInitParam_t *pdpInitParam)
{	
	/* check chid */
	if( chid >= g_MaxVoIPPorts ) {
		printf( "Invalid chid for dial plan initialization!\n" );
		return;
	}
	
	InitializeDialPlan( &dialplan_internal_vars[ chid ], pdpInitParam );
}

void Uninit_DialPlan( int chid )
{
	/* check chid */
	if( chid >= g_MaxVoIPPorts ) {
		printf( "Invalid chid for dial plan initialization!\n" );
		return;
	}
	
	UninitializeDialPlan( &dialplan_internal_vars[ chid ] );
}


/*
 * This file contains two parts of functions.
 * - One of them is catenate string functions, which helps to generate 
 *   string for SIP. Its output looks like 'sip:xxxxxx'.
 * - The other is try to dial functions, which tests whether match dial 
 *   plan. If yes, do call out action.
 */

/* ======================================================
 * Catenate string
 * ====================================================== */
extern int create_sip_url(const char *src_ptr, char *sip_url, const char chDot);
extern int IsNumericalString( const unsigned char *pszCheckNumericalString );
extern const unsigned char * speed_dial_check( int chid, 
										const gchar *pDialCode, 
										gchar *pSpeedDialCode );


/*______________________________________________________________________________
**  CatenateDialPlanString
**
**  descriptions: 
**  parameters:pUserUrl : origin dial string
               pSipUrl : point to output sting 
               pszDot
**  return:
**  note:
**____________________________________________________________________________*/


static void CatenateDialPlanString( const LinphoneCore *lc,
									const unsigned char *pUserUrl,
									unsigned char *pSipUrl,
									const unsigned char *pszDot )
{
	/* NOTE: pUserUrl and pSipUrl have to be DIFFERENT. */

	int idx_start, idx_end;
	unsigned char szDial[ 256 ];
	const unsigned char *pszUserUrlAddPrefixDial;
	const voipCfgPortParam_t *voip_ptr;

	/* check chid */
	if( lc ->chid >= g_MaxVoIPPorts ) {
		printf( "Invalid chid for dial plan initialization!\n" );
		return;
	}
	
	if( pUserUrl == pSipUrl )
		printf( "UNEXPECTED: Src and dest are the same.\n" );

	/* add "sip:" prefix ? */
	if( memcmp( pUserUrl, "sip:", 4 ) == 0 ) {
		/* use sip url directly */
		strcpy( pSipUrl, pUserUrl );
		goto label_catenate_dial_plan_done;
	} else if( strstr( pUserUrl, pszDot ) == NULL ) {
		/* use proxy call --> continue to do follows */
	} else if( create_sip_url( pUserUrl, szDial, pszDot[ 0 ] ) != -1 ) {
		/* use direct ip call. [output 'sip:xxx.xxx.xxx.xxx'] */
		strcpy( pSipUrl, szDial );
		goto label_catenate_dial_plan_done;
	}
	
	/* Auto Prefix? */
	voip_ptr = &g_pVoIPCfg->ports[ lc ->chid ];
	
	if( MatchPrefixUnsetProcess( &dialplan_internal_vars[ lc ->chid ], pUserUrl ) ||
		!IsNumericalString( pUserUrl ) ) 
	{
		pszUserUrlAddPrefixDial = pUserUrl;
	} else {
		strcpy( szDial, voip_ptr ->auto_prefix );
		strcat( szDial, pUserUrl );
		
		pszUserUrlAddPrefixDial = szDial;
	}
	
	/* Replace ? */
	if( MatchReplaceRuleProcess( &dialplan_internal_vars[ lc ->chid ], 
								 pszUserUrlAddPrefixDial, 
								 &idx_start, &idx_end , voip_ptr ->replace_rule_target) )
	{
		#if 0
		if( idx_start != 0 ) {
			printf( "Replace rule must match 0.\n" );
			goto label_unexpected_replace_index;
		}
		#endif

		if(idx_start!=0)//prefix before replace part, xxx<:>
		{
			strncat(pSipUrl, pszUserUrlAddPrefixDial, idx_start);
			//g_message("adding prefix str before replace, pSipUrl=%s\n", pSipUrl);
		}	

		
		strcat( pSipUrl, voip_ptr ->replace_rule_target );
		//g_message("adding replace str, pSipUrl=%s\n", pSipUrl);
		
		if(idx_start+idx_end < strlen(pszUserUrlAddPrefixDial))//suffix after replace part, <:> xxx
		{
			strcat( pSipUrl, idx_start+ pszUserUrlAddPrefixDial + idx_end );
			//g_message("adding suffix str after replace , pSipUrl=%s\n", pSipUrl);
		}
		g_message("replace finish,pszUserUrlAddPrefixDial=%s pSipUrl=%s\n", pszUserUrlAddPrefixDial, pSipUrl);
	} else {
label_unexpected_replace_index:
		strcpy( pSipUrl, pszUserUrlAddPrefixDial );
	}

label_catenate_dial_plan_done:
	return;
}

static void CatenateProxyInfoIfNecessary( const LinphoneCore *lc,
										  unsigned char *pUrlWithoutSip )
{
	unsigned char szSipUrl[ 256 ];

	/* ok. User's URL has to be acknowledged. */
	if( memcmp( pUrlWithoutSip, "sip:", 4 ) == 0 )
		return;

	voipCfgPortParam_t *voip_ptr;
	const voipCfgProxy_t *proxy_cfg;

	voip_ptr = &g_pVoIPCfg->ports[ lc ->chid ];
	proxy_cfg = &voip_ptr->proxies[lc->default_proxy ? lc->default_proxy->index : lc->default_proxy_index];

	if (proxy_cfg->port == 5060)
		sprintf(szSipUrl, "sip:%s@%s", pUrlWithoutSip, 
			proxy_cfg->addr);
	else
		sprintf(szSipUrl, "sip:%s@%s:%d", pUrlWithoutSip, 
			proxy_cfg->addr, proxy_cfg->port);

	strcpy( pUrlWithoutSip, szSipUrl );
}

static void DoCallxxxByDialPlan( LinphoneCore *lc, 
								 guint32 ssid, 
								 char *src_ptr,
								 int bCalloutCommand,
								 const unsigned char *pszDirectDial )
{
	/*
	 * - Use bCalloutCommand to decide to add prefix 'call' and set state
	 *   to CALL_OUT.
	 * - If its format is acknowledaged, 'sip:' is catenated.
	 *   (Where, format is possibly IP, or 'sip:....' string.)
	 */

	const unsigned char *pszBasicDialString;
	char szDot[ 2 ] = { '\x0', '\x0' };
	int offset;

	/* Initialize */
	if( bCalloutCommand ) {
		SetSessionState(lc->chid, ssid, SS_STATE_CALLOUT);
		strcpy(src_ptr, "call ");
		offset = 5;
	} else {
		src_ptr[ 0 ] = '\x0';
		offset = 0;
	}

	/* speed dial check */
	if( pszDirectDial ) {
		pszBasicDialString = pszDirectDial;
		szDot[ 0 ] = '.';
	} else if( ( pszBasicDialString = speed_dial_check( lc ->chid, lc ->dial_data, NULL ) ) ) {
		szDot[ 0 ] = '.';
	} else if( ( pszBasicDialString = GetAbbreviatedDialingUrl( lc ->chid, lc ->dial_data ) ) ) {
		szDot[ 0 ] = '.';
	} else {
		pszBasicDialString = lc ->dial_data;
		szDot[ 0 ] = '*';
	}

	/* 
	 * Produce a 'sip:xxxx' string, or a proxy call is adopted. 
	 * Later, 'sip:' and proxy information will be catenated.
	 */
	CatenateDialPlanString( lc, pszBasicDialString, src_ptr + offset, szDot );
	
	digitcode_init(lc);	
}

void CallInviteByDialPlan(LinphoneCore *lc, guint32 ssid, char *src_ptr)
{
	DoCallxxxByDialPlan( lc, ssid, src_ptr, 1 /* call out command */, NULL );

#if 0	
	printf( "===================================\n" );
	printf( "dialplan: [%s]\n", src_ptr );
	printf( "===================================\n" );
#endif
}

void CallTransferByDialPlan( LinphoneCore *lc, guint32 ssid )
{
	char szRefer[ LINE_MAX_LEN ];
	
	DoCallxxxByDialPlan( lc, ssid, szRefer, 0 /* NOT call out command */, NULL );
	CatenateProxyInfoIfNecessary( lc, szRefer );
	
	SetActiveSession(lc->chid, ssid, TRUE);
	linphone_core_transfer_call(lc, ssid, szRefer);
}

void CallForwardByDialPlan( const LinphoneCore *lc,
							const unsigned char *pUserUrl,
							unsigned char *pSipUrl,
							const unsigned char *pszDot )
{
	CatenateDialPlanString( lc, pUserUrl, pSipUrl, pszDot );
	
	/* 
	 * If user's url is not acknowledged (prefix is NOT 'sip:'), 
	 * we do proxy call. 
	 */
	CatenateProxyInfoIfNecessary( lc, pSipUrl );
}

void HotlineCallInviteByDialPlan(LinphoneCore *lc, guint32 ssid, char *src_ptr,
								 const char *pszDirectDial )
{
	DoCallxxxByDialPlan( lc, ssid, src_ptr, 1 /* call out command */, pszDirectDial );
}

void CallTestByDialPlan(LinphoneCore *lc, guint32 ssid, char *src_ptr)
{
	DoCallxxxByDialPlan(lc, ssid, src_ptr, 0, NULL);
}

/* ======================================================
 * Try to call xxxx
 * ====================================================== */
static int DoDialPlanTryCallxxx( LinphoneCore *lc, 
								 guint32 ssid, 
								 char *src_ptr,
								 int bCallInvite )
{
	dpMatchParam_t dpMatchParam;

	/* check chid */
	if( lc ->chid >= g_MaxVoIPPorts ) {
		printf( "Invalid chid for dial plan initialization!\n" );
		return 0;
	}
	
	if(g_pVoIPCfg->ports[ lc ->chid ].digitmap_enable==0)
		return 0;
	
	dpMatchParam.pDialData = lc ->dial_data;
	dpMatchParam.nDialDataLen = lc ->dial_data_index;
	
	switch( MatchDialProcess( &dialplan_internal_vars[ lc ->chid ], &dpMatchParam ) ) {

	case 0:
		printf("%s(%d)\n",__FUNCTION__,__LINE__);
		break;

	case 1:
		/* User input match the dial plan setting, then dial out. */
		if( bCallInvite )
			CallInviteByDialPlan( lc, ssid, src_ptr );
		else
			CallTransferByDialPlan( lc, ssid );
		return 1;

	/* 
	 * Turn on, if we think dial plan is used to *RESTRICT* dial number.
	 * Otherwise, dial plan is seen as a dial *ASSISTANT*. 
	 * NOTE: We don't consider _CONNECT_EDIT state, so this option need
	 *       to be refined.
	 */
	case 2:

		printf("%s(%d)FIXME\n",__FUNCTION__,__LINE__);
#if 0 //ericchung: dialplan match rule, if no match any dialplan , can't dial out.
		/* User input too much, then play busy tone. */
		rcm_tapi_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
		if( bCallInvite )
			SetSysState(lc->chid, SYS_STATE_EDIT_ERROR);
		else
			SetSysState(lc->chid, SYS_STATE_TRANSFER_EDIT_ERROR);
		
#endif
		return 0;
	}
	
	return 0;
}


int DoDialPlanCheck( LinphoneCore *lc)
{
	dpMatchParam_t dpMatchParam;
	dialplan_vars_t *me=NULL;

	/* check chid */
	if( lc ->chid >= g_MaxVoIPPorts ) {
		printf( "Invalid chid for dial plan initialization!\n" );
		return 0;
	}
	
	/* check digit map enable or not */
	if(g_pVoIPCfg->ports[ lc ->chid ].digitmap_enable==0){
		g_message("always pass  digitmap_enable disable!\n");
		return 1;
	}

	dpMatchParam.pDialData = lc ->dial_data;
	dpMatchParam.nDialDataLen = lc ->dial_data_index;
	me=&dialplan_internal_vars[ lc ->chid ];

	/* check use or not? */
	if( me ->bUseDialplan == 0 ) {
		g_message("always pass  Dial Plan because no dialplan!\n");
		return 1;
	}
	
	switch( MatchDialProcess( &dialplan_internal_vars[ lc ->chid ], &dpMatchParam ) ) {
	case 1:
    	case 2:
		/* Match */
		g_message("DoDialPlanCheck match dial plan\n");
		return 1;
		break;

	case 0:
	case -1:
		/* no match */
		g_message("DoDialPlanCheck NO match  dial plan\n");

		return 0;
		break;
        }

}
int DialPlanTryCallInvite( LinphoneCore *lc, guint32 ssid, char *src_ptr)
{
	int ret;
	ret= DoDialPlanTryCallxxx( lc, ssid, src_ptr, 1 /* call invite */ );
	g_message("\r\n DialPlanTryCallInvite return %d\n",ret);
	return ret;
}

int DialPlanTryCallTransfer( LinphoneCore *lc, guint32 ssid )
{
	return DoDialPlanTryCallxxx( lc, ssid, NULL /* ignore */, 0 /* NOT call invite */ );
}



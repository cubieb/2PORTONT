#include <string.h>
#include "linphonecore.h"
#include "pstnroute.h"

//#define PR_UNIT_TEST

//#define NO_CALLOUT_PREFIX  /* it is remove prefix */

#ifdef PR_UNIT_TEST
static unsigned char *szPrefix[ 5 ] = {
	"022",
	"03",
	"04",
	"07",
	"009",
};
#endif

#if 0
static const unsigned char szPrefix[] = {
	"002,03,04,07,009"
};
#endif

const unsigned char *GetPSTNRoutingNumber( const LinphoneCore *lc, 
										   const unsigned char *pURI, 
										   int bPrefixCallSz )
{
	/*
	 * pURI: User's dialing number, or processed by dial plan 
	 * bPrefixCallSz: after processing of dial plan, its prefix may be 'call'. 
	 */
#ifdef PR_UNIT_TEST
	int i, len;
#endif
	const unsigned char *pchSrc, *pchDst;
	unsigned char chSrc, chDst;
	int nMatch;
	const voipCfgPortParam_t * const pCfg = &g_pVoIPCfg->ports[lc->chid];
	
	/* make sure that there is a FXO port. */
	//if( g_MaxVoIPPorts > RTK_VOIP_SLIC_NUM( g_VoIP_Feature ) )
	if( RTK_VOIP_DAA_NUM( g_VoIP_Feature ) > 0 )
		;
	else
		return NULL;
	
	/* shift pURI to correct position */
	if( bPrefixCallSz ) {
		/* prefix should be 'call ' */
		if( memcmp( pURI, "call ", 5 ) == 0 )
			pURI += 5;
		else {
			printf( "PSTN routing prefix should be 'call'\n" );
			return NULL;
		}
	}
	
	/* check prefix */
#if 0
	pchSrc = szPrefix;
#else
	pchSrc = pCfg ->PSTN_routing_prefix;
#endif
	nMatch = 0;
	pchDst = pURI;
	
	while( ( chSrc = *pchSrc ++ ) ) {
		
		if( chSrc >= '0' && chSrc <= '9' ) {
			
			if( nMatch < 0 )
				;
			else if( *pchDst ++ == chSrc )
				nMatch ++;
			else
				nMatch = -1;
			
		} else {
		
			if( nMatch > 0 )	/* match prefix */
				goto label_match_a_certain_prefix;	
		
			nMatch = 0;
			pchDst = pURI;
		}
	}
	
	if( nMatch > 0 && chSrc == '\x0' )		/* match the last prefix */
		goto label_match_a_certain_prefix;
	
#ifdef PR_UNIT_TEST
	for( i = 0; i < 5; i ++ ) {
		len = strlen( szPrefix[ i ] );
		
		if( memcmp( pURI, szPrefix[ i ], len ) == 0 )
			return pURI;	/* match!! */
	}
#endif
	
	return NULL;

label_match_a_certain_prefix:

#ifdef NO_CALLOUT_PREFIX
	return pURI + nMatch;
#else
	return pURI;
#endif
}



#include <string.h>
#include "linphonecore.h"
#include "abbreviated.h"

const unsigned char *GetAbbreviatedDialingUrl( int chid,
									const unsigned char *pDialingName )
{
	int i;
	
	for( i = 0; i < MAX_ABBR_DIAL_NUM; i ++ ) {
		
		if( g_pVoIPCfg->ports[chid].abbr_dial[i].name[ 0 ] && 
			g_pVoIPCfg->ports[chid].abbr_dial[i].url[ 0 ] )
		{
		} else
			continue;
		
		if( strcmp( g_pVoIPCfg->ports[chid].abbr_dial[i].name,
					pDialingName ) == 0 )
		{
			return g_pVoIPCfg->ports[chid].abbr_dial[i].url;
		}
	}

	return NULL;
}


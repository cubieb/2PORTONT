#include <stdio.h>
#include <unistd.h>
#include "voip_feature.h"
#include "voip_manager.h"

int main()
{
	int i;
	int dsp_nr;
	
	// TH: add for ethernet DSP booting
	if (RTK_VOIP_CHECK_IS_IPC_ARCH(g_VoIP_Feature) &&
		RTK_VOIP_CHECK_IS_IPC_HOST(g_VoIP_Feature))
	{
		dsp_nr = RTK_VOIP_DSP_DEVICE_NUMBER( g_VoIP_Feature );
		
		for (i=0; i < dsp_nr; i++)
		{
			fprintf(stderr , "Wait DSP %d/%d booting ......\n", i, dsp_nr);
			fflush(stdout);//force above message to display before while loop
			
			while( rtk_CheckDspAllSoftwareReady(i) == 0 ) {
				fprintf(stderr ,  "." );
				fflush(stdout);
				sleep( 1 );
			}
				
			fprintf(stderr , "DSP %d Software Ready!\n", i);
			
			if( RTK_VOIP_CHECK_IS_IPC_ETHERNETDSP( g_VoIP_Feature ) )
				goto label_ethernet_dsp_next;
			else
				continue;

label_ethernet_dsp_next:			
			if (i == 0)
				rtk_SetDspIdToDsp(15);  // force DSP to gen mido interrupt
			rtk_SetDspIdToDsp(i);
		}
	}
	
	return 0;
}


#include <unistd.h>
#include "voip_manager.h"

int main(void)
{
	int i;
	
	for (i=0; i<CON_CH_NUM; i++)
	{
		if( !RTK_VOIP_IS_SLIC_CH( i, g_VoIP_Feature ) )
			continue;
		
		if (i == 0)
			rtk_SetSlicRingCadence(i, 600, 600); // test value
		else
			rtk_SetSlicRingCadence(i, 1500, 1000); // default value

		rtk_SetRingFxs(i, 1);
		sleep(3);
		rtk_SetRingFxs(i, 0);
	}

	return 0;
}

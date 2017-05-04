#include "voip_manager.h"

int main()
{
	int i;

	for (i=0; i<SLIC_CH_NUM; i++)
	{
		if (i == 0)
			rtk_Set_SLIC_Ring_Cadence(i, 600, 600); // test value
		else
			rtk_Set_SLIC_Ring_Cadence(i, 1500, 1000); // default value

		rtk_SetRingFXS(i, 1);
		sleep(3);
		rtk_SetRingFXS(i, 0);
	}

	return 0;
}

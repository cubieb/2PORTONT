#include "voip_manager.h"

int dbg_main(int argc, char *argv[])
{
	if (argc == 2 || argc == 3)
	{
		int dbg;
		int watchdog;

		dbg = atoi(argv[1]);
		watchdog = ( argc == 3 ? atoi(argv[2]) : 0xff );
		rtk_debug_with_watchdog(dbg, watchdog);
		printf("set dbg flag = %d\n", dbg);
	}
	else if (argc == 4)
	{
		if (atoi(argv[1]) == 1)
		{
			rtk_Set_CID_DTMF_MODE(0, 0x8, 0x8); // priori 1st ring, auto AC
		}
		else if(atoi(argv[1]) == 2)	
		{
			rtk_Set_CID_DTMF_MODE(0, 0, 0x10);// after 1st ring, non-auto
		}
		else if(atoi(argv[1]) == 3)	
		{
			rtk_Gen_Dtmf_CID(0, "654321");
		}
		else if(atoi(argv[1]) == 4)	
		{
			rtk_Gen_Dtmf_CID(0, "A3939080C");
		}
		else if(atoi(argv[1]) == 5)	
		{
			rtk_Gen_Dtmf_CID(0, "D3#");
		}
		else if (atoi(argv[1]) == 6)
		{
			rtk_Set_Impedance(atoi(argv[2]));
		}
		else if (atoi(argv[1]) == 7)
		{
			rtk_Gen_SLIC_CPC(atoi(argv[2]), atoi(argv[3]));
		}
	}
	else
	{
		printf("use: %s dbg_flag [watchdog]\n", argv[0]);
	}

	return 0;
}


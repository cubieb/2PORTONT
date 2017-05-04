#include "voip_manager.h"

int ram_main(int argc, char *argv[])
{
	if (argc == 3)
	{
		int chid, reg;

		chid = atoi(argv[1]);

		if (argv[2][0] == 'r')
		{
			rtk_reset_slic(chid, 1); //1: set to A-law
			printf("reset slic %d done\n", chid);
		}
		else
		{
			reg = atoi(argv[2]);
			printf("read: chid = %d, ram =%d, val = %x\n", 
				chid, reg, rtk_Get_SLIC_Ram_Val(chid, reg));
		}
	}
	else if (argc == 4)
	{
		int chid, reg, val;

		chid = atoi(argv[1]);
		reg = atoi(argv[2]);
		val = atoi(argv[3]);

		rtk_Set_SLIC_Ram_Val(chid, reg, val);
		printf("write: chid = %d, ram = %d, val = %x\n", 
			chid, reg, val);
	}
	else
	{
		printf("use: %s chid ram_num [ram_val]\n", argv[0]);
	}

	return 0;
}


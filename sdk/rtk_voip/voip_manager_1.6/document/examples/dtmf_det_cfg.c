#include <stdio.h>
#include <stdlib.h>
#include "voip_manager.h"

void ShowUsage(char *cmd)
{
	printf("Usage:\n");
        printf( "* Enable/Disable DTMF det:\n"	\
        	" - %s <chid> <dir> <enable>\n" \
        	"   - dir => 0 : TDM-side, 1: IP-side\n" \
        	"   - enable => 0: disable, 1: enable\n" \
        	"\n* Set DTMF det parameters:\n" \
        	" - %s <chid> <dir> <threshold> <on_time> <fore_twist> <rev_twist> <freq_offset>\n" \
        	"   - dir => 0 : TDM-side, 1: IP-side\n" \
                "   - threshold => 0 ~ 40, it means 0 ~ -40 dBm\n" \
                "   - on_time_10ms => 3~12, it means minimum dtmf on time\n" \
                "   - fore_twist => 1 ~ 16, it means 0 ~ 16 dB\n" \
                "   - rev_twist => 1 ~ 16, it means 0 ~ 16 dB\n" \
                "   - freq_offset => 0 : not support, 1: support\n" , cmd, cmd);
        exit(0);
}

int main(int argc, char *argv[])
{

	if (argc == 4)
	{
		rtk_SetDtmfCfg(atoi(argv[1]), atoi(argv[3]), atoi(argv[2]));
		printf("set DTMF det = %s, ch%d, dir%d\n", atoi(argv[3]) ? "On" : "Off", atoi(argv[1]), atoi(argv[2]));
	}
	else if (argc == 7)
	{
#if 0
		rtk_SetDtmfDetParam(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));
#else
		TstDtmfDetPara stDtmfDetPara;

		stDtmfDetPara.ch_id = atoi(argv[1]);
		stDtmfDetPara.dir = atoi(argv[2]);
		stDtmfDetPara.thres_upd = 1;
		stDtmfDetPara.thres = atoi(argv[3]);
		stDtmfDetPara.on_time_upd = 1;
		stDtmfDetPara.on_time = atoi(argv[4]);
		stDtmfDetPara.twist_upd = 1;
		stDtmfDetPara.fore_twist = atoi(argv[5]);
		stDtmfDetPara.rev_twist = atoi(argv[6]);
		stDtmfDetPara.freq_offset_upd = 0;
		
		rtk_SetDtmfDetParamUpdate(&stDtmfDetPara);
#endif
	}
	else if (argc == 8)
	{
		TstDtmfDetPara stDtmfDetPara;

		stDtmfDetPara.ch_id = atoi(argv[1]);
		stDtmfDetPara.dir = atoi(argv[2]);
		stDtmfDetPara.thres_upd = 1;
		stDtmfDetPara.thres = atoi(argv[3]);
		stDtmfDetPara.on_time_upd = 1;
		stDtmfDetPara.on_time = atoi(argv[4]);
		stDtmfDetPara.twist_upd = 1;
		stDtmfDetPara.fore_twist = atoi(argv[5]);
		stDtmfDetPara.rev_twist = atoi(argv[6]);
		stDtmfDetPara.freq_offset_upd = 1;
		stDtmfDetPara.freq_offset = atoi(argv[7]);
		
		rtk_SetDtmfDetParamUpdate(&stDtmfDetPara);
	}
	else
	{
		ShowUsage(argv[0]);
	}

	return 0;
}


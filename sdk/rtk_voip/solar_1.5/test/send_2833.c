#include <sys/time.h>
#include "voip_manager.h"

int send_2833_main(int argc, char *argv[])
{
	if (argc == 5)
	{
		if (argv[1][0] == 'l')
			rtk_LimitMaxRfc2833DtmfDuration(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
		else
			rtk_SetRTPRFC2833(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
	}
	else if (argc == 3)
	{
		rtk_SetRFC2833SendByAP(atoi(argv[1]), atoi(argv[2]));
	}
	else if (argc == 4)
	{
		rtk_SetRFC2833TxConfig(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
	}
	else
	{
		printf("****** Usage ******\n");
		printf("'send_2833 chid 0' to set RFC2833 sent by DSP.\n");
		printf("'send_2833 chid 1' to set RFC2833 sent by application.\n");
		printf("'send_2833 chid 10 0' set TX volume to -10 dBm and set RFC2833 sent by DSP.\n");
		printf("'send_2833 chid 5 1' set TX volume to -5 dBm and set RFC2833 sent by application.\n");
		printf("'send_2833 chid sid dtmf_event duration' to send RFC2833 event\n");
		printf("Note: above tests are only workable when you set to 2833 mode, and during VoIP call\n");
		printf("'send_2833 limit chid duration(ms) bEnable(0 or 1)' to config RFC2833 DTMF duation max. limitation\n");
	}


	return 0;
}



#include "voip_manager.h"

void ShowUsage(char *cmd)
{
	printf("usage: %s <mode> <caller_id> <FSK area> <FSK_type> <DTMF mode> \n" \
		"  - mode => 0 is DTMF, 1 is FSK\n" \
		"  - caller_id => caller id string\n" \
		"  - FSK area[2:0] => 0 BELLCORE, 1: ETSI, 2: BT, 3: NTT\n" \
		"  - FSK area[bit7]=> FSK date & time sync\n" \
		"  - FSK area[bit6]=> reverse polarity before caller id (For FSK)\n" \
		"  - FSK area[bit5]=> short ring before caller id (For FSK)\n" \
		"  - FSK area[bit4]=> dual alert tone before caller id (For FSK)\n" \
		"  - FSK area[bit3]=> caller id Prior Ring (FSK & DTMF)\n" \
		"  - FSK type => 1 is type-I, 2 is type-II\n"\
		"  - DTMF mode[1:0]=> Start digit, 0:A, 1:B, 2:C, 3:D\n" \
		"  - DTMF mode[3:2]=> End digit, 0:A, 1:B, 2:C, 3:D\n"	\
		"  - DTMF mode[4]=> Auto start/end digit send, 0:suto mode 1:non-auto\n" \
		"	(non-auto mode: DSP send caller ID string only. If caller ID need start/end digits, developer should add them to caller ID strings.)\n", cmd);
	exit(0);
}

int main(int argc, char *argv[])
{
	unsigned int mode, fsk_area, dtmf_mode, fsk_type;

	if (argc < 4)
	{
		ShowUsage(argv[0]);
	}
	mode = atoi(argv[1]);
	switch (mode)
	{
	case 0:
		if (argc == 5)
		{
			fsk_area = atoi(argv[3]);
			dtmf_mode = atoi(argv[4]);
			rtk_Set_CID_DTMF_MODE(0, fsk_area, dtmf_mode); //for DTMF caller id prior ring or not. & set the caller id start/end digit
			rtk_Gen_Dtmf_CID(0, argv[2]);
		}
		else if(argc == 4)
		{
			fsk_area = atoi(argv[3]);
			dtmf_mode = 0;	// start/end digit : A.
			rtk_Set_CID_DTMF_MODE(0, fsk_area, dtmf_mode); //for DTMF caller id prior ring or not.
			rtk_Gen_Dtmf_CID(0, argv[2]);
		}
		else if(argc == 3)
		{
			if (argv[2] == 1)
			{
				// for DTMF caller id priori 1st ring, auto send start/end digit
				rtk_Set_CID_DTMF_MODE(0, 0x8, 0x8);							
				rtk_Gen_Dtmf_CID(0, "654321");
			}
			else if (argv[2] == 2)
			{
				// for DTMF caller id after 1st ring, non-auto
				rtk_Set_CID_DTMF_MODE(0, 0, 0x10);							
				// DSP send caller ID string only. If caller ID need start/end digits, developer should add them to caller ID strings	
				rtk_Gen_Dtmf_CID(0, "A654321C");
			}
		}
		break;
	case 1:
		if (argc == 5)
		{
			fsk_area = atoi(argv[3]);
			
			//rtk_Set_CID_FSK_GEN_MODE(0, 1);
			
			if ((fsk_area&7) > CID_DTMF)
				printf("wrong FSK area => 0 BELLCORE, 1: ETSI, 2: BT, 3: NTT\n");
			else if(atoi(argv[4])!= 1 && atoi(argv[4])!= 2)
				printf("wrong fsk type: should be type-I(1) or type-II(2)\n");
			else
			{	
				fsk_type = atoi(argv[4])- 1 ;
				rtk_Set_FSK_Area(0, fsk_area);
				rtk_Gen_FSK_CID(0, argv[2], (void *) 0, (void *) 0, fsk_type/*FSK Type*/); // 
			}
		}
		break;
	default:
		ShowUsage(argv[0]);
	}
	return 0;
}

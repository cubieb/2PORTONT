#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include "voip_manager.h"

#define SLIC_CH_NUM 2

int main(void)
{
	int i;
	int32 ret;
	static int8 pre_Event=0;
	int8 Event=0;
	int8 Energy;
	int16 Duration;
	
	SLICEVENT SlicEvent = SLICEVENT_NONE;

	for (i=0; i<SLIC_CH_NUM; i++)
	{
		rtk_Set_Voice_Gain(i, 0, 0);
		rtk_Set_Flash_Hook_Time(i, 0, 300);
		rtk_Set_flush_fifo(i);					// flush kernel fifo before app run
	}

main_loop:

	for (i=0; i<SLIC_CH_NUM; i++)
	{

		if( ( ret = rtk_GetSlicEvent( i, &SlicEvent ) ) < 0 )
			return ret;

		if ( SlicEvent == SLICEVENT_OFFHOOK_2 ) /* PHONE_STILL_OFF_HOOK */
		{
			// detect DTMF
			rtk_GetDtmfEvent(i, 0, &Event, &Energy, &Duration);
		
			if ( (Event != 'E' ) && (Event !='Z') )
			{
				if (Event >= '1' && Event <= '9')
				{
					if ( (Event - pre_Event) != 1)
						printf("====> Error, cur=%c, pre=%c\n", Event, pre_Event);
				}
				else if (Event == '0')
				{
					printf("\n\n");
				}
				else if (Event == 'A')
				{
					if (pre_Event != '9')
						printf("====> Error, cur=%c, pre=%c\n", Event, pre_Event);
				}
				else if (Event == 'B')
				{
					if (pre_Event != 'A')
						printf("====> Error, cur=%c, pre=%c\n", Event, pre_Event);
				}
				else if (Event == 'C')
				{
					if (pre_Event != 'B')
						printf("====> Error, cur=%c, pre=%c\n", Event, pre_Event);
				}
				else if (Event == 'D')
				{
					if (pre_Event != 'C')
						printf("====> Error, cur=%c, pre=%c\n", Event, pre_Event);
				}
				else if (Event == '*')
				{
					if (pre_Event != 'D')
						printf("====> Error, cur=%c, pre=%c\n", Event, pre_Event);
				}
				else if (Event == '#')
				{
					if (pre_Event != '*')
						printf("====> Error, cur=%c, pre=%c\n", Event, pre_Event);
				}

				printf("%c, ", Event);

				pre_Event = Event;
			}
			else if (Event == 'E' )
			{
				printf("%ddBFS, %dms\n", Energy, Duration);
			}
		}
		else if ( SlicEvent == SLICEVENT_OFFHOOK ) /* PHONE_OFF_HOOK */
		{
			/* when phone offhook, enable pcm and DTMF detection */
			rtk_enablePCM(i, 1);
			rtk_Set_DTMF_CFG(i, 1, 0);
		}
		else if ( SlicEvent == SLICEVENT_ONHOOK ) /* PHONE_ON_HOOK */
		{
			rtk_enablePCM(i, 0);
			rtk_Set_DTMF_CFG(i, 0, 0);
			rtk_Onhook_Reinit(i);
		}
		
		usleep(100000 / SLIC_CH_NUM); // 100 ms
	}

	goto main_loop;

	return 0;
}

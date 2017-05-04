#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "voip_manager.h"
#include "voip_control.h"

int main(int argc, char* argv[]){
	int opt = 0;
	int chid = 0, isPlugged = 0, type = 0;
	//int roh[4], voltage[6], flt[6];
	TstVoipSLICLineRoh roh;
	TstVoipSLICLineVoltage voltage;
	TstVoipSLICLineResFlt flt;
	TstVoipSLICLineCap cap;
	TstVoipSLICLineLoopCurrent loopcurrent;
	
	memset(&roh, 0, sizeof(TstVoipSLICLineRoh));
	memset(&voltage, 0, sizeof(TstVoipSLICLineVoltage));
	memset(&flt, 0, sizeof(TstVoipSLICLineResFlt));
	memset(&cap, 0, sizeof(TstVoipSLICLineCap));
	memset(&loopcurrent, 0, sizeof(TstVoipSLICLineLoopCurrent));
	
	if ( argc != 5 )
	{
		printf("Please input channel id\n");
		printf("\t-c : channel id = 0 or 1\n");
		printf("\t-t : \n");
		printf("\t\t 0:port detect\n");
		printf("\t\t 1:roh\n");
		printf("\t\t 2:line voltage\n");
		printf("\t\t 3:RES FLT\n");
		printf("\t\t 4:Capacity\n");
		printf("\t\t 5:Loop Current\n");
		return 0;
	}
	
	while (( opt = getopt(argc, argv, "c:t:")) != -1 )
	{
		switch (opt)
		{
			case 'c':
				sscanf(optarg, "%d", &chid);
				break;				
			case 't':
				sscanf(optarg, "%d", &type);
				break;								
			case ':':
				printf("need to more parameters\n");
				break;
		}
	}

//	printf("chid = [%d], type = [%d]\n", chid, type);

	if ( type == 0 ){
		isPlugged = rtk_GetPortStatus(chid);
	
		if ( isPlugged )
			printf("phone is plugged\n");
		else
			printf("phone isn't been plugged\n");
	}else if ( type == 1 ){
		//memset(roh, 0, sizeof(int)*4);
		rtk_GetLineRoh(chid, &roh);
#if 0		
		printf("fltMask = 0x[%x]\n", roh[0]);
		printf("rLoop1 = 0x[%x]\n", roh[1]);
		printf("rLoop2 = 0x[%x]\n", roh[2]);
		printf("measStatus = 0x[%x]\n", roh[3]);
#else
		printf("fltMask = 0x[%x]\n", roh.fltMask);
		printf("measStatus = 0x[%x]\n", roh.measStatus);
		printf("rLoop1 = 0x[%x]\n", roh.rLoop1);
		printf("rLoop2 = 0x[%x]\n", roh.rLoop2);
#endif		
	}else if ( type == 2 ){
		//memset(voltage, 0, sizeof(int)*6);
		rtk_GetLineVoltage(chid, &voltage);
#if 0		
		printf("vAcDiff = %d\n", voltage[0]);
		printf("vAcRing = %d\n", voltage[1]);
		printf("vAcTip = %d\n", voltage[2]);
		printf("vDcDiff = %d\n", voltage[3]);
		printf("vDcRing = %d\n", voltage[4]);
		printf("vDcTip = %d\n", voltage[5]);
#else
		printf("vAcDiff = %d\n", voltage.vAcDiff);
		printf("vAcRing = %d\n", voltage.vAcRing);
		printf("vAcTip = %d\n", voltage.vAcTip);
		printf("vDcDiff = %d\n", voltage.vDcDiff);
		printf("vDcRing = %d\n", voltage.vDcRing);
		printf("vDcTip = %d\n", voltage.vDcTip);		
		printf("fltMask = 0x[%x]\n", voltage.fltMask);
		printf("measStatus = 0x[%x]\n", voltage.measStatus);
#endif		
	}else if ( type == 3 ){	
		//memset(flt, 0, sizeof(int)*6);
		rtk_GetLineResflt(chid, &flt);
#if 0		
		printf("rGnd = %d\n", flt[0]);
		printf("rrg = %d\n", flt[1]);
		printf("rtg = %d\n", flt[2]);
		printf("rtr = %d\n", flt[3]);
		printf("fltMask = 0x[%x]\n", flt[4]);
		printf("measStatus = %d\n", flt[5]);
#else		
		printf("rGnd = %d\n", flt.rGnd);
		printf("rrg = %d\n", flt.rrg);
		printf("rtg = %d\n", flt.rtg);
		printf("rtr = %d\n", flt.rtr);
		printf("fltMask = 0x[%x]\n", flt.fltMask);
		printf("measStatus = 0x[%x]\n", flt.measStatus);
#endif
	}else if ( type == 4 ){	 // capacitance
		rtk_GetLineCap(chid, &cap);
		printf("crg = %d\n", cap.crg);
		printf("ctg = %d\n", cap.ctg);
		printf("ctr = %d\n", cap.ctr);
		printf("fltMask = 0x[%x]\n", cap.fltMask);
		printf("measStatus = 0x[%x]\n", cap.measStatus);	
	}else if ( type == 5 ){	 // Loop Current
		rtk_GetLineLoopCurrent(chid, &loopcurrent);
		printf("iTestLoad = %d\n", loopcurrent.iTestLoad);
		printf("rTestLoad = %d\n", loopcurrent.rTestLoad);
		printf("vTestLoad = %d\n", loopcurrent.vTestLoad);
		printf("fltMask = 0x[%x]\n", loopcurrent.fltMask);
		printf("measStatus = 0x[%x]\n", loopcurrent.measStatus);	
	}
	return 0;
}


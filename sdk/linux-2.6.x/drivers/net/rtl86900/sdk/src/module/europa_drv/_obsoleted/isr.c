//#include <stdio.h>
//#include <reg_sfr.h>
//#include <asicreg.h>

//uint8  data timeCount;
//volatile uint8  idata _th0;
//volatile uint32 data current_time;
//volatile uint8  idata _th0;
//volatile uint8  idata _tl0;
//uint32 data system_clock;
//uint8  data timeCount;
#if 0
extern void setReg(uint16, uint16);//test

void switch_isr() interrupt 0
{
	;
}

void timer_0_isr() interrupt 1 //5ms
{
    EA = 0;
    
    /*100M, 200HZ*/
    TH0 = 0x5D;
    TL0 = 0x3D;
    
    timeCount++;
    
    
    setReg(0x1202, 0xEEEE);//test
    
    //printf("cccccccccccccccccccccccc\n");  
    if(timeCount%10==0)//test
        setReg(0x13a1, timeCount);//test
    
    EA = 1;

}
#endif

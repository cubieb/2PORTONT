/*	
 *	re_gpiosmi.c:
 *	Using two GPIO pins to emulate MDC/MDIO signals to access external
 *	RTL830x series PHY registers.
*/

#include <linux/types.h>
//#include <platform.h>
/*linux-2.6.19*/
#include <bspchip.h>
#include "re_gpiosmi.h"
//#include "../../arch/mips/realtek/rtl8672/gpio.h"
/*linux-2.6.19*/
#include "../../arch/rlx/bsp/gpio.h"

#ifdef GPIO_SIMULATE
#define PABCDIR		(GPIO_PABCD_DIR-GPIOCR_BASE)//jiunming, for 8672, (GPIO_MDC<=7?0x010:0x000)
#define PABCDAT		(GPIO_PABCD_DAT-GPIOCR_BASE)//jiunming, for 8672, (GPIO_MDC<=7?0x014:0x004)
//Using GPIO Port A Pin 6 to emulate MDIO and GPIO Port A Pin 7 to emulate MDC
#define REG32GPIO(reg)	*(volatile unsigned int*)(GPIOCR_BASE+reg) //jiunming, for 8672, *(volatile unsigned int*)(0xb9c01000+reg)
#define GPIO_SHIFT(x)	(x)	//jiunming, for 8672,((x<=7?x:(x-8))+16)
/* Change clock to 1 */
void _rtl8305s_smiZBit(void) {
	unsigned int i;
	//REG32(PABCDIR) = (REG32(PABCDIR)& 0x3FFFFFFF) | 0x80000000;
	REG32GPIO(PABCDIR)= (REG32GPIO(PABCDIR)& ~((1<<GPIO_SHIFT(GPIO_MDC))|(1<<GPIO_SHIFT(GPIO_MDIO))))
	 | (1<<GPIO_SHIFT(GPIO_MDC));
	//REG32(PABCDAT) = (REG32(PABCDAT) & 0x3FFFFFFF);
	//REG32GPIO(PABCDAT) = (REG32GPIO(PABCDAT) & 0xFFe7FFFF);
	gpioClear(GPIO_MDC);//GPIO_PA4
	gpioClear(GPIO_MDIO);//GPIO_PA3
	for(i=0; i<25; i++);
}

/* Generate  1 -> 0 transition and sampled at 1 to 0 transition time */
void _rtl8305s_smiReadBit(unsigned char * data) {
	unsigned int i;
	//REG32(PABCDIR) = (REG32(PABCDIR)& 0x3FFFFFFF) | 0x80000000;
	REG32GPIO(PABCDIR)= (REG32GPIO(PABCDIR)& ~((1<<GPIO_SHIFT(GPIO_MDC))|(1<<GPIO_SHIFT(GPIO_MDIO))))
	 | (1<<GPIO_SHIFT(GPIO_MDC));
	//REG32(PABCDAT) = (REG32(PABCDAT) & 0x3FFFFFFF) | 0x80000000;
	//REG32GPIO(PABCDAT) = (REG32GPIO(PABCDAT) & 0xFFe7FFFF) | 0x00100000;
	gpioSet(GPIO_MDC); //GPIO_PA4
	gpioClear(GPIO_MDIO); //GPIO_PA3
	for(i=0; i<25; i++);
	//REG32(PABCDAT) = (REG32(PABCDAT) & 0x3FFFFFFF);
	//REG32GPIO(PABCDAT) = (REG32GPIO(PABCDAT) & 0xFFe7FFFF);	
	gpioClear(GPIO_MDC);//GPIO_PA4
	gpioClear(GPIO_MDIO);//GPIO_PA3	
	//*data = (REG32(PABCDAT) & 0x40000000)?1:0;
	*data = (REG32GPIO(PABCDAT) & (1<<GPIO_SHIFT(GPIO_MDIO)))?1:0;
}

/* Generate  0 -> 1 transition and put data ready during 0 to 1 whole period */
void _rtl8305s_smiWriteBit(unsigned char data) {
	unsigned int i;
	
	//REG32(PABCDIR) = REG32(PABCDIR) | 0xC0000000;
	REG32GPIO(PABCDIR) = REG32GPIO(PABCDIR) | (1<<GPIO_SHIFT(GPIO_MDC)) | (1<<GPIO_SHIFT(GPIO_MDIO));
	if(data) {/* Write 1 */
		//REG32(PABCDAT) = (REG32(PABCDAT) & 0x3FFFFFFF) | 0x40000000;
		//REG32GPIO(PABCDAT) = (REG32GPIO(PABCDAT) & 0xFFe7FFFF) | 0x00080000;
		gpioClear(GPIO_MDC);//GPIO_PA4
		gpioSet(GPIO_MDIO);//GPIO_PA3

		for(i=0; i<25; i++);
		//REG32(PABCDAT) = (REG32(PABCDAT) & 0x3FFFFFFF) | 0xC0000000;
		//REG32GPIO(PABCDAT) = (REG32GPIO(PABCDAT) & 0xFFe7FFFF) | 0x00180000;
		gpioSet(GPIO_MDC);//GPIO_PA4
		gpioSet(GPIO_MDIO);//GPIO_PA3
	} else {
		//REG32(PABCDAT) = (REG32(PABCDAT) & 0x3FFFFFFF);
		//REG32GPIO(PABCDAT) = (REG32GPIO(PABCDAT) & 0xFFe7FFFF);
		gpioClear(GPIO_MDC);//GPIO_PA4
		gpioClear(GPIO_MDIO);//GPIO_PA3
		for(i=0; i<25; i++);
		//REG32(PABCDAT) = (REG32(PABCDAT) & 0x3FFFFFFF) | 0x80000000;
		//REG32GPIO(PABCDAT) = (REG32GPIO(PABCDAT) & 0xFFe7FFFF) | 0x00100000;
		gpioSet(GPIO_MDC);//GPIO_PA4
		gpioClear(GPIO_MDIO);//GPIO_PA3
	}
}

/*
@func void | rtl8305s_smiRead | Read data from RTL8305SB/RTL8305S through MDC/MDIO interface
@parm uint8 | phyad | Specify the PHY to get information.
@parm uint8 | regad | Specify the Regist of PHY to get information. 
@parm uint16 * | data | The data get from RTL8305SB/RTL8305S.
@comm
Specify the PHY address and register address to get data from RTL8305SB/RTL8305S
Change MDIO to input state after write operation.
*/
void rtl8305s_smiRead(unsigned char phyad, unsigned char regad, unsigned short * data) {
	int i;
	unsigned char readBit;

	/* Configure port A pin 6, 7 to be GPIO and disable interrupts of these two pins */
	//REG32(PABCCNR) = REG32(PABCCNR) & 0x3FFFFFFF;
	//REG32(PABIMR) = REG32(PABIMR) & 0xFFFFFFF;
	/* 32 continuous 1 as preamble*/
	for(i=0; i<32; i++)
		_rtl8305s_smiWriteBit(1);
	/* ST: Start of Frame, <01>*/
	_rtl8305s_smiWriteBit(0);
	_rtl8305s_smiWriteBit(1);
	/* OP: Operation code, read is <10> */
	_rtl8305s_smiWriteBit(1);
	_rtl8305s_smiWriteBit(0);
	/* PHY Address */
	for(i=4; i>=0; i--) 
		_rtl8305s_smiWriteBit((phyad>>i)&0x1);
	/* Register Address */
	for(i=4; i>=0; i--) 
		_rtl8305s_smiWriteBit((regad>>i)&0x1);
	/* TA: Turnaround <z0> */
	_rtl8305s_smiZBit();
	_rtl8305s_smiReadBit(&readBit);
	/* Data */
	*data = 0;
	for(i=15; i>=0; i--) {
		_rtl8305s_smiReadBit(&readBit);
		*data = (*data<<1) | readBit;
	}
	/*add  an extra clock cycles for robust reading , ensure partner stop output signal
	and not affect the next read operation, because TA steal a clock*/     
	_rtl8305s_smiWriteBit(1);
	_rtl8305s_smiZBit();
}

/*
@func void | rtl8305s_smiWrite | Write data to RTL8305SB/RTL8305SC through MDC/MDIO interface
@parm uint8 | phyad | Specify the PHY to put information.
@parm uint8 | regad | Specify the Regist of PHY to put information. 
@parm uint16 | data | The data put to RTL8305SB/RTL8305S.
@comm
Specify the PHY address and register address to put data to RTL8305SB/RTL8305S.
Change MDIO to input state after write operation.
*/
void rtl8305s_smiWrite(unsigned char phyad, unsigned char regad, unsigned short data) {
	int i;

	/* Configure port A pin 6, 7 to be GPIO and disable interrupts of these two pins */
	//REG32(PABCCNR) = REG32(PABCCNR) & 0x3FFFFFFF;
	//REG32(PABIMR) = REG32(PABIMR) & 0xFFFFFFF;
	/* 32 continuous 1 as preamble*/
	for(i=0; i<32; i++)
		_rtl8305s_smiWriteBit(1);
	/* ST: Start of Frame, <01>*/
	_rtl8305s_smiWriteBit(0);
	_rtl8305s_smiWriteBit(1);
	/* OP: Operation code, write is <01> */
	_rtl8305s_smiWriteBit(0);
	_rtl8305s_smiWriteBit(1);
	/* PHY Address */
	for(i=4; i>=0; i--) 
		_rtl8305s_smiWriteBit((phyad>>i)&0x1);
	/* Register Address */
	for(i=4; i>=0; i--) 
		_rtl8305s_smiWriteBit((regad>>i)&0x1);
	/* TA: Turnaround <10> */
	_rtl8305s_smiWriteBit(1);
	_rtl8305s_smiWriteBit(0);
	/* Data */
	for(i=15; i>=0; i--) 
		_rtl8305s_smiWriteBit((data>>i)&0x1);
	_rtl8305s_smiZBit();
}

#endif


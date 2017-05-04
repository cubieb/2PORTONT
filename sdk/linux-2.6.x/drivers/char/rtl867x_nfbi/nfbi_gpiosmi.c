#include <linux/types.h>
#include <bspchip.h>
#include "nfbi_gpiosmi.h"
#include "../../arch/rlx/bsp/gpio.h"



#ifdef NFBI_GPIO_SIMULATE
#define GPIO_SMI_MDC		(GPIO_F_4)
#define GPIO_SMI_MDIO		(GPIO_F_3)
#define GPIO_SMI_MDCBIT		(1<<(GPIO_F_4-GPIO_E_0))
#define GPIO_SMI_MDIOBIT	(1<<(GPIO_F_3-GPIO_E_0))
#define GPIO_SMI_BITMASK	(~(GPIO_SMI_MDCBIT|GPIO_SMI_MDIOBIT))
#define PORT_DIR			(GPIO_PEFGH_DIR)
#define PORT_DAT			(GPIO_PEFGH_DAT)


/* Change clock to 1 */
static void smiZBit(void) 
{
	unsigned int i;

	REG32(PORT_DIR)= (REG32(PORT_DIR)&GPIO_SMI_BITMASK) | GPIO_SMI_MDCBIT;
	gpioClear(GPIO_SMI_MDC);
	//gpioClear(GPIO_SMI_MDIO);
	//for(i=0; i<25; i++);
}

/* Generate  1 -> 0 transition and sampled at 1 to 0 transition time */
static void smiReadBit(unsigned char * data) 
{
	unsigned int i;

	REG32(PORT_DIR)= (REG32(PORT_DIR)&GPIO_SMI_BITMASK) | GPIO_SMI_MDCBIT;
	gpioSet(GPIO_SMI_MDC); 
	//gpioClear(GPIO_SMI_MDIO);
	//for(i=0; i<25; i++);
	gpioClear(GPIO_SMI_MDC);
	//gpioClear(GPIO_SMI_MDIO);
	*data = (REG32(PORT_DAT)&(GPIO_SMI_MDIOBIT))?1:0;
}

/* Generate  0 -> 1 transition and put data ready during 0 to 1 whole period */
static void smiWriteBit(unsigned char data) 
{
	unsigned int i;
	
	REG32(PORT_DIR) = REG32(PORT_DIR)|GPIO_SMI_MDCBIT|GPIO_SMI_MDIOBIT;
	if(data) {/* Write 1 */
		gpioClear(GPIO_SMI_MDC);
		gpioSet(GPIO_SMI_MDIO);
		//for(i=0; i<25; i++);
		gpioSet(GPIO_SMI_MDC);
		//gpioSet(GPIO_SMI_MDIO);
	} else {
		gpioClear(GPIO_SMI_MDC);
		gpioClear(GPIO_SMI_MDIO);
		//for(i=0; i<25; i++);
		gpioSet(GPIO_SMI_MDC);
		//gpioClear(GPIO_SMI_MDIO);
	}
}

static void smiFreePort(void) 
{
	REG32(PORT_DIR)= REG32(PORT_DIR)&GPIO_SMI_BITMASK;
}

void NFBI_smiRead(unsigned char phyad, unsigned char regad, unsigned short * data) 
{
	int i;
	unsigned char readBit;

	/* 32 continuous 1 as preamble*/
	for(i=0; i<32; i++)
		smiWriteBit(1);
	/* ST: Start of Frame, <01>*/
	smiWriteBit(0);
	smiWriteBit(1);
	/* OP: Operation code, read is <10> */
	smiWriteBit(1);
	smiWriteBit(0);
	/* PHY Address */
	for(i=4; i>=0; i--) 
		smiWriteBit((phyad>>i)&0x1);
	/* Register Address */
	for(i=4; i>=0; i--) 
		smiWriteBit((regad>>i)&0x1);
	/* TA: Turnaround <z0> */
	smiZBit();
	smiReadBit(&readBit);
	/* Data */
	*data = 0;
	for(i=15; i>=0; i--) {
		smiReadBit(&readBit);
		*data = (*data<<1) | readBit;
	}
	/*add  an extra clock cycles for robust reading , ensure partner stop output signal
	and not affect the next read operation, because TA steal a clock*/     
	//smiWriteBit(1);
	//smiZBit();
	smiReadBit(&readBit);/*only mdc, don't care mdio*/
	smiFreePort();
}

void NFBI_smiWrite(unsigned char phyad, unsigned char regad, unsigned short data) 
{
	int i;

	/* 32 continuous 1 as preamble*/
	for(i=0; i<32; i++)
		smiWriteBit(1);
	/* ST: Start of Frame, <01>*/
	smiWriteBit(0);
	smiWriteBit(1);
	/* OP: Operation code, write is <01> */
	smiWriteBit(0);
	smiWriteBit(1);
	/* PHY Address */
	for(i=4; i>=0; i--) 
		smiWriteBit((phyad>>i)&0x1);
	/* Register Address */
	for(i=4; i>=0; i--) 
		smiWriteBit((regad>>i)&0x1);
	/* TA: Turnaround <10> */
	smiWriteBit(1);
	smiWriteBit(0);
	/* Data */
	for(i=15; i>=0; i--) 
		smiWriteBit((data>>i)&0x1);

	/*clear mdio to 0*/
	smiWriteBit(0);
	smiZBit();
	smiFreePort();
}
#endif /*NFBI_GPIO_SIMULATE*/


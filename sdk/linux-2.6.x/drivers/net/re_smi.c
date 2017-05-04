/*	
 *	re_smi.c: MDC/MDIO control
*/

#include <linux/types.h>
#include <linux/irqflags.h>
#include <asm/cpu-info.h>
#include <linux/delay.h>
#include "re867x.h"
#include "re_gpiosmi.h"

void internal_select_page(unsigned char page){
	unsigned int tmp=0;
	unsigned short value=0;

	local_irq_disable(); //cli();
	tmp=(1)<<26 | (31&0x1f)<<16;
	RTL_W32(MIIAR,tmp);
	do
	{
		udelay(100);
	}
	while (!(RTL_R32(MIIAR) & 0x80000000));
	value=RTL_R32(MIIAR)&0xfffc;
	value|=page;
	
	tmp=1<<31 | (1&0x1f)<<26 | (31&0x1f)<<16 | (value&0xffff);	
	RTL_W32(MIIAR,tmp);
	do
	{
		udelay(100);
	}
	while (RTL_R32(MIIAR) & 0x80000000);
	local_irq_enable();//sti();	
}

void internal_miiar_write(unsigned char phyaddr,unsigned char regaddr,unsigned short value){
	unsigned int tmp=0;

	local_irq_disable(); //cli();
	tmp=1<<31 | (phyaddr&0x1f)<<26 | (regaddr&0x1f)<<16 | (value&0xffff);	
	RTL_W32(MIIAR,tmp);
	do
	{
		udelay(100);
	}
	while (RTL_R32(MIIAR) & 0x80000000);
	local_irq_enable();//sti();	
}

void internal_miiar_read(unsigned char phyaddr,unsigned char regaddr,unsigned short *value){
	unsigned int tmp=0;

	local_irq_disable(); //cli();
	tmp=(phyaddr&0x1f)<<26 | (regaddr&0x1f)<<16;
	RTL_W32(MIIAR,tmp);
	do
	{
		udelay(100);
	}
	while (!(RTL_R32(MIIAR) & 0x80000000));
	*value=RTL_R32(MIIAR)&0xffff;
	local_irq_enable();//sti();	
}

void internal_miiar_WriteBit(unsigned char phyad, unsigned char regad, unsigned char bit, unsigned char value) {
	unsigned short regData;
	
	if(bit>=16)
		return;
	internal_miiar_read(phyad, regad, &regData);
	if(value) 
		regData = regData | (1<<bit);
	else
		regData = regData & ~(1<<bit);
	internal_miiar_write(phyad, regad, regData);
}

void ext_miiar_write(unsigned char phyaddr,unsigned char regaddr,unsigned short value){
	unsigned int tmp=0;

	local_irq_disable(); //cli();
	tmp=1<<31 | (phyaddr&0x1f)<<26 | (regaddr&0x1f)<<16 | (value&0xffff);	
	//RTL_W32(MIIAR,tmp);
	*(volatile unsigned int*)(0xb801005c)=tmp;
	do
	{
		mdelay(20);
	}
	while ((*(volatile unsigned int*)(0xb801005c)) & 0x80000000);
	local_irq_enable();//sti();	
}

void ext_miiar_read(unsigned char phyaddr,unsigned char regaddr,unsigned short *value){
	unsigned int tmp=0;

	local_irq_disable(); //cli();
	tmp=(phyaddr&0x1f)<<26 | (regaddr&0x1f)<<16;
	//RTL_W32(MIIAR,tmp);
	*(volatile unsigned int*)(0xb801005c)=tmp;
	do
	{
		mdelay(20);
	}
	while (!((*(volatile unsigned int*)(0xb801005c)) & 0x80000000));
	*value=(*(volatile unsigned int*)(0xb801005c) )&0xffff;
	local_irq_enable();//sti();	
}

void miiar_write(unsigned char phyaddr,unsigned char regaddr,unsigned short value){
	#if (! defined(GPIO_SIMULATE) && ! defined(CONFIG_INT_PHY))
	unsigned short tmp_value;
	#endif
#ifdef GPIO_SIMULATE
	local_irq_disable(); //cli();
	rtl8305s_smiWrite(phyaddr,regaddr,value);
	local_irq_enable();//sti();	
#else
	#if  CONFIG_INT_PHY
	// use MAC1(with internal PHY) MIIAR
	internal_miiar_write(phyaddr,regaddr,value);        
	#else
	// use MAC2(with MII to external switch) MIIAR
	ext_miiar_write(phyaddr,regaddr,value);
	// Kaohj -- patch for MAC2 mii polling: set phy id of MIIAR to cpu port
	ext_miiar_read(virt2phy[SWITCH_VPORT_TO_867X],0,&tmp_value);
	#endif
#endif	
}



void miiar_read(unsigned char phyaddr,unsigned char regaddr,unsigned short *value){
	#if (! defined(GPIO_SIMULATE) && ! defined(CONFIG_INT_PHY))
	unsigned short tmp_value;
	#endif
#ifdef GPIO_SIMULATE
	local_irq_disable(); //cli();
	rtl8305s_smiRead(phyaddr,regaddr,value);
	local_irq_enable();//sti();	
#else
	#if  CONFIG_INT_PHY
	// use MAC1(with internal PHY) MIIAR
	internal_miiar_read(phyaddr,regaddr,value);   
	#else
	// use MAC2(with MII to external switch) MIIAR
	ext_miiar_read(phyaddr,regaddr,value);
	// Kaohj -- patch for MAC2 mii polling: set phy id of MIIAR to cpu port
	ext_miiar_read(virt2phy[SWITCH_VPORT_TO_867X],0,&tmp_value);
	#endif
#endif	
}

void miiar_WriteBit(unsigned char phyad, unsigned char regad, unsigned char bit, unsigned char value) {
	unsigned short regData;
	
	if(bit>=16)
		return;
	miiar_read(phyad, regad, &regData);
	if(value) 
		regData = regData | (1<<bit);
	else
		regData = regData & ~(1<<bit);
	miiar_write(phyad, regad, regData);
}


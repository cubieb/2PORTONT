/*
* Copyright c                  Realtek Semiconductor Corporation, 2006 
* All rights reserved.
* 
* Program : Control  MDC/MDIO connected RTL8305Sx
* Abstract : 
* Author : Robin Zheng-bei Xing(robin_xing@realsil.com.cn)                
*  $Id: mdcmdio.c,v 1.1.1.1 2010/05/05 09:00:51 jiunming Exp $
*/
/*	@doc MDCMDIO_API

	@module mdcmdio.c -  MDC/MDIO API documentation	|
	This document explains API to use two GPIO pin  to simulate MDC/MDIO signal which is 
	based on RTL8651B platform, if you use other platform, please modify this file.	
	@normal 

	Copyright <cp>2006 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

 	@head3 List of Symbols |
 	Here is a list of all functions and variables in this module.

 	@index | MDCMDIO_API

*/
#include "Rtl8306_types.h"
#include "mdcmdio.h"      /*RTL8651B file*/
// Kaohj -- miiar_read() and miiar_write()
#include "../re_smi.h"

/*
@func int32 | smiRead | read data from phy register
@parm uint32 | phyad | PHY address (0~31)
@parm uint32 | regad | Register address (0 ~31)
@parm uint32* | data | Register value
@comm 
This function could read register through MDC/MDIO serial 
interface, and it is platform  related. It use two GPIO pins 
to simulate MDC/MDIO timing. MDC is sourced by the Station Management 
entity to the PHY as the timing reference for transfer of information
on the MDIO signal. MDC is an aperiodic signal that has no maximum high 
or low times. The minimum high and low times for MDC shall be 160 ns each, 
and the minimum period for MDC shall be 400 ns. Obeying frame format defined
by IEEE802.3 standard, you could access Phy registers. If you want to 
port it to other CPU, please modify static functions which are called 
by this function.
*/

int32 smiRead(uint32 phyad, uint32 regad, uint32 * data) 
{
	uint16 mydata;
	miiar_read(phyad , (unsigned char)regad ,(unsigned short*)&mydata); 
	*data = (uint32)mydata;
	return	SUCCESS;
}

/*
@func int32 | smiWrite | Write data to Phy register
@parm uint32 | phyad | Phy address (0~31)
@parm uint32 | regad | Register address (0~31)
@parm uint32 | data | data to be written into Phy register
@comm 
This function could write register through MDC/MDIO serial 
interface, and it is platform  related. It use two GPIO pins 
to simulate MDC/MDIO timing. MDC is sourced by the Station Management 
entity to the PHY as the timing reference for transfer of information
on the MDIO signal. MDC is an aperiodic signal that has no maximum high 
or low times. The minimum high and low times for MDC shall be 160 ns each, 
and the minimum period for MDC shall be 400 ns. Obeying frame format defined
by IEEE802.3 standard, you could access Phy registers. If you want to 
port it to other CPU, please modify static functions which are called 
by this function.
*/

int32 smiWrite(uint32 phyad, uint32 regad, uint32 data)
{
	miiar_write(phyad , (unsigned char)regad , (unsigned short)data);
	return 	SUCCESS;	
}


/*
int32 | smiInit | Init Rtl8651B smi interface
uint32 | port | Specify Rtl8651B GPIO port
uint32 | pinMDC | Set which gpio pin as MDC 
uint32 | pinMDIO | Set which gpio pin as MDIO
This function is only for Rtl8651B, use it to specify
GPIO pins as MDC/MDIO signal. It should be called at first.
*/
#if 0
int32 smiInit(uint32 port, uint32 pinMDC, uint32 pinMDIO)
{
	uint32 gpioId;
	int32 res;

	/* Initialize GPIO smi_MDC  as SMI MDC signal */
	gpioId = GPIO_ID(port, pinMDC);
	res = _rtl865x_initGpioPin(gpioId, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	if (res != SUCCESS)
		return res;
	smi_MDC = gpioId;

	/* Initialize GPIO smi_MDIO  as SMI MDIO signal */
	gpioId = GPIO_ID(port, pinMDIO);
	res = _rtl865x_initGpioPin(gpioId, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);
	if (res != SUCCESS)
		return res;
	smi_MDIO = gpioId;
	return SUCCESS;
	
}
#endif

/*
@func int32 | smiReadBit | read one bit of PHY register
@parm uint32 | phyad | Phy address (0~31)
@parm uint32 | regad | Register address (0~31)
@parm uint32 | bit | Register bit (0~15)
@parm uint32* | pdata | Bit value
*/
int32 smiReadBit(uint32 phyad, uint32 regad, uint32 bit, uint32 * pdata) 
{
	uint32 regData;

	if ((phyad > 31) || (regad > 31) || (bit > 15) || (pdata == NULL) )  return	FAILED;
	if(bit>=16)
		* pdata = 0;
	else {
		smiRead(phyad, regad, &regData);
		if(regData & (1<<bit)) 
			* pdata = 1;
		else
			* pdata = 0;
	}
	
	return SUCCESS;
}

/*
@func int32 | smiWriteBit | read one bit of PHY register
@parm uint32 | phyad | Phy address (0~31)
@parm uint32 | regad | Register address (0~31)
@parm uint32 | bit | Register bit (0~15)
@parm uint32 | data | Bit value to be written
*/

int32 smiWriteBit(uint32 phyad, uint32 regad, uint32 bit, uint32 data) 
{
	uint32 regData;
	
	if ((phyad > 31) || (regad > 31) || (bit > 15) || (data > 1) ) 	return	FAILED;
	smiRead(phyad, regad, &regData);
	if(data) 
		regData = regData | (1<<bit);
	else
		regData = regData & ~(1<<bit);
	smiWrite(phyad, regad, regData);
	
	return SUCCESS;
}

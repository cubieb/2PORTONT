/*
* Copyright (C) 2010 Realtek Semiconductor Corp.
* All Rights Reserved.
*
* This program is the proprietary software of Realtek Semiconductor
* Corporation and/or its licensors, and only be used, duplicated,
* modified or distributed under the authorized license from Realtek.
*
* ANY USE OF THE SOFTWARE OTEHR THAN AS AUTHORIZED UNDER 
* THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
* 
* $Revision: 1.1 $ 
* $Date: 2011/03/03 08:39:16 $
*
* Purpose : MDC/MDIO example code
*
* Feature :  This file consists of following modules:
*                
*
*/


//#include "rtl_types.h"
//#include "gpio.h"
#include "rtl8306e_types.h"
#include "mdcmdio.h"      /*RTL8651B file*/
//#include "asicRegs.h"
//#include "rtl8306e_types.h"
#include "../re_smi.h"



/* Function Name:
 *      smiRead
 * Description:
 *      Read data from phy register
 * Input:
 *      phyad   - PHY address (0~31)
 *      regad   -  Register address (0 ~31) 
 * Output:
 *      data    -  Register value 
 * Return:
 *      SUCCESS         -  Success
 *      FAILED            -  Failure
 * Note:
 *     This function could read register through MDC/MDIO serial 
 *     interface, and it is platform  related. It use two GPIO pins 
 *     to simulate MDC/MDIO timing. MDC is sourced by the Station Management 
 *     entity to the PHY as the timing reference for transfer of information
 *     on the MDIO signal. MDC is an aperiodic signal that has no maximum high 
 *     or low times. The minimum high and low times for MDC shall be 160 ns each, 
 *     and the minimum period for MDC shall be 400 ns. Obeying frame format defined
 *     by IEEE802.3 standard, you could access Phy registers. If you want to 
 *     port it to other CPU, please modify static functions which are called 
 *      by this function.
 */
int32 smiRead(uint32 phyad, uint32 regad, uint32 * data) 
{
    uint16 mydata;
    miiar_read(phyad , (unsigned char)regad ,(unsigned short*)&mydata); 
    *data = (uint32)mydata;
    
    return  SUCCESS;
}

/* Function Name:
 *      smiWrite
 * Description:
 *      Write data to Phy register
 * Input:
 *      phyad   - PHY address (0~31)
 *      regad   -  Register address (0 ~31)
 *      data    -  Data to be written into Phy register
 * Output:
 *      none
 * Return:
 *      SUCCESS         -  Success
 *      FAILED            -  Failure
 * Note:
 *     This function could read register through MDC/MDIO serial 
 *     interface, and it is platform  related. It use two GPIO pins 
 *     to simulate MDC/MDIO timing. MDC is sourced by the Station Management 
 *     entity to the PHY as the timing reference for transfer of information
 *     on the MDIO signal. MDC is an aperiodic signal that has no maximum high 
 *     or low times. The minimum high and low times for MDC shall be 160 ns each, 
 *     and the minimum period for MDC shall be 400 ns. Obeying frame format defined
 *     by IEEE802.3 standard, you could access Phy registers. If you want to 
 *     port it to other CPU, please modify static functions which are called 
*      by this function.
 */

int32 smiWrite(uint32 phyad, uint32 regad, uint32 data)
{
    miiar_write(phyad , (unsigned char)regad , (unsigned short)data);
    return SUCCESS; 
}


/* Function Name:
 *      smiInit
 * Description:
 *      Init Rtl8651B smi interface
 * Input:
 *      port        - Specify Rtl8651B GPIO port
 *      pinMDC    - Set which gpio pin as MDC 
 *      pinMDIO   - Set which gpio pin as MDIO
 * Output:
 *      none
 * Return:
 *      SUCCESS         -  Success
 *      FAILED            -  Failure
 * Note:
 *      This function is only for Rtl8651B, use it to specify
 *      GPIO pins as MDC/MDIO signal. It should be called at first.
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

/* Function Name:
 *      smiReadBit
 * Description:
 *      Read one bit of PHY register
 * Input:
 *      phyad   - PHY address (0~31)
 *      regad   -  Register address (0 ~31) 
 *      bit       -  Register bit (0~15)   
 * Output:
 *      pdata    - the pointer of  Register bit value 
 * Return:
 *      SUCCESS         -  Success
 *      FAILED            -  Failure
 * Note:
 */

int32 smiReadBit(uint32 phyad, uint32 regad, uint32 bit, uint32 * pdata) 
{
    uint32 regData;

    if ((phyad > 31) || (regad > 31) || (bit > 15) || (pdata == NULL) ) 
        return  FAILED;
    
    if(bit>=16)
        * pdata = 0;
    else 
    {
        smiRead(phyad, regad, &regData);
        if(regData & (1<<bit)) 
            * pdata = 1;
        else
            * pdata = 0;
    }
    return SUCCESS;
}

/* Function Name:
 *      smiWriteBit
 * Description:
 *      Write one bit of PHY register
 * Input:
 *      phyad   - PHY address (0~31)
 *      regad   -  Register address (0 ~31) 
 *      bit       -  Register bit (0~15)   
 *      data     -  Bit value to be written
 * Output:
 *      none
 * Return:
 *      SUCCESS         -  Success
 *      FAILED            -  Failure
 * Note:
 */

int32 smiWriteBit(uint32 phyad, uint32 regad, uint32 bit, uint32 data) 
{
    uint32 regData;
    
    if ((phyad > 31) || (regad > 31) || (bit > 15) || (data > 1) ) 
        return  FAILED;
    smiRead(phyad, regad, &regData);
    if(data) 
        regData = regData | (1<<bit);
    else
        regData = regData & ~(1<<bit);
    smiWrite(phyad, regad, regData);
    return SUCCESS;
}





















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
* $Date: 2011/03/03 08:28:10 $
*
* Purpose : asic-level driver implementation for RTL8306E switch
*
*  Feature :  This file consists of following modules:
*                1) 
*
*/
#if 0 //shlee, for one-armed router
#include <stdio.h>
#include <rtl8306e_types.h>
#include <rtl8306e_asicdrv.h>
#ifndef RTK_X86_ASICDRV
#include <mdcmdio.h>
#endif
#else
//#include <stdio.h>
#include "Rtl8306_types.h"
#include "rtl8306e_asicdrv.h"
#ifndef RTK_X86_ASICDRV
#include "mdcmdio.h"
#endif

#endif

/* Function Name:
 *      rtl8306e_reg_set
 * Description:
 *      Write Asic Register
 * Input:
 *      phyad   - Specify Phy address (0 ~6)
 *      regad    - Specify register address (0 ~31)
 *      npage   - Specify page number (0 ~3)
 *      value    - Value to be write into the register
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function you could write all configurable registers of RTL8306, 
 *      it is realized by calling functions smiRead and smiWrite which are switch
 *      MDC/MDIO interface access functions. Those two functions use two GPIO 
 *      pins to simulate MDC/MDIO timing,  and they are based on rtl8651b platform,
 *      to modify them,  you can port all asic API to other platform.
 */

int32 rtl8306e_reg_set(uint32 phyad, uint32 regad, uint32 npage, uint32 value) 
{

#ifdef RTK_X86_ASICDRV
    uint32 rdata; 

    if ((phyad >= RTL8306_PHY_NUMBER) || (npage >= RTL8306_PAGE_NUMBER))
        return FAILED;
    
    /* Select PHY Register Page through configuring PHY 0 Register 16 [bit1 bit15] */
    value = value & 0xFFFF;
    rdata = r_phy(8,0, 16); 
    switch (npage) 
    {
        case RTL8306_REGPAGE0:
            w_phy(8,0, 16, (rdata & 0x7FFF) | 0x0002);
            break;
        case RTL8306_REGPAGE1:
            w_phy(8,0, 16, rdata | 0x8002 );
            break;
        case RTL8306_REGPAGE2:
            w_phy(8,0, 16, rdata & 0x7FFD);
            break;
        case RTL8306_REGPAGE3:
            w_phy(8,0, 16, (rdata & 0xFFFD) | 0x8000);
            break;
        case RTL8306_REGPAGE4:
            rdata = r_phy(8, 5, 16);             
            rdata |= 0x2;
            w_phy(8, 5, 16, rdata);
            break;
        default:
            return FAILED;
    }
    
    w_phy(8,phyad, regad, value);

    if(RTL8306_REGPAGE4 == npage)
    {
        rdata = r_phy(8, 5, 16);             
        rdata &= ~0x2;
        w_phy(8, 5, 16, rdata);
    }
    
    return SUCCESS;

#else
    uint32 rdata; 

    if ((phyad >= RTL8306_PHY_NUMBER) || (npage >= RTL8306_PAGE_NUMBER))
        return FAILED;
    
    /* Select PHY Register Page through configuring PHY 0 Register 16 [bit1 bit15] */
    value = value & 0xFFFF;
    smiRead(0, 16, &rdata); 
    switch (npage) 
    {
        case RTL8306_REGPAGE0:
            smiWrite(0, 16, (rdata & 0x7FFF) | 0x0002);
            break;
        case RTL8306_REGPAGE1:
            smiWrite(0, 16, rdata | 0x8002 );
            break;
        case RTL8306_REGPAGE2:
            smiWrite(0, 16, rdata & 0x7FFD);
            break;
        case RTL8306_REGPAGE3:
            smiWrite(0, 16, (rdata & 0xFFFD) | 0x8000);
            break;
        case RTL8306_REGPAGE4:
            smiRead(5, 16, &rdata);
            rdata |= 0x2;
            smiWrite(5, 16, rdata);
            break;                        
        default:
            return FAILED;
    }

    smiWrite(phyad, regad, value);
    
    if (RTL8306_REGPAGE4)
    {
        /*exit page 4*/
        smiRead(5, 16, &rdata);
        rdata &= ~0x2;
        smiWrite(5, 16, rdata);        
    }
    
    return SUCCESS;
    
#endif
}


/* Function Name:
 *      rtl8306e_reg_get
 * Description:
 *      Read Asic Register
 * Input:
 *      phyad   - Specify Phy address (0 ~6)
 *      regad    - Specify register address (0 ~31)
 *      npage   - Specify page number (0 ~3)
 * Output:
 *      pvalue    - The pointer of value read back from register
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function you could write all configurable registers of RTL8306, 
 *      it is realized by calling functions smiRead and smiWrite which are switch
 *      MDC/MDIO interface access functions. Those two functions use two GPIO 
 *      pins to simulate MDC/MDIO timing,  and they are based on rtl8651b platform,
 *      to modify them,  you can port all asic API to other platform.
 */
 
int32 rtl8306e_reg_get(uint32 phyad, uint32 regad, uint32 npage, uint32 *pvalue)
{

#ifdef RTK_X86_ASICDRV
    uint32 rdata;

    if ((phyad >= RTL8306_PHY_NUMBER) ||(npage >= RTL8306_PAGE_NUMBER))
        return FAILED;

    /* Select PHY Register Page through configuring PHY 0 Register 16 [bit1 bit15] */
    rdata = r_phy(8,0, 16); 
    switch (npage) 
    {
        case RTL8306_REGPAGE0:
            w_phy(8,0, 16, (rdata & 0x7FFF) | 0x0002);
            break;
        case RTL8306_REGPAGE1:
            w_phy(8,0, 16, rdata | 0x8002 );
            break;
        case RTL8306_REGPAGE2:
            w_phy(8,0, 16, rdata & 0x7FFD);
            break;
        case RTL8306_REGPAGE3:
            w_phy(8,0, 16, (rdata & 0xFFFD) | 0x8000);
            break;
        case RTL8306_REGPAGE4:
            rdata = r_phy(8, 5, 16);             
            rdata |= 0x2;
            w_phy(8, 5, 16, rdata);    
            break;
        default:
            return FAILED;
    }

    rdata = r_phy(8,phyad, regad);
    *pvalue = rdata & 0xFFFF;

    if(RTL8306_REGPAGE4 == npage)
    {
        rdata = r_phy(8, 5, 16);             
        rdata &= ~0x2;
        w_phy(8, 5, 16, rdata);
    }
    
    return SUCCESS;

#else
    uint32 rdata;

    if ((phyad >= RTL8306_PHY_NUMBER) || (npage >= RTL8306_PAGE_NUMBER))
        return FAILED;

    /* Select PHY Register Page through configuring PHY 0 Register 16 [bit1 bit15] */
    smiRead(0, 16, &rdata); 
    switch (npage) 
    {
        case RTL8306_REGPAGE0:
            smiWrite(0, 16, (rdata & 0x7FFF) | 0x0002);
            break;
        case RTL8306_REGPAGE1:
            smiWrite(0, 16, rdata | 0x8002 );
            break;
        case RTL8306_REGPAGE2:
            smiWrite(0, 16, rdata & 0x7FFD);
            break;
        case RTL8306_REGPAGE3:
            smiWrite(0, 16, (rdata & 0xFFFD) | 0x8000);
            break;
        case RTL8306_REGPAGE4:
            smiRead(5, 16, &rdata);
            rdata |= 0x2;
            smiWrite(5, 16, rdata);            
            break;
        default:
            return FAILED;
    }

    smiRead(phyad, regad, pvalue);

    *pvalue = *pvalue & 0xFFFF;
    if (RTL8306_REGPAGE4)
    {
        /*exit page 4*/
        smiRead(5, 16, &rdata);
        rdata &= ~0x2;
        smiWrite(5, 16, rdata);        
    }
        
    return SUCCESS;
#endif
}


/* Function Name:
 *      rtl8306e_regbit_set
 * Description:
 *      Write one bit of Asic Register
 * Input:
 *      phyad   - Specify Phy address (0 ~6)
 *      regad    - Specify register address (0 ~31)
 *      bit        - Specify bit position(0 ~ 15)
 *      npage   - Specify page number (0 ~3)
 *      value    - Value to be write(0, 1)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function  you could write each bit of  all configurable registers of RTL8306.
 */

int32 rtl8306e_regbit_set(uint32 phyad, uint32 regad, uint32 bit, uint32 npage,  uint32 value) 
{
    uint32 rdata;
    
    if ((phyad >= RTL8306_PHY_NUMBER) || (npage >= RTL8306_PAGE_NUMBER) ||
        (bit > 15) || (value > 1))
        return FAILED;
    
    rtl8306e_reg_get(phyad, regad,  npage, &rdata);
    if (value) 
        rtl8306e_reg_set(phyad, regad, npage, rdata | (1 << bit));
    else
        rtl8306e_reg_set(phyad, regad, npage, rdata & (~(1 << bit)));
    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_regbit_get
 * Description:
 *      Read one bit of Asic  PHY Register
 * Input:
 *      phyad   - Specify Phy address (0 ~6)
 *      regad    - Specify register address (0 ~31)
 *      bit        - Specify bit position(0 ~ 15)
 *      npage   - Specify page number (0 ~3)
 * Output:
 *      pvalue  - The pointer of value read back
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function you could read each bit of  all configurable registers of RTL8306
 */

int32 rtl8306e_regbit_get(uint32 phyad, uint32 regad, uint32 bit, uint32 npage,  uint32 * pvalue) 
{
    uint32 rdata;

    if ((phyad >= RTL8306_PHY_NUMBER) || (npage >= RTL8306_PAGE_NUMBER) || 
        (bit > 15) || (pvalue == NULL))
        return FAILED;
    
    rtl8306e_reg_get(phyad, regad, npage, &rdata);
    if (rdata & (1 << bit))
        *pvalue =1;
    else 
        *pvalue =0;

    return SUCCESS;
}

int32 rtl8306e_asic_init(void)
{
    /*by default: enable xor lut hash algorithm*/
    rtl8306e_regbit_set(1, 23, 12, 0, 1);   

    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_switch_maxPktLen_set
 * Description:
 *      set Max packet length which could be forwarded by
 * Input:
 *      maxLen         -  max packet length
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      maxLen could be set : 
 *      RTL8306_MAX_PKT_LEN_1518 -1518 bytes without any tag; 1522 bytes: 
 *              with VLAN tag or CPU tag, 1526 bytes with CPU and VLAN tag;
 *      RTL8306_MAX_PKT_LEN_1536 - 1536 bytes (all tags counted);
 *      RTL8306_MAX_PKT_LEN_1552 - 1552 bytes (all tags counted); 
 *      RTL8306_MAX_PKT_LEN_2000 - 2000 bytes (all tags counted) 
 *              
 */ 

int32 rtl8306e_switch_maxPktLen_set(uint32 maxLen)
{
    switch(maxLen)
    {
        case RTL8306_MAX_PKT_LEN_1518:
            rtl8306e_regbit_set(2, 23, 1, 3, 1);
            rtl8306e_regbit_set(0, 29, 15, 3, 0);
            break;

        case RTL8306_MAX_PKT_LEN_1536:
            rtl8306e_regbit_set(2, 23, 1, 3, 0);
            rtl8306e_regbit_set(0, 18, 14, 0, 1);
            rtl8306e_regbit_set(0, 29, 15, 3, 0);            
            break;

        case RTL8306_MAX_PKT_LEN_1552:
            rtl8306e_regbit_set(2, 23, 1, 3, 0);
            rtl8306e_regbit_set(0, 18, 14, 0, 0);
            rtl8306e_regbit_set(0, 29, 15, 3, 0);            
            break;

        case RTL8306_MAX_PKT_LEN_2000:
            rtl8306e_regbit_set(0, 29, 15, 3, 1);
            break;
            
        default:
            return FAILED;

   }

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_switch_maxPktLen_get
 * Description:
 *      set Max packet length which could be forwarded by
 * Input:
 *      none
 * Output:
 *      maxLen         -  max packet length
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      maxLen could be set : 
 *      RTL8306_MAX_PKT_LEN_1518 -1518 bytes without any tag; 1522 bytes: 
 *              with VLAN tag or CPU tag, 1526 bytes with CPU and VLAN tag;
 *      RTL8306_MAX_PKT_LEN_1536 - 1536 bytes (all tags counted);
 *      RTL8306_MAX_PKT_LEN_1552 - 1552 bytes (all tags counted); 
 *      RTL8306_MAX_PKT_LEN_2000 - 2000 bytes (all tags counted) 
 *              
 */ 
int32 rtl8306e_switch_maxPktLen_get(uint32 *pMaxLen)
{
    uint32 regval;

    if (pMaxLen == NULL)
        return FAILED;
    
    rtl8306e_regbit_get(0, 29, 15, 3, &regval);
    if(regval)
    {
        *pMaxLen = RTL8306_MAX_PKT_LEN_2000;
    }
    else
    {
        rtl8306e_regbit_get(2, 23, 1, 3, &regval);
        if (regval)
        {
            *pMaxLen = RTL8306_MAX_PKT_LEN_1518;
        }
        else 
        {
            rtl8306e_regbit_get(0, 18, 14, 0, &regval);
            if (regval)
            {
                *pMaxLen = RTL8306_MAX_PKT_LEN_1536;            
            }
            else
            {
                *pMaxLen = RTL8306_MAX_PKT_LEN_1552;                        
            }
        }
    }
    
    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_port_etherPhy_set
 * Description:
 *      Configure PHY setting
 * Input:
 *      phy                    - Specify the phy to configure
 *      autoNegotiation    - Specify whether enable auto-negotiation
 *      advCapability       - When auto-negotiation is enabled, specify the advertised capability
 *      speed                 - When auto-negotiation is disabled, specify the force mode speed
 *      fullDuplex            - When auto-negotiatoin is disabled, specify the force mode duplex mode
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      When auto-negotiation is enabled, the advertisement capability is used to handshaking with link partner.
 *      When auto-negotiation is disabled, the phy is configured into force mode and the speed and duplex mode 
 *      setting is based on speed and fullDuplex setting.Port number should be smaller than RTL8306_PHY_NUMBER.
 *      AdverCapability should be ranged between RTL8306_ETHER_AUTO_100FULL and RTL8306_ETHER_AUTO_10HALF.
 *      Speed should be either RTL8306_ETHER_SPEED_100 or RTL8306_ETHER_SPEED_10.
 */
int32 rtl8306e_port_etherPhy_set(uint32 phy, uint32 autoNegotiation, uint32 advCapability, uint32 speed, uint32 fullDuplex) 
{
    uint32 ctrlReg;

    if(phy >= RTL8306_PHY_NUMBER || 
       advCapability < RTL8306_ETHER_AUTO_100FULL ||
       advCapability > RTL8306_ETHER_AUTO_10HALF ||
       (speed != 100 && speed != 10))
           return FAILED;

    if(RTL8306_ETHER_AUTO_100FULL == advCapability)
        rtl8306e_reg_set(phy, 4, 0, RTL8306_CAPABLE_PAUSE | RTL8306_CAPABLE_100BASE_TX_FD 
                                  | RTL8306_CAPABLE_100BASE_TX_HD | RTL8306_CAPABLE_10BASE_TX_FD 
                                  | RTL8306_CAPABLE_10BASE_TX_HD | 0x1);
    else if(RTL8306_ETHER_AUTO_100HALF == advCapability)
        rtl8306e_reg_set(phy, 4, 0, RTL8306_CAPABLE_PAUSE | RTL8306_CAPABLE_100BASE_TX_HD
                                  | RTL8306_CAPABLE_10BASE_TX_FD | RTL8306_CAPABLE_10BASE_TX_HD | 0x1);
    else if( RTL8306_ETHER_AUTO_10FULL == advCapability)
        rtl8306e_reg_set(phy, 4, 0, RTL8306_CAPABLE_PAUSE | RTL8306_CAPABLE_10BASE_TX_FD 
                                  | RTL8306_CAPABLE_10BASE_TX_HD | 0x1);
    else if(RTL8306_ETHER_AUTO_10HALF == advCapability)
        rtl8306e_reg_set(phy, 4, 0, RTL8306_CAPABLE_PAUSE | RTL8306_CAPABLE_10BASE_TX_HD | 0x1);

    /*Each time the link ability of the RTL8306 is reconfigured, 
     *the auto-negotiation process should be executed to allow
     *the configuration to take effect. 
     */
    if(TRUE == autoNegotiation) 
        ctrlReg = RTL8306_ENABLE_AUTONEGO | RTL8306_RESTART_AUTONEGO; 
    else
        ctrlReg = 0;
    if(100 == speed) 
        ctrlReg |= RTL8306_SPEED_SELECT_100M;
    if(TRUE == fullDuplex)
        ctrlReg |= RTL8306_SELECT_FULL_DUPLEX;
    rtl8306e_reg_set(phy, 0, RTL8306_REGPAGE0, ctrlReg);

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_port_etherPhy_get
 * Description:
 *       Get PHY setting
 * Input:
 *      phy                    - Specify the phy to configure
 * Output:
 *      pAutoNegotiation    - Get whether auto-negotiation is enabled
 *      pAdvCapability       - When auto-negotiation is enabled, Get the advertised capability
 *      pSpeed                 - When auto-negotiation is disabled, Get the force mode speed
 *      pFullDuplex            - When auto-negotiatoin is disabled, Get the force mode duplex mode

 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      When auto-negotiation is enabled, the advertisement capability is used to handshaking with link partner.
 *      When auto-negotiation is disabled, the phy is configured into force mode and the speed and duplex mode 
 *      setting is based on speed and fullDuplex setting.Port number should be smaller than RTL8306_PHY_NUMBER.
 *      AdverCapability should be ranged between RTL8306_ETHER_AUTO_100FULL and RTL8306_ETHER_AUTO_10HALF.
 *      Speed should be either RTL8306_ETHER_SPEED_100 or RTL8306_ETHER_SPEED_10.
 */

int32 rtl8306e_port_etherPhy_get(uint32 phy, uint32 *pAutoNegotiation, uint32 *pAdvCapability, uint32 *pSpeed, uint32 *pFullDuplex)
{
    uint32 regData;

    if((phy >= RTL8306_PHY_NUMBER) || (NULL == pAutoNegotiation) || (NULL == pAdvCapability)
        || (NULL ==  pSpeed) || (NULL == pFullDuplex))
        return FAILED;

    rtl8306e_reg_get(phy, 0, RTL8306_REGPAGE0, &regData);
    *pAutoNegotiation = (regData & RTL8306_ENABLE_AUTONEGO) ? TRUE: FALSE;
    *pSpeed = (regData & RTL8306_SPEED_SELECT_100M) ? 100: 10;
    *pFullDuplex = (regData & RTL8306_SELECT_FULL_DUPLEX) ? TRUE: FALSE;

    rtl8306e_reg_get(phy, 4, RTL8306_REGPAGE0, &regData);
    if(regData & RTL8306_CAPABLE_100BASE_TX_FD)
        *pAdvCapability = RTL8306_ETHER_AUTO_100FULL;
    else if(regData & RTL8306_CAPABLE_100BASE_TX_HD)
        *pAdvCapability = RTL8306_ETHER_AUTO_100HALF;
    else if(regData & RTL8306_CAPABLE_10BASE_TX_FD)
        *pAdvCapability = RTL8306_ETHER_AUTO_10FULL;
    else if(regData & RTL8306_CAPABLE_10BASE_TX_HD)
        *pAdvCapability = RTL8306_ETHER_AUTO_10HALF;
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_port_port5LinkStatus_set
 * Description:
 *      Force port 5 link up or link down
 * Input:
 *      enabled   - true or false
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Port 5 should be manully enable / disable
 */
int32 rtl8306e_port_port5LinkStatus_set(uint32 enabled) 
{
    uint32 duplex, speed, nway;

    /*save phy 6 reg 0.13, 0.12, 0.8*/
    rtl8306e_regbit_get(6, 0, 13, 0, &speed);
    rtl8306e_regbit_get(6, 0, 12, 0, &nway);
    rtl8306e_regbit_get(6, 0, 8, 0, &duplex);
      
    rtl8306e_regbit_set(6, 22, 15, 0, enabled ? 1:0);
    
    /*restore phy 6 reg 0.13, 0.12, 0.8*/
    rtl8306e_regbit_set(6, 0, 13, 0, speed);
    rtl8306e_regbit_set(6, 0, 12, 0, nway);
    rtl8306e_regbit_set(6, 0, 8, 0, duplex);

    return SUCCESS;       
}

/* Function Name:
 *      rtl8306e_port_port5LinkStatus_get
 * Description:
 *      get port 5 link status
 * Input:
 *      none
 * Output:
*      enabled   - true or false
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Port 5 should be manully enable / disable
 */

int32 rtl8306e_port_port5LinkStatus_get(uint32 *pEnabled)
{
    uint32 bitValue;

    if (NULL == pEnabled) 
        return FAILED;
    
    rtl8306e_regbit_get(6, 22, 15, 0, &bitValue);
    *pEnabled = (bitValue ? TRUE : FALSE);
    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_port_phyLinkStatus_get
 * Description:
 *      Get PHY Link Status
 * Input:
*      phy        - Specify the phy 
 * Output:
*      plinkUp   - Describe whether link status is up or not
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *       Read the link status of PHY register 1
 */

int32 rtl8306e_port_phyLinkStatus_get(uint32 phy, uint32 *plinkUp) 
{
    uint32 bitValue;

    if (NULL == plinkUp)
        return FAILED;
    
    rtl8306e_regbit_get(phy, 1, 2, RTL8306_REGPAGE0, &bitValue);
    rtl8306e_regbit_get(phy, 1, 2, RTL8306_REGPAGE0, &bitValue);    
    *plinkUp = (bitValue ? TRUE: FALSE);

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_port_phyAutoNegotiationDone_get
 * Description:
 *      Get PHY auto-negotiation result status
 * Input:
 *      phy      - Specify the phy to get status
 * Output:
*      pDone   -  Describe whether auto-negotiation is done or not
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Read the auto-negotiation complete of PHY register 1.
 */

int32 rtl8306e_port_phyAutoNegotiationDone_get(uint32 phy, uint32 *pDone) 
{
    uint32 bitValue;

    if (NULL == pDone)
        return FAILED;
    
    rtl8306e_regbit_get(phy, 1, 5, 0, &bitValue);
    *pDone = (bitValue ? TRUE: FALSE);
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_port_phyLoopback_set
 * Description:
 *       Set PHY loopback
 * Input:
 *      phy         - Specify the phy to configure
 *      enabled   - Enable phy loopback
 * Output:
 *      none      
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Only phy 0~4 could be configured the phy loopback
 */

int32 rtl8306e_port_phyLoopback_set(uint32 phy, uint32 enabled) 
{
    if(phy > 4)
        return FAILED;
    rtl8306e_regbit_set(phy, 0, 14, 0, enabled ? 1 : 0);
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_port_phyLoopback_get
 * Description:
 *      Get PHY loopback setting
 * Input:
 *      phy         - Specify the phy to get status
 * Output:
 *      pEnabled  -  phy loopback setting
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *
 */

int32 rtl8306e_port_phyLoopback_get(uint32 phy, uint32 *pEnabled) 
{
    uint32 bitValue;

    if (NULL == pEnabled)
        return FAILED;
    
    rtl8306e_regbit_get(phy, 0, 14, 0, &bitValue);
    *pEnabled = (bitValue ? TRUE: FALSE);
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_portLearningAbility_set
 * Description:
 *      Enable/disable physical port learning ability
 * Input:
 *      port        - Specify port number (0 ~ 5)
 * Output:
 *      enabled -  TRUE or FALSE
 * Return:
 *      SUCCESS
 * Note:
 *
 */
int32 rtl8306e_portLearningAbility_set(uint32 port, uint32 enabled)
{
    if (port > RTL8306_PORT5)
        return FAILED;
    if (RTL8306_PORT5 == port)
        port++;

    rtl8306e_regbit_set(port, 24, 9, 0, enabled ? 1 : 0);

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_port_isolation_set
 * Description:
 *      set port isolation 
 * Input:
 *      isomsk    - port isolation port mask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      each bit of isomsk determine two port's isolation,
 *      1 means two port could not forward packet between
 *      each other.
 *      bit 0  - Port0 & Port1 
 *      bit 1  - Port0 & Port2 
 *      bit 2  - Port0 & Port3
 *      bit 3  - Port0 & Port4
 *      bit 4  - Port0 & Port5
 *      bit 5  - Port1 & Port2
 *      bit 6  - Port1 & Port3
 *      bit 7  - Port1 & Port4
 *      bit 8  - Port1 & Port5
 *      bit 9  - Port2 & Port3
 *      bit 10 - Port2 & Port4
 *      bit 11 - Port2 & Port5
 *      bit 12 - Port3 & Port4
 *      bit 13 - Port3 & Port5
 *      bit 14 - Port4 & Port5
 */

int32 rtl8306e_port_isolation_set(uint32 isomsk)
{
    uint32 regval;

    rtl8306e_reg_get(0, 29, 3, &regval);
    regval &= ~0x7FFF;
    regval |= isomsk & 0x7FFF;
    rtl8306e_reg_set(0, 29, 3, regval);
    
    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_port_isolation_set
 * Description:
 *      set port isolation 
 * Input:
 *      none
 * Output:
 *      pIsomsk    -  the pointer of port isolation port mask
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      each bit of isomsk determine two port's isolation,
 *      1 means two port could not forward packet between
 *      each other.
 *      bit 0  - Port0 & Port1 
 *      bit 1  - Port0 & Port2 
 *      bit 2  - Port0 & Port3
 *      bit 3  - Port0 & Port4
 *      bit 4  - Port0 & Port5
 *      bit 5  - Port1 & Port2
 *      bit 6  - Port1 & Port3
 *      bit 7  - Port1 & Port4
 *      bit 8  - Port1 & Port5
 *      bit 9  - Port2 & Port3
 *      bit 10 - Port2 & Port4
 *      bit 11 - Port2 & Port5
 *      bit 12 - Port3 & Port4
 *      bit 13 - Port3 & Port5
 *      bit 14 - Port4 & Port5
 */

int32 rtl8306e_port_isolation_get(uint32 *pIsomsk)
{
    uint32 regval;

    if (NULL == pIsomsk)
        return FAILED;
    rtl8306e_reg_get(0, 29, 3, &regval);
    *pIsomsk = regval & 0x7FFF;

    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_vlan_tagAware_set
 * Description:
 *      Configure switch to be VLAN tag awared
 * Input:
 *      enabled  - Configure RTL8306 VLAN tag awared
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      If switch is unawared VLAN tag, packet with vlan tag is treated as untagged pkt
 *      and assigned PVID as VID.
 */
int32 rtl8306e_vlan_tagAware_set(uint32 enabled) 
{
    rtl8306e_regbit_set(0, 16, 10, 0, enabled ? 0: 1);        
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_vlan_tagAware_set
 * Description:
 *      Get switch to be VLAN tag awared  or not
 * Input:
 *      none
 * Output:
 *      pEnabled  - the pointer of RTL8306 VLAN tag awared status
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      If switch is unawared VLAN tag, packet with vlan tag is treated as untagged pkt
 *      and assigned PVID as VID.
 */
 
int32 rtl8306e_vlan_tagAware_get(uint32 * pEnabled) 
{
    uint32 bitValue;

    if (NULL == pEnabled)
        return FAILED;
    
    rtl8306e_regbit_get(0, 16, 10, 0, &bitValue);
    *pEnabled = (bitValue ? FALSE: TRUE);
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_vlan_IgrFilterEnable_set
 * Description:
 *      Configure VLAN ingress filter
 * Input:
 *      enabled  - enable or disable
 * Output:
 *      none 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *
 */
int32 rtl8306e_vlan_IgrFilterEnable_set(uint32 enabled) 
{

    rtl8306e_regbit_set(0, 16, 9, 0, enabled ? 0 : 1);    
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_vlan_IgrFilterEnable_get
 * Description:
 *      Get VLAN ingress filter enabled or disabled
 * Input:
 *      none
 * Output:
 *      pEnabled  - enable or disable
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *
 */
int32 rtl8306e_vlan_IgrFilterEnable_get(uint32 *pEnabled) 
{
    uint32 bitValue;

    if (NULL == pEnabled)
        return FAILED;
    
    rtl8306e_regbit_get(0, 16, 9, 0, &bitValue);
    *pEnabled = (bitValue ? 0: 1);
    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_vlan_leaky_set
 * Description:
 *      Configure switch to forward frames to other VLANs ignoring the egress rule.
 * Input:
 *      type   -  vlan leaky type
 *      enabled  - enable or disable
 * Output:
 *      none 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      type coulde be:
 *          RTL8306_VALN_LEAKY_UNICAST - Vlan leaky for unicast pkt
 *          RTL8306_VALN_LEAKY_MULTICAST - Vlan leaky for multicast pkt
 *          RTL8306_VALN_LEAKY_ARP - Vlan leaky for ARP brodcast pkt 
 *          RTL8306_VALN_LEAKY_MIRROR - Vlan leaky for mirror function

 *    1.When the Vlan leaky for unicast pkt is enabled, it enables the inter-VLANs unicast packet forwarding. 
 *    That is, if the L2 look up MAC table search hit, then the unicast packet will be forwarded
 *    to the egress port ignoring the egress rule.
 *    2.When Vlan leaky for multicast pkt is enabled, multicast packet may be flood to all multicast address
 *    group member set, ignoring the VLAN member set domain limitation.
 *    3.When Vlan leaky for ARP pkt is enabled, the ARP broadcast packets will be forward to all the other
 *    ports ignoring the egress rule.
 *    4.When Vlan leaky for mirror function is enabled, it enables the inter-VLANs mirror function, 
 *    ignoring the VLAN member set domain limitation.
 */
int32 rtl8306e_vlan_leaky_set(uint32 type, uint32 enabled)
{
    switch(type) 
    {
        case RTL8306_VALN_LEAKY_UNICAST:
            rtl8306e_regbit_set(0, 18, 11, 0, enabled ? 0 : 1);
            break;
        case RTL8306_VALN_LEAKY_MULTICAST:
            rtl8306e_regbit_set(2, 23, 7, 3, enabled ? 0 : 1);
            break;
        case RTL8306_VALN_LEAKY_ARP:
            rtl8306e_regbit_set(0, 18, 10, 0, enabled ? 0 : 1);
            break;
        case RTL8306_VALN_LEAKY_MIRROR:
            rtl8306e_regbit_set(2, 23, 6, 3, enabled ? 0 : 1);
            break;
        default:
            return FAILED;
    }    
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_vlan_leaky_get
 * Description:
 *      Get switch whether forwards unicast frames to other VLANs
 * Input:
 *      type   -  vlan leaky type
 * Output:
 *      pEnabled  - the pointer of Vlan Leaky status(Dsiabled or Enabled) 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *   type coulde be:
 *          RTL8306_VALN_LEAKY_UNICAST - Vlan leaky for unicast pkt
 *          RTL8306_VALN_LEAKY_MULTICAST - Vlan leaky for multicast pkt
 *          RTL8306_VALN_LEAKY_ARP - Vlan leaky for ARP brodcast pkt 
 *    1.When the Vlan leaky for unicast pkt is enabled, it enables the inter-VLANs unicast packet forwarding. 
 *    That is, if the L2 look up MAC table search hit, then the unicast packet will be forwarded
 *    to the egress port ignoring the egress rule.
 *    2.When Vlan leaky for multicast pkt is enabled, multicast packet may be flood to all multicast address
 *    group member set, ignoring the VLAN member set domain limitation.
 *    3.When Vlan leaky for ARP pkt is enabled, the ARP broadcast packets will be forward to all the other
 *    ports ignoring the egress rule.
 *    4.When Vlan leaky for mirror function is enabled, it enables the inter-VLANs mirror function, 
 *    ignoring the VLAN member set domain limitation.
 */

int32 rtl8306e_vlan_leaky_get(uint32 type, uint32 *pEnabled)
{
    uint32 bitValue;

    if(NULL == pEnabled)
        return FAILED;

    switch(type) 
    {
        case RTL8306_VALN_LEAKY_UNICAST:
            rtl8306e_regbit_get(0, 18, 11, 0, &bitValue);
            break;
        case RTL8306_VALN_LEAKY_MULTICAST:
            rtl8306e_regbit_get(2, 23, 7, 3, &bitValue);
            break;
        case RTL8306_VALN_LEAKY_ARP:
            rtl8306e_regbit_get(0, 18, 10, 0, &bitValue);
            break;
        case RTL8306_VALN_LEAKY_MIRROR:
            rtl8306e_regbit_get(2, 23, 6, 3, &bitValue);
            break;
        default:
            return FAILED;
    }
    *pEnabled = (bitValue ?  FALSE : TRUE);

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_vlan_nullVidReplace_set
 * Description:
 *      Configure switch to replace Null VID tagged frame by PVID if it is tag aware
 * Input:
 *      port   -  port number
 *      enabled  - enable or disable
 * Output:
 *      none 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      1.When Null VID replacement is enabled, 8306E only captures tagged packet with VID=0,
 *      then replace VID with input port's PVID. If switch received a packet that is not tagged, 
 *      it will not insert a tag with PVID to this packet.
 *      2. When Null VID replacement is disabled, switch will drop or deal the null VID tagged 
 *      frame depends on the configuration.
 */
int32 rtl8306e_vlan_nullVidReplace_set(uint32 port, uint32 enabled)
{
    uint32 speed, duplex, nway;

    if (port > RTL8306_PORT5)
        return FAILED;

    /*save mac 4 or port status when operate reg.22*/    
    if (4 == port) 
    {
        rtl8306e_regbit_get(5, 0, 13, 0, &speed);
        rtl8306e_regbit_get(5, 0, 12, 0, &nway);
        rtl8306e_regbit_get(5, 0, 8, 0, &duplex);            
    } 
    else if (5 == port) 
    {
        rtl8306e_regbit_get(6, 0, 13, 0, &speed);
        rtl8306e_regbit_get(6, 0, 12, 0, &nway);
        rtl8306e_regbit_get(6, 0, 8, 0, &duplex);            
    }

    /*Port 5 corresponding PHY6*/
    if (RTL8306_PORT5 == port)  
        port ++ ; 
    
    rtl8306e_regbit_set(port, 22, 12, 0, enabled ? 1 : 0);
    
    /*restore mac 4 or port status when operate reg.22*/    
    if (4 == port) 
    {
        rtl8306e_regbit_set(5, 0, 13, 0, speed);
        rtl8306e_regbit_set(5, 0, 12, 0, nway);
        rtl8306e_regbit_set(5, 0, 8, 0, duplex);            
    }
    else if (6 == port) 
    { 
        /*for port++ when port 5*/
        rtl8306e_regbit_set(6, 0, 13, 0, speed);
        rtl8306e_regbit_set(6, 0, 12, 0, nway);
        rtl8306e_regbit_set(6, 0, 8, 0, duplex);
    }           
    
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_vlan_nullVidReplace_get
 * Description:
 *      Configure switch to forward frames to other VLANs ignoring the egress rule.
 * Input:
 *      port   -  port number
 * Output:
 *      pEnabled  - the pointer of Null VID replacement ability(Dsiabled or Enabled) 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      1.When Null VID replacement is enabled, 8306E only captures tagged packet with VID=0,
 *      then replace VID with input port's PVID. If switch received a packet that is not tagged, 
 *      it will not insert a tag with PVID to this packet.
 *      2. When Null VID replacement is disabled, switch will drop or deal the null VID tagged 
 *      frame depends on the configuration.
 */
int32 rtl8306e_vlan_nullVidReplace_get(uint32 port, uint32 *pEnabled)
{
    uint32 bitValue;
    
    if ( (port > RTL8306_PORT5) || (NULL == pEnabled ))
        return FAILED;
    
    /*Port 5 corresponding PHY6*/    
    if (RTL8306_PORT5 == port)  
        port ++ ; 
    
    rtl8306e_regbit_get(port, 22, 12, 0, &bitValue);
    *pEnabled = (bitValue ? TRUE : FALSE);

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_vlan_portPvidIndex_set
 * Description:
 *      Configure switch port PVID index 
 * Input:
 *      port           -   Specify the port(port 0 ~ port 5) to configure VLAN index
 *      vlanIndex    -   Specify the VLAN index
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are 16 vlan entry, VID of vlan entry pointed by PVID index  is the PVID 
 */
int32 rtl8306e_vlan_portPvidIndex_set(uint32 port, uint32 vlanIndex)
{
    uint32 regValue;

    if(port > RTL8306_PORT5 || vlanIndex > (RTL8306_VLAN_ENTRYS -1))
        return FAILED;
    
    if (port < RTL8306_PORT5) 
    {
        rtl8306e_reg_get(port, 24, 0, &regValue);
        regValue = (regValue & 0xFFF) | (vlanIndex << 12);
        rtl8306e_reg_set(port, 24, 0, regValue);
    } else 
    {
        rtl8306e_reg_get(0, 26, 1, &regValue);
        regValue = (regValue & 0xFFF) | (vlanIndex << 12);
        rtl8306e_reg_set(0, 26, 1, regValue);
    }
       
    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_vlan_portPvidIndex_get
 * Description:
 *      Get switch port PVID index 
 * Input:
 *      port            -   Specify the port(port 0 ~ port 5) to configure VLAN index
 * Output:
 *      pVlanIndex   -   pointer of VLAN index number
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are 16 vlan entry, VID of vlan entry pointed by PVID index  is the PVID 
 */
 
int32 rtl8306e_vlan_portPvidIndex_get(uint32 port, uint32 *pVlanIndex) 
{
    if((port > RTL8306_PORT5) || pVlanIndex == NULL)
        return FAILED;
    
    if (port < RTL8306_PORT5) 
        rtl8306e_reg_get(port, 24, 0, pVlanIndex);
    else 
        rtl8306e_reg_get(0, 26, 1, pVlanIndex);
    *pVlanIndex = (*pVlanIndex >> 12) & 0xF;
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_vlan_portAcceptFrameType_set
 * Description:
 *      Set VLAN support frame type
 * Input:
 *      port                                 - Port id
 *      accept_frame_type             - accept frame type
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *    The API is used for checking 802.1Q tagged frames.
 *    The accept frame type as following:
 *    RTL8306E_ACCEPT_ALL
 *    RTL8306E_ACCEPT_TAG_ONLY
 *    RTL8306E_ACCEPT_UNTAG_ONLY
 */
int32 rtl8306e_vlan_portAcceptFrameType_set(uint32 port, rtl8306e_acceptFrameType_t accept_frame_type)
{
    uint32 regval;
    
    if(port > RTL8306_PORT5)
        return FAILED;
    
    if(RTL8306_PORT0 == port)
    {
        rtl8306e_reg_get(0, 31, 0, &regval);
        regval &= ~(0x3 << 7);
        regval |= (accept_frame_type << 7);
        rtl8306e_reg_set(0, 31, 0, regval);
    }
    else if (RTL8306_PORT5 == port)
    {
        rtl8306e_reg_get(6, 30, 1, &regval);
        regval &= ~(0x3 << 6);
        regval |= (accept_frame_type << 6);
        rtl8306e_reg_set(6, 30, 1, regval);        
    }
    else
    {
        rtl8306e_reg_get(port, 21, 2, &regval);
        regval &= ~(0x3 << 11);
        regval |= (accept_frame_type << 11);
        rtl8306e_reg_set(port, 21, 2, regval);        
    }

    return SUCCESS;
}



/* Function Name:
 *      rtl8306e_vlan_portAcceptFrameType_get
 * Description:
 *      Get VLAN support frame type
 * Input:
 *      port                                 - Port id
 * Output:
 *      pAccept_frame_type             - accept frame type pointer
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *    The API is used for checking 802.1Q tagged frames.
 *    The accept frame type as following:
 *    RTL8306E_ACCEPT_ALL
 *    RTL8306E_ACCEPT_TAG_ONLY
 *    RTL8306E_ACCEPT_UNTAG_ONLY
 */
int32 rtl8306e_vlan_portAcceptFrameType_get(uint32 port, rtl8306e_acceptFrameType_t *pAccept_frame_type)
{
    uint32 regval;
    
    if((port > RTL8306_PORT5) || (pAccept_frame_type == NULL))
        return FAILED;

    if(RTL8306_PORT0 == port)
    {
        rtl8306e_reg_get(0, 31, 0, &regval);
        regval &= (0x3 << 7);
        *pAccept_frame_type = regval >> 7;
    }
    else if (RTL8306_PORT5 == port)
    {
        rtl8306e_reg_get(6, 30, 1, &regval);
        regval &= (0x3 << 6);
        *pAccept_frame_type = regval >> 6;
    }
    else
    {
        rtl8306e_reg_get(port, 21, 2, &regval);
        regval &= (0x3 << 11);
        *pAccept_frame_type = regval >> 11;        
    }

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_vlan_tagInsert_set
 * Description:
 *      Insert VLAN tag by ingress port
 * Input:
 *      egPort               - egress port number 0~5
 *      igPortMsk           - ingress port mask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      egPort is packet egress port, if the packet is untagged and its igress port
 *      is in the igPortMsk, it will be inserted with an VLAN tag.
 */
int32 rtl8306e_vlan_tagInsert_set(uint32 egPort, uint32 igPortMsk)
{
    uint32 regval;
    
    if((egPort > RTL8306_PORT5) || (igPortMsk > 0x3F))
        return FAILED;
    
    /*enable EN_INSERT_TAG*/
    rtl8306e_regbit_set(0, 28, 0, 3, 1);

    if(RTL8306_PORT0 == egPort)
    {
        rtl8306e_reg_get(0, 28, 3, &regval);
        regval &= ~(0x3F << 1);
        regval |= (igPortMsk << 1);
        rtl8306e_reg_set(0, 28, 3, regval);        
    }
    else if(RTL8306_PORT5 == egPort)
    {
        rtl8306e_reg_get(6, 30, 1, &regval);
        regval &= ~0x3F;
        regval |= igPortMsk;
        rtl8306e_reg_set(6, 30, 1, regval);
    }
    else
    {
        rtl8306e_reg_get(egPort, 30, 1, &regval);
        regval &= ~(0x3F << 9);
        regval |= (igPortMsk << 9);
        rtl8306e_reg_set(egPort, 30, 1, regval);
    }
                
    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_vlan_tagInsert_get
 * Description:
 *      get  ingress port mask of VLAN tag insertion for untagged packet
 * Input:
 *      egPort               - egress port number 0~5
 *      igPortMsk           - ingress port mask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      egPort is packet egress port, if the packet is untagged and its igress port
 *      is in the igPortMsk, it will be inserted with an VLAN tag.
 */
int32 rtl8306e_vlan_tagInsert_get(uint32 egPort, uint32 * pIgPortMsk)
{
    uint32 regval ;
    
    if ((egPort > RTL8306_PORT5) || (NULL == pIgPortMsk))
        return FAILED;

    /*EN_INSERT_TAG = 0, insert tag operation is disable*/
    rtl8306e_regbit_get(0, 28, 0, 3, &regval);
    if(!regval)
        *pIgPortMsk = 0;
    
    if(RTL8306_PORT0 == egPort)
    {
        rtl8306e_reg_get(0, 28, 3, &regval);
        regval &= (0x3F << 1);
        *pIgPortMsk = regval >> 1;
    }
    else if(RTL8306_PORT5 == egPort)
    {
        rtl8306e_reg_get(6, 30, 1, &regval);
        regval &= 0x3F;
        *pIgPortMsk = regval;
    }
    else 
    {
        rtl8306e_reg_get(egPort, 30, 1, &regval);
        regval &= (0x3F << 9);
        *pIgPortMsk = regval >> 9;
    }
    
    return SUCCESS;
}

/* Function Name:
 *      rtk_vlan_set
 * Description:
 *      Set a VLAN entry
 * Input:
 *      vlanIndex  - VLAN entry index
 *      vid           - VLAN ID to configure
 *      mbrmsk     - VLAN member set portmask
 *      untagmsk  - VLAN untag set portmask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 16 VLAN entry supported. User could configure the member set and untag set
 *     for specified vid through this API. The portmask's bit N means port N.
 *     For example, mbrmask 23=0x17=010111 means port 0,1,2,4 in the member set.
 */
int32 rtl8306e_vlan_entry_set(uint32 vlanIndex, uint32 vid, uint32 mbrmsk, uint32 untagmsk )
{
    uint32 regValue;

    if ((vid > RTL8306_VIDMAX -1) || (mbrmsk > 0x3F) || (untagmsk > 0x3F))
        return FAILED;
    
    /*enable EN_UNTAG_SET*/
    rtl8306e_regbit_set(0, 29, 12, 0, 1);
    
    /*set vlan entry*/
    switch(vlanIndex) 
    {
        case 0: /* VLAN[A] */
            rtl8306e_reg_get(0, 25, 0, &regValue);
            regValue = (regValue & 0xF000) | vid ;
            rtl8306e_reg_set(0, 25, 0, regValue);
            rtl8306e_reg_get(0, 24, 0, &regValue);
            regValue = (regValue & 0xFFC0) | mbrmsk;
            rtl8306e_reg_set(0, 24, 0, regValue);
            rtl8306e_reg_get(0, 29, 0, &regValue);
            regValue &= ~0x3F;
            regValue |= untagmsk;
            rtl8306e_reg_set(0, 29, 0, regValue);            
            break;

        case 1: /* VLAN[B] */
            rtl8306e_reg_get(1, 25, 0, &regValue);
            regValue = (regValue & 0xF000) | vid ;
            rtl8306e_reg_set(1, 25, 0, regValue);
            rtl8306e_reg_get(1, 24, 0, &regValue);
            regValue = (regValue & 0xFFC0) | mbrmsk;
            rtl8306e_reg_set(1, 24, 0, regValue);
            rtl8306e_reg_get(1, 30, 1, &regValue);
            regValue &= ~0x3F;
            regValue |= untagmsk;
            rtl8306e_reg_set(1, 30, 1, regValue);            
            break;

        case 2: /* VLAN[C] */
            rtl8306e_reg_get(2, 25, 0, &regValue);
            regValue = (regValue & 0xF000) | vid ;
            rtl8306e_reg_set(2, 25, 0, regValue);
            rtl8306e_reg_get(2, 24, 0, &regValue);
            regValue = (regValue & 0xFFC0) | mbrmsk;
            rtl8306e_reg_set(2, 24, 0, regValue);
            rtl8306e_reg_get(2, 30, 1, &regValue);
            regValue &= ~0x3F;
            regValue |= untagmsk;
            rtl8306e_reg_set(2, 30, 1, regValue);                
            break;

        case 3: /* VLAN[D] */
            rtl8306e_reg_get(3, 25, 0, &regValue);
            regValue = (regValue & 0xF000) | vid;
            rtl8306e_reg_set(3, 25, 0, regValue);
            rtl8306e_reg_get(3, 24, 0, &regValue);
            regValue = (regValue & 0xFFC0) | mbrmsk;
            rtl8306e_reg_set(3, 24, 0, regValue);
            rtl8306e_reg_get(3, 30, 1, &regValue);
            regValue &= ~0x3F;
            regValue |= untagmsk;
            rtl8306e_reg_set(3, 30, 1, regValue);                
            break;

        case 4: /* VLAN[E] */
            rtl8306e_reg_get(4, 25, 0, &regValue);
            regValue = (regValue & 0xF000) | vid;
            rtl8306e_reg_set(4, 25, 0, regValue);
            rtl8306e_reg_get(4, 24, 0, &regValue);
            regValue = (regValue & 0xFFC0) | mbrmsk;
            rtl8306e_reg_set(4, 24, 0, regValue);
            rtl8306e_reg_get(4, 30, 1, &regValue);
            regValue &= ~0x3F;
            regValue |= untagmsk;
            rtl8306e_reg_set(4, 30, 1, regValue);                            
            break;

        case 5: /* VLAN[F] */
            rtl8306e_reg_get(0, 27, 1, &regValue);
            regValue = (regValue & 0xF000) | vid;
            rtl8306e_reg_set(0, 27, 1, regValue);
            rtl8306e_reg_get(0, 26, 1, &regValue);
            regValue = (regValue & 0xFFC0) | mbrmsk;
            rtl8306e_reg_set(0, 26, 1, regValue);
            rtl8306e_reg_get(0, 26, 1, &regValue);
            regValue &= ~(0x3F << 6);
            regValue |= (untagmsk << 6);
            rtl8306e_reg_set(0, 26, 1, regValue);            
            break;
        
        case 6: /* VLAN[G] */
            rtl8306e_reg_get(1, 27, 1, &regValue);
            regValue = (regValue & 0xF000) | vid;
            rtl8306e_reg_set(1, 27, 1, regValue);
            rtl8306e_reg_get(1, 26, 1, &regValue);
            regValue = (regValue & 0xFFC0) | mbrmsk;
            rtl8306e_reg_set(1, 26, 1, regValue);
            rtl8306e_reg_get(1, 26, 1, &regValue);
            regValue &= ~(0x3F << 6);
            regValue |= (untagmsk << 6);
            rtl8306e_reg_set(1, 26, 1, regValue);                        
            break;

        case 7: /* VLAN[H] */
            rtl8306e_reg_get(2, 27, 1, &regValue);
            regValue = (regValue & 0xF000) | vid;
            rtl8306e_reg_set(2, 27, 1, regValue);
            rtl8306e_reg_get(2, 26, 1, &regValue);
            regValue = (regValue & 0xFFC0) | mbrmsk;
            rtl8306e_reg_set(2, 26, 1, regValue);
            rtl8306e_reg_get(2, 26, 1, &regValue);
            regValue &= ~(0x3F << 6);
            regValue |= (untagmsk << 6);
            rtl8306e_reg_set(2, 26, 1, regValue);                                    
            break;

        case 8: /* VLAN[I] */
            rtl8306e_reg_get(3, 27, 1, &regValue);
            regValue = (regValue & 0xF000) | vid;
            rtl8306e_reg_set(3, 27, 1, regValue);
            rtl8306e_reg_get(3, 26, 1, &regValue);
            regValue = (regValue & 0xFFC0) | mbrmsk;
            rtl8306e_reg_set(3, 26, 1, regValue);
            rtl8306e_reg_get(3, 26, 1, &regValue);
            regValue &= ~(0x3F << 6);
            regValue |= (untagmsk << 6);
            rtl8306e_reg_set(3, 26, 1, regValue);                        
            
            break;

        case 9: /* VLAN[J] */
            rtl8306e_reg_get(4, 27, 1, &regValue);
            regValue = (regValue & 0xF000) | vid;
            rtl8306e_reg_set(4, 27, 1, regValue);
            rtl8306e_reg_get(4, 26, 1, &regValue);
            regValue = (regValue & 0xFFC0) | mbrmsk;
            rtl8306e_reg_set(4, 26, 1, regValue);
            rtl8306e_reg_get(4, 26, 1, &regValue);
            regValue &= ~(0x3F << 6);
            regValue |= (untagmsk << 6);
            rtl8306e_reg_set(4, 26, 1, regValue);                                    
            break;

        case 10: /* VLAN[K] */
            rtl8306e_reg_get(0, 29, 1, &regValue);
            regValue = (regValue & 0xF000) | vid;
            rtl8306e_reg_set(0, 29, 1, regValue);
            rtl8306e_reg_get(0, 28, 1, &regValue);
            regValue = (regValue & 0xFFC0) | mbrmsk;
            rtl8306e_reg_set(0, 28, 1, regValue);
            rtl8306e_reg_get(0, 28, 1, &regValue);
            regValue &= ~(0x3F << 6);
            regValue |= (untagmsk << 6);
            rtl8306e_reg_set(0, 28, 1, regValue);                                    
            break;

        case 11: /* VLAN[L] */
            rtl8306e_reg_get(1, 29, 1, &regValue);
            regValue = (regValue & 0xF000) | vid;
            rtl8306e_reg_set(1, 29, 1, regValue);
            rtl8306e_reg_get(1, 28, 1, &regValue);
            regValue = (regValue & 0xFFC0) | mbrmsk;
            rtl8306e_reg_set(1, 28, 1, regValue);
            rtl8306e_reg_get(1, 28, 1, &regValue);
            regValue &= ~(0x3F << 6);
            regValue |= (untagmsk << 6);
            rtl8306e_reg_set(1, 28, 1, regValue);                                                
            break;

        case 12: /* VLAN[M] */
            rtl8306e_reg_get(2, 29, 1, &regValue);
            regValue = (regValue & 0xF000) | vid;
            rtl8306e_reg_set(2, 29, 1, regValue);
            rtl8306e_reg_get(2, 28, 1, &regValue);
            regValue = (regValue & 0xFFC0) | mbrmsk;
            rtl8306e_reg_set(2, 28, 1, regValue);
            rtl8306e_reg_get(2, 28, 1, &regValue);
            regValue &= ~(0x3F << 6);
            regValue |= (untagmsk << 6);
            rtl8306e_reg_set(2, 28, 1, regValue);                                                
            break;

        case 13: /* VLAN[N] */
            rtl8306e_reg_get(3, 29, 1, &regValue);
            regValue = (regValue & 0xF000) | vid;
            rtl8306e_reg_set(3, 29, 1, regValue);
            rtl8306e_reg_get(3, 28, 1, &regValue);
            regValue = (regValue & 0xFFC0) |mbrmsk;
            rtl8306e_reg_set(3, 28, 1, regValue);
            rtl8306e_reg_get(3, 28, 1, &regValue);
            regValue &= ~(0x3F << 6);
            regValue |= (untagmsk << 6);
            rtl8306e_reg_set(3, 28, 1, regValue);                                                
            break;

        case 14: /* VLAN[O] */
            rtl8306e_reg_get(4, 29, 1, &regValue);
            regValue = (regValue & 0xF000) | vid;
            rtl8306e_reg_set(4, 29, 1, regValue);
            rtl8306e_reg_get(4, 28, 1, &regValue);
            regValue = (regValue & 0xFFC0) | mbrmsk;
            rtl8306e_reg_set(4, 28, 1, regValue);
            rtl8306e_reg_get(4, 28, 1, &regValue);
            regValue &= ~(0x3F << 6);
            regValue |= (untagmsk << 6);
            rtl8306e_reg_set(4, 28, 1, regValue);                                                
            break;

        case 15: /* VLAN[P] */
            rtl8306e_reg_get(0, 31, 1, &regValue);
            regValue = (regValue & 0xF000) | vid;
            rtl8306e_reg_set(0, 31, 1, regValue);
            rtl8306e_reg_get(0, 30, 1, &regValue);
            regValue = (regValue & 0xFFC0) | mbrmsk;
            rtl8306e_reg_set(0, 30, 1, regValue);
            rtl8306e_reg_get(0, 30, 1, &regValue);
            regValue &= ~(0x3F << 6);
            regValue |= (untagmsk << 6);
            rtl8306e_reg_set(0, 30, 1, regValue);                                                
            break;
        default:
            return FAILED;

    }    
    
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_vlan_entry_get
 * Description:
 *      Get a VLAN entry
 * Input:
 *      vlanIndex  - VLAN entry index
 * Output:
 *      pVid           -  the pointer of VLAN ID 
 *      pMbrmsk     -  the pointer of VLAN member set portmask
 *      pUntagmsk  -  the pointer of VLAN untag set portmask
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 16 VLAN entry supported. User could configure the member set and untag set
 *     for specified vid through this API. The portmask's bit N means port N.
 *     For example, mbrmask 23=0x17=010111 means port 0,1,2,4 in the member set.
 */
int32 rtl8306e_vlan_entry_get(uint32 vlanIndex, uint32 *pVid, uint32 *pMbrmsk, uint32 *pUntagmsk)
{

    if(  (NULL == pVid) ||  (NULL == pMbrmsk))
        return FAILED;
    
    switch(vlanIndex)
    {
        case 0: /*VLAN[A]*/
            rtl8306e_reg_get(0, 25, 0, pVid);
            rtl8306e_reg_get(0, 24, 0, pMbrmsk);
            rtl8306e_reg_get(0, 29, 0, pUntagmsk);            
            break;        
            
        case 1: /*VLAN[B]*/
            rtl8306e_reg_get(1, 25, 0, pVid);
            rtl8306e_reg_get(1, 24, 0, pMbrmsk);
            rtl8306e_reg_get(1, 30, 1, pUntagmsk);                        
            break;
            
        case 2: /*VLAN[C]*/
            rtl8306e_reg_get(2, 25, 0, pVid);
            rtl8306e_reg_get(2, 24, 0, pMbrmsk);
            rtl8306e_reg_get(2, 30, 1, pUntagmsk);                                    
            break;
            
        case 3: /*VLAN[D]*/
            rtl8306e_reg_get(3, 25, 0, pVid);
            rtl8306e_reg_get(3, 24, 0, pMbrmsk);
            rtl8306e_reg_get(3, 30, 1, pUntagmsk);                                    
            break;
            
        case 4: /*VLAN[E]*/
            rtl8306e_reg_get(4, 25, 0, pVid);
            rtl8306e_reg_get(4, 24, 0, pMbrmsk);
            rtl8306e_reg_get(4, 30, 1, pUntagmsk);                                    
            break;
            
        case 5: /*VLAN[F]*/
            rtl8306e_reg_get(0, 27, 1, pVid);
            rtl8306e_reg_get(0, 26, 1, pMbrmsk);
            rtl8306e_reg_get(0, 26, 1, pUntagmsk);   
            *pUntagmsk >>= 6;
            break;
            
        case 6: /*VLAN[G]*/
            rtl8306e_reg_get(1, 27, 1, pVid);
            rtl8306e_reg_get(1, 26, 1, pMbrmsk);
            rtl8306e_reg_get(1, 26, 1, pUntagmsk);   
            *pUntagmsk >>= 6;
            break;
            
        case 7: /*VLAN[H]*/
            rtl8306e_reg_get(2, 27, 1, pVid);
            rtl8306e_reg_get(2, 26, 1, pMbrmsk);
            rtl8306e_reg_get(2, 26, 1, pUntagmsk);   
            *pUntagmsk >>= 6;
            break;
            
        case 8: /*VLAN[I]*/        
            rtl8306e_reg_get(3, 27, 1, pVid);
            rtl8306e_reg_get(3, 26, 1, pMbrmsk);
            rtl8306e_reg_get(3, 26, 1, pUntagmsk);   
            *pUntagmsk >>= 6;
            break;
            
        case 9: /*VLAN[J]*/
            rtl8306e_reg_get(4, 27, 1, pVid);
            rtl8306e_reg_get(4, 26, 1, pMbrmsk);
            rtl8306e_reg_get(4, 26, 1, pUntagmsk);   
            *pUntagmsk >>= 6;
            break;
            
        case 10: /*VLAN[K]*/
            rtl8306e_reg_get(0, 29, 1, pVid);
            rtl8306e_reg_get(0, 28, 1, pMbrmsk);
            rtl8306e_reg_get(0, 28, 1, pUntagmsk);   
            *pUntagmsk >>= 6;
            break;
            
        case 11: /*VLAN[L]*/
            rtl8306e_reg_get(1, 29, 1, pVid);
            rtl8306e_reg_get(1, 28, 1, pMbrmsk);
            rtl8306e_reg_get(1, 28, 1, pUntagmsk);   
            *pUntagmsk >>= 6;
            break;
            
        case 12: /*VLAN[M]*/
            rtl8306e_reg_get(2, 29, 1, pVid);
            rtl8306e_reg_get(2, 28, 1, pMbrmsk);
            rtl8306e_reg_get(2, 28, 1, pUntagmsk);   
            *pUntagmsk >>= 6;
            break;
            
        case 13: /*VLAN[N]*/
            rtl8306e_reg_get(3, 29, 1, pVid);
            rtl8306e_reg_get(3, 28, 1, pMbrmsk);
            rtl8306e_reg_get(3, 28, 1, pUntagmsk);   
            *pUntagmsk >>= 6;
            break;
            
        case 14: /*VLAN[O]*/
            rtl8306e_reg_get(4, 29, 1, pVid);
            rtl8306e_reg_get(4, 28, 1, pMbrmsk);
            rtl8306e_reg_get(4, 28, 1, pUntagmsk);   
            *pUntagmsk >>= 6;
            break;
            
        case 15: /*VLAN[P]*/
            rtl8306e_reg_get(0, 31, 1, pVid);
            rtl8306e_reg_get(0, 30, 1, pMbrmsk);
            rtl8306e_reg_get(0, 30, 1, pUntagmsk);   
            *pUntagmsk >>= 6;
            break;
            
        default:
            return  FAILED;
    }
    
    *pVid  &= 0xFFF;
    *pMbrmsk  &= 0x3F;
    *pUntagmsk &= 0x3F;
    
    return  SUCCESS;
}


/* Function Name:
 *      rtl8306e_vlan_vlanBasedPriority_set
 * Description:
 *       Set VID based priority
 * Input:
 *      vlanIndex   -   Specify VLAN entry index
 *      pri            -   the specified VLAN priority  
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      each VLAN could be assigned an priority. if vlanIndex > 15, it means the VID
 *      is not in VLAN entries, unmatched VID could also be assigned an priority.
 *       
 */
int32 rtl8306e_vlan_vlanBasedPriority_set(uint32 vlanIndex, uint32 pri)
{
    uint32 regval;
    
    if (pri > 3)
        return FAILED;

    switch(vlanIndex)
    {
        case 0: /*VLAN[A]*/
            rtl8306e_regbit_set(0, 29, 8, 0, 1);
            rtl8306e_reg_get(0, 29, 0, &regval);
            regval &= ~(0x3 << 6);
            regval |= (pri << 6);
            rtl8306e_reg_set(0, 29, 0, regval);            
            break;        
            
        case 1: /*VLAN[B]*/
            rtl8306e_regbit_set(1, 30, 8, 1, 1);
            rtl8306e_reg_get(1, 30, 1, &regval);
            regval &= ~(0x3 << 6);
            regval |= (pri << 6);
            rtl8306e_reg_set(1, 30, 1, regval);                                   
            break;
            
        case 2: /*VLAN[C]*/
            rtl8306e_regbit_set(2, 30, 8, 1, 1);
            rtl8306e_reg_get(2, 30, 1, &regval);
            regval &= ~(0x3 << 6);
            regval |= (pri << 6);
            rtl8306e_reg_set(2, 30, 1, regval);                                   
            break;
            
        case 3: /*VLAN[D]*/
            rtl8306e_regbit_set(3, 30, 8, 1, 1);
            rtl8306e_reg_get(3, 30, 1, &regval);
            regval &= ~(0x3 << 6);
            regval |= (pri << 6);
            rtl8306e_reg_set(3, 30, 1, regval);                                               
            break;
            
        case 4: /*VLAN[E]*/
            rtl8306e_regbit_set(4, 30, 8, 1, 1);
            rtl8306e_reg_get(4, 30, 1, &regval);
            regval &= ~(0x3 << 6);
            regval |= (pri << 6);
            rtl8306e_reg_set(4, 30, 1, regval);                                               
            break;
            
        case 5: /*VLAN[F]*/
            rtl8306e_regbit_set(0, 27, 14, 1, 0);
            rtl8306e_reg_get(0, 27, 1, &regval);
            regval &= ~(0x3 << 12);
            regval |= (pri << 12);
            rtl8306e_reg_set(0, 27, 1, regval);                                               
            break;
            
        case 6: /*VLAN[G]*/
            rtl8306e_regbit_set(1, 26, 14, 1, 0);
            rtl8306e_reg_get(1, 26, 1, &regval);
            regval &= ~(0x3 << 12);
            regval |= (pri << 12);
            rtl8306e_reg_set(1, 26, 1, regval);                                                           
            break;
            
        case 7: /*VLAN[H]*/
            rtl8306e_regbit_set(2, 26, 14, 1, 0);
            rtl8306e_reg_get(2, 26, 1, &regval);
            regval &= ~(0x3 << 12);
            regval |= (pri << 12);
            rtl8306e_reg_set(2, 26, 1, regval);                                                                       
            break;
            
        case 8: /*VLAN[I]*/        
            rtl8306e_regbit_set(3, 26, 14, 1, 1);
            rtl8306e_reg_get(3, 26, 1, &regval);
            regval &= ~(0x3 << 12);
            regval |= (pri << 12);
            rtl8306e_reg_set(3, 26, 1, regval);                                                                                   
            break;
            
        case 9: /*VLAN[J]*/
            rtl8306e_regbit_set(4, 26, 14, 1, 1);
            rtl8306e_reg_get(4, 26, 1, &regval);
            regval &= ~(0x3 << 12);
            regval |= (pri << 12);
            rtl8306e_reg_set(4, 26, 1, regval);                                                                                               
            break;
            
        case 10: /*VLAN[K]*/
            rtl8306e_regbit_set(0, 28, 14, 1, 1);
            rtl8306e_reg_get(0, 28, 1, &regval);
            regval &= ~(0x3 << 12);
            regval |= (pri << 12);
            rtl8306e_reg_set(0, 28, 1, regval);                                                                                                           
            break;
            
        case 11: /*VLAN[L]*/
            rtl8306e_regbit_set(1, 28, 14, 1, 1);
            rtl8306e_reg_get(1, 28, 1, &regval);
            regval &= ~(0x3 << 12);
            regval |= (pri << 12);
            rtl8306e_reg_set(1, 28, 1, regval);                                                                                                                       
            break;
            
        case 12: /*VLAN[M]*/
            rtl8306e_regbit_set(2, 28, 14, 1, 0);
            rtl8306e_reg_get(2, 28, 1, &regval);
            regval &= ~(0x3 << 12);
            regval |= (pri << 12);
            rtl8306e_reg_set(2, 28, 1, regval);                                                                                                                                   
            break;
            
        case 13: /*VLAN[N]*/
            rtl8306e_regbit_set(3, 28, 14, 1, 0);
            rtl8306e_reg_get(3, 28, 1, &regval);
            regval &= ~(0x3 << 12);
            regval |= (pri << 12);
            rtl8306e_reg_set(3, 28, 1, regval);                                                                                                                                               
            break;
            
        case 14: /*VLAN[O]*/
            rtl8306e_regbit_set(4, 28, 14, 1, 0);
            rtl8306e_reg_get(4, 28, 1, &regval);
            regval &= ~(0x3 << 12);
            regval |= (pri << 12);
            rtl8306e_reg_set(4, 28, 1, regval);                                                                                                                                                           
            break;
            
        case 15: /*VLAN[P]*/
            rtl8306e_regbit_set(0, 30, 14, 1, 0);
            rtl8306e_reg_get(0, 30, 1, &regval);
            regval &= ~(0x3 << 12);
            regval |= (pri << 12);
            rtl8306e_reg_set(0, 30, 1, regval);                                                                                                                                                                       
            break;
            
        default: /*unmatched vlan entry*/
            rtl8306e_regbit_set(0, 29, 11, 0, 1);
            rtl8306e_reg_get(0, 29, 0, &regval);
            regval &= ~(0x3 << 9);
            regval |= (pri << 9);
            rtl8306e_reg_set(0, 29, 0, regval);                                                                                                                                                                                                          
            return  FAILED;
    }
    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_vlan_vlanBasedPriority_get
 * Description:
 *       Get VID based priority
 * Input:
 *      vlanIndex   -   Specify VLAN entry index
 * Output:
 *      pPri           -   the pointer of specified VLAN priority  
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      each VLAN could be assigned an priority. if vlanIndex > 15, it means the VID
 *      is not in VLAN entries, unmatched VID could also be assigned an priority.
 *       
 */
int32 rtl8306e_vlan_vlanBasedPriority_get(uint32 vlanIndex, uint32 *pPri)
{
    uint32 regval;
    
    if (NULL == pPri)
        return FAILED;

    switch(vlanIndex)
    {
        case 0: /*VLAN[A]*/
            rtl8306e_reg_get(0, 29, 0, &regval);
            regval &= (0x3 << 6);
            *pPri = regval >> 6;
            break;        
            
        case 1: /*VLAN[B]*/
            rtl8306e_reg_get(1, 30, 1, &regval);
            regval &= (0x3 << 6);
            *pPri = regval >> 6;
            break;
            
        case 2: /*VLAN[C]*/
            rtl8306e_reg_get(2, 30, 1, &regval);
            regval &= (0x3 << 6);
            *pPri = regval >> 6;
            break;
            
        case 3: /*VLAN[D]*/
            rtl8306e_reg_get(3, 30, 1, &regval);
            regval &= (0x3 << 6);
            *pPri = regval >> 6;
            break;
            
        case 4: /*VLAN[E]*/
            rtl8306e_reg_get(4, 30, 1, &regval);
            regval &= (0x3 << 6);
            *pPri = regval >> 6;
            break;
            
        case 5: /*VLAN[F]*/
            rtl8306e_reg_get(0, 27, 1, &regval);
            regval &= (0x3 << 12);
            *pPri = regval >> 12;
            break;
            
        case 6: /*VLAN[G]*/
            rtl8306e_reg_get(1, 26, 1, &regval);
            regval &= (0x3 << 12);
            *pPri = regval >> 12;
            break;
            
        case 7: /*VLAN[H]*/
            rtl8306e_reg_get(2, 26, 1, &regval);
            regval &= (0x3 << 12);
            *pPri = regval >> 12;
            break;
            
        case 8: /*VLAN[I]*/        
            rtl8306e_reg_get(3, 26, 1, &regval);
            regval &= (0x3 << 12);
            *pPri = regval >> 12;
            break;
            
        case 9: /*VLAN[J]*/
            rtl8306e_reg_get(4, 26, 1, &regval);
            regval &= (0x3 << 12);
            *pPri = regval >> 12;
            break;
            
        case 10: /*VLAN[K]*/
            rtl8306e_reg_get(0, 28, 1, &regval);
            regval &= (0x3 << 12);
            *pPri = regval >> 12;
            break;
            
        case 11: /*VLAN[L]*/
            rtl8306e_reg_get(1, 28, 1, &regval);
            regval &= (0x3 << 12);
            *pPri = regval >> 12;
            break;
            
        case 12: /*VLAN[M]*/
            rtl8306e_reg_get(2, 28, 1, &regval);
            regval &= (0x3 << 12);
            *pPri = regval >> 12;
            break;
            
        case 13: /*VLAN[N]*/
            rtl8306e_reg_get(3, 28, 1, &regval);
            regval &= (0x3 << 12);
            *pPri = regval >> 12;
            break;
            
        case 14: /*VLAN[O]*/
            rtl8306e_reg_get(4, 28, 1, &regval);
            regval &= (0x3 << 12);
            *pPri = regval >> 12;
            break;
            
        case 15: /*VLAN[P]*/
            rtl8306e_reg_get(0, 30, 1, &regval);
            regval &= (0x3 << 12);
            *pPri = regval >> 12;
            break;
            
        default: /*unmatched vlan entry*/
            rtl8306e_reg_get(0, 29, 0, &regval);
            regval &= (0x3 << 9);
            *pPri = regval >> 9;
            return  FAILED;
    }
    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_vlan_transEnable_set
 * Description:
 *      Enable VLAN translation 
 * Input:
 *      enable       -   enable or disable VLAN translation
 *      portmask    -   NNI port is set 1 and UNI port is set 0
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Only the traffic between NNI port and UNI port will be change with VID,
 *      the VLAN tranlation function and Q-in-Q(SVLAN) could not be enabled  
 *      at the same time, user should choose one of them. 
 */
int32 rtl8306e_vlan_transEnable_set(uint32 enable, uint32 portmask)
{
    uint32 regval;

    rtl8306e_reg_get(6, 30, 3, &regval);

    if(enable)
    {
        regval &= ~0xff;
        regval |= (1 << 6) | (portmask & 0x3F);
        rtl8306e_reg_set(6, 30, 3, regval);
    }
    else
    {
        regval &= ~0xff;
        rtl8306e_reg_set(6, 30, 3, regval);
    }

    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_vlan_transEnable_get
 * Description:
 *      Get VLAN translation setting
 * Input:
 *      none
 * Output:
 *      pEnable       -   the pointer of enable or disable VLAN translation
 *      pPortMask    -   the pointer of VLAN translation port mask
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Only the traffic between NNI port and UNI port will be change with VID,
 *      the VLAN tranlation function and Q-in-Q(SVLAN) could not be enabled  
 *      at the same time, user should choose one of them. 
 */
int32 rtl8306e_vlan_transEnable_get(uint32 *pEnable, uint32 *pPortMask)
{
    uint32 regval;

    if((NULL == pEnable) || (NULL == pPortMask))
        return FAILED;
    
    rtl8306e_reg_get(6, 30, 3, &regval);
    if((regval & (0x3 << 6)) == (0x1 << 6))
    {
        *pEnable = 1;
        *pPortMask = regval & 0x3F;
    }
    else
    {
        *pEnable = 0;
        *pPortMask = 0;
    }

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_vlan_transparentEnable_set
 * Description:
 *      Enable VLAN transparent 
 * Input:
 *      enable       -   enable or disable VLAN transparent
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */
int32 rtl8306e_vlan_transparentEnable_set(uint32 enable)
{
    uint32 port;

    if (enable)
    {
        /*set vlan disabled*/
        rtl8306e_regbit_set(0, 18, 8, 0, 1);
        
        /*disable tag aware*/
        rtl8306e_vlan_tagAware_set(FALSE); 

        /*disable EN_UNTAG_SET*/
        rtl8306e_regbit_set(0, 29, 12, 0, 0);
        
        /*set vlan translation disabled*/
        rtl8306e_regbit_set(6, 30, 6, 3, 0);
        
        /*set port0~5 untag unawre and disable Non-PVID discard*/
        for (port = 0; port < 5; port ++)
        {
            rtl8306e_regbit_set(port, 22, 15, 0, 1);
            rtl8306e_regbit_set(port, 22, 11, 0, 0);
        } 
         rtl8306e_regbit_set(6, 24, 1, 0, 1);
         rtl8306e_regbit_set(6, 22, 11, 0, 0);
         
        /*disable EN_INSERT_TAG*/
        rtl8306e_regbit_set(0, 28, 0, 3, 0);
    }
    else
    {
        /*set vlan enabled*/
        rtl8306e_regbit_set(0, 18, 8, 0, 0);
        
        /*enable tag aware*/
        rtl8306e_vlan_tagAware_set(TRUE); 
    }

    
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_vlan_transVid_set
 * Description:
 *      Set the translated VID
 * Input:
 *      vlanIndex   -   the VLAN entry index
 *      transVid     -   the new VID
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      The VID in the entry indexed by vlanIndex will be replaced with
 *      new VID.
 */ 
int32 rtl8306e_vlan_transVid_set(uint32 vlanIndex, uint32 transVid)    
{
   uint32 regval;
   
   if((vlanIndex > 15) || (transVid > 0xFFF))
        return FAILED;

   if(vlanIndex < 15)     
   {
        rtl8306e_reg_get(6, 17 + vlanIndex, 4, &regval);
        regval &= ~0xfff;
        regval |= transVid;
        rtl8306e_reg_set(6, 17 + vlanIndex, 4, regval);
   }
   else
   {
        rtl8306e_reg_get(6, 18, 1, &regval);
        regval &= ~0xfff;
        regval |= transVid;
        rtl8306e_reg_set(6, 18, 1, regval);
   }
       
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_vlan_transVid_get
 * Description:
 *      Get the translated VID
 * Input:
 *      vlanIndex   -   the VLAN entry index
 * Output:
 *      pTransVid   -   the pointer of the new VID
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      The VID in the entry indexed by vlanIndex will be replaced with
 *      new VID.
 */ 
int32 rtl8306e_vlan_transVid_get(uint32 vlanIndex, uint32 *pTransVid)
{
    uint32 regval;

   if((vlanIndex > 15) || (NULL == pTransVid))
        return FAILED;

   if(vlanIndex < 15)
   {
       rtl8306e_reg_get(6, 17 + vlanIndex, 4, &regval);
       *pTransVid = (regval & 0xfff);
   }
   else
   {
       rtl8306e_reg_get(6, 18, 1, &regval);
       *pTransVid = (regval & 0xfff);
   }

    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_svlan_tagAdmit_set
 * Description:
 *      Set Q-in-Q tag admit control
 * Input:
 *      port          -   port id
 *      enable       -   enable tag admit control 
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *       
 */
int32 rtl8306e_svlan_tagAdmit_set(uint32 port, uint32 enable)
{
    if (port > 5)
        return FAILED;

    if(5 == port)
    {
        rtl8306e_regbit_set(6, 23, 0, 0, enable ? 1:0);
    }
    else
    {
        rtl8306e_regbit_set(6, 17 + port, 0, 0, enable ? 1:0);
    }

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_svlan_tagAdmit_get
 * Description:
 *      Get Q-in-Q tag admit control
 * Input:
 *      port          -   port id
 * Output:
 *      pEnable     -   the pointer of enable tag admit control 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *       
 */
int32 rtl8306e_svlan_tagAdmit_get(uint32 port, uint32 *pEnable)
{
    uint32 regval;
    
    if(port > 5)
        return FAILED;
    
    if(5 == port)
    {
        rtl8306e_reg_get(6, 23, 0, &regval);
        *pEnable = regval ? 1:0;
    }
    else 
    {
        rtl8306e_reg_get(6, 17 + port, 0, &regval);
        *pEnable = regval ? 1:0;        
    }
        

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_svlan_otagSrc_set
 * Description:
 *      Set how to decide outer tag vid and priority 
 * Input:
 *      port          -   port id
 *      ovidSrc      -   ovid comes from
 *      opriSrc      -    opri comes from 
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *       ovidSrc RTL8306E_VIDSRC_POVID : ovid is port-based ovid,    RTL8306E_VIDSRC_NVID:ovid is new vid(translated vid)
 *       opriSrc  RTL8306E_PRISRC_PPRI   : opri is port-based priority, RTL8306E_PRISRC_1PRMK: opri is 1p remarking value
 */
int32 rtl8306e_svlan_otagSrc_set(uint32 port, uint32 ovidSrc, uint32 opriSrc)
{
    uint32 regval;
    
    if(port > 5)
        return FAILED;

    if(5 == port)
    {
        rtl8306e_reg_get(6, 23, 0, &regval);
        regval &= ~0xC;
        regval |= ovidSrc ? (1 << 3):0;
        regval |= opriSrc ? (1 << 2) :0;
        rtl8306e_reg_set(6, 23, 0, regval);
    }
    else
    {
        rtl8306e_reg_get(6, 17 + port, 0, &regval);
        regval &= ~0xC;
        regval |= ovidSrc ? (1 << 3):0;
        regval |= opriSrc ? (1 << 2) :0;
        rtl8306e_reg_set(6, 17 + port, 0, regval);        
    }

    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_svlan_otagSrc_get
 * Description:
 *      Get how to decide outer tag vid and priority 
 * Input:
 *      port            -   port id
 * Output:
 *      pOvidsrc      -   the pointer of ovid comes from
 *      pOpriSrc      -   the pointer of opri comes from 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *       ovidSrc RTL8306E_VIDSRC_POVID : ovid is port-based ovid,    RTL8306E_VIDSRC_NVID:ovid is new vid(translated vid)
 *       opriSrc  RTL8306E_PRISRC_PPRI   : opri is port-based priority, RTL8306E_PRISRC_1PRMK: opri is 1p remarking value
 */

int32 rtl8306e_svlan_otagSrc_get(uint32 port, uint32 *pOvidsrc, uint32 *pOpriSrc)
{
    uint32 regval;

    if(port > 5)
        return FAILED;

    if(5 == port)
    {
        rtl8306e_reg_get(6, 23, 0, &regval);
        *pOvidsrc = (regval & (1 << 3)) ? 1:0;
        *pOpriSrc = (regval & (1 << 2)) ? 1:0;
    }
    else
    {
        rtl8306e_reg_get(6, 17 + port, 0, &regval);
        *pOvidsrc = (regval & (1 << 3)) ? 1:0;
        *pOpriSrc = (regval & (1 << 2)) ? 1:0;
    }

    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_cpu_set
 * Description:
 *       Specify Asic CPU port 
 * Input:
 *      port       -   Specify the port
 *      enTag    -    CPU tag insert or not
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      If the port is specified RTL8306_NOCPUPORT, it means
 *      that no port is assigned as cpu port
 */
int32 rtl8306e_cpu_set(uint32 port, uint32 enTag) 
{
    uint32 regValue;

    if (port > RTL8306_NOCPUPORT)
        return FAILED;

    if (port < RTL8306_PORT_NUMBER) 
    {
        /*enable CPU port Function */
        rtl8306e_regbit_set(2, 21, 15, 3, 0);        
        
        /*whether inserting CPU tag*/
        rtl8306e_regbit_set(2, 21, 12, 3, enTag ? 1 : 0);
        
        /*enable the ability to check cpu tag*/
        rtl8306e_regbit_set(4, 21, 7, 0, 1);
        
        /*enable removing CPU tag*/
        rtl8306e_regbit_set(2, 21, 11, 3, 1);
        rtl8306e_reg_get(4, 21, 0, &regValue);
        regValue = (regValue & 0xFFF8) | port;
        rtl8306e_reg_set(4, 21, 0, regValue);

        /*enable asic recaculate crc for pkt with cpu tag*/
        rtl8306e_regbit_set(4, 21, 3, 0, 0);
        
        /*disable IEEE802.1x function of CPU Port*/
        if (port < RTL8306_PORT5) 
        {
            rtl8306e_regbit_set(port, 17, 9, 2, 0);
            rtl8306e_regbit_set(port, 17, 8, 2, 0);
        }
        else 
        {
            rtl8306e_regbit_set(6, 17, 9, 2, 0);
            rtl8306e_regbit_set(6, 17, 8, 2, 0);
        }
        
        /*Port 5 should be enabled especially*/
        if (RTL8306_PORT5 == port)
        {
            rtl8306e_regbit_set(6, 22, 15, 0, TRUE);
        }
    }   
    else 
    {
        /*disable CPU port Function */
        rtl8306e_regbit_set(2, 21, 15, 3, 1);
        rtl8306e_regbit_set(2, 21, 12, 3, 0);        
        rtl8306e_reg_get(4, 21, 0, &regValue);
        regValue = (regValue & 0xFFF8) | port;
        rtl8306e_reg_set(4, 21, 0, regValue);
    }   


    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_cpu_set
 * Description:
 *       Get Asic CPU port number
 * Input:
 *      none
 * Output:
 *      pPort     - the pointer of CPU port number
 *      pEnTag  - the pointer of CPU tag insert or not
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      If the port is specified RTL8306_NOCPUPORT, it means
 *      that no port is assigned as cpu port
 */
int32 rtl8306e_cpu_get(uint32 *pPort, uint32 *pEnTag) 
{
    uint32 regValue;
    uint32 bitValue, bitVal, cpufun;

    if ((NULL == pPort ) || (NULL == pEnTag))
        return FAILED;
    
    rtl8306e_regbit_get(2, 21, 12, 3, &bitValue);
    rtl8306e_regbit_get(2, 21, 15, 3, &cpufun);
    rtl8306e_regbit_get(4, 21, 7, 0, &bitVal);
    
    rtl8306e_reg_get(4, 21, 0, &regValue);
    *pPort = regValue & 0x7;

    if ((!cpufun) && bitValue && bitVal)
        *pEnTag = TRUE;
    else 
        *pEnTag = FALSE;
    
    if(cpufun)
        *pPort = RTL8306_NOCPUPORT;
    
    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_qos_softReset_set
 * Description:
 *      Software reset the asic
 * Input:
 *      none
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      reset packet buffer.
 */
 
int32 rtl8306e_qos_softReset_set(void) 
{    
    /*software reset*/
    rtl8306e_regbit_set(0, 16, 12, 0, 1);    
    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_qos_queueNum_set
 * Description:
 *      Set egress port queue number (1 ~4)
 * Input:
 *      none
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Queue number is global configuration for switch.
 */

int32 rtl8306e_qos_queueNum_set(uint32 num) 
{
    uint32 regValue;
    
    if ((num == 0) ||(num > 4))
        return FAILED;
    
    rtl8306e_reg_get(2, 22, 3, &regValue);
    rtl8306e_reg_set(2, 22, 3, (regValue & 0xFFF3) | ((num-1) << 2));
    
    /*A soft-reset is required after configuring queue num*/
     rtl8306e_qos_softReset_set( );
    
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_qos_queueNum_set
 * Description:
 *      Set egress port queue number (1 ~4)
 * Input:
 *      none
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Queue number is global configuration for switch.
 */
int32 rtl8306e_qos_queueNum_get(uint32 *pNum) 
{
    uint32 regValue;

    if (NULL == pNum) 
        return FAILED;
    
    rtl8306e_reg_get(2, 22, 3, &regValue);
    *pNum =    ((regValue & 0xC) >> 2) + 1;
    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_qos_priToQueMap_set
 * Description:
 *     Set priority to Queue ID mapping
 * Input:
 *      priority   -  priority value (0 ~ 3)
 *      qid        -  Queue id (0~3)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Packets could be classified into specified queue through their priority. 
 *      we can use this function to set pkt priority with queue id mapping
 */
int32 rtl8306e_qos_priToQueMap_set(uint32 priority, uint32 qid) 
{
    uint32 regValue;

    if ((qid > 3) || (priority > 3)) 
        return FAILED;
    
    rtl8306e_reg_get(1, 22, 3, &regValue);
    switch(priority) 
    {
        case 0:
            regValue = (regValue & 0xFFFC) | qid;
            break;
        case 1:
            regValue = (regValue & 0xFFF3) | (qid << 2);
            break;
        case 2:
            regValue = (regValue & 0xFFCF) | (qid << 4);
            break;
        case 3:
            regValue = (regValue & 0xFF3F) | (qid << 6);
            break;
        default:
            return FAILED;
    }
    rtl8306e_reg_set(1, 22, 3, regValue);
    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_qos_priToQueMap_get
 * Description:
 *      Get priority to Queue ID mapping
 * Input:
 *      priority   -  priority value (0 ~ 3)
 * Output:
 *      pQid      -  pointer of Queue id (0~3)
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Packets could be classified into specified queue through their priority. 
 *      we can use this function to set pkt priority with queue id mapping
 */
int32 rtl8306e_qos_priToQueMap_get(uint32 priority, uint32 *pQid) 
{
    uint32 regValue;

    if ((priority > 3) || (NULL == pQid))
        return FAILED;
    
    rtl8306e_reg_get(1, 22, 3, &regValue);
    
    switch(priority) 
    {
        case 0:
            *pQid = regValue & 0x3;
            break;
        case 1:
            *pQid = (regValue & 0xC) >> 2;
            break;
        case 2:
            *pQid = (regValue & 0x30) >> 4;
            break;
        case 3:
            *pQid = (regValue & 0xC0) >> 6;
            break;
        default:
            return FAILED;
    }
    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_qos_portRate_set
 * Description:
 *      Set port bandwidth control
 * Input:
 *      port            -  port number (0~5)
 *      n64Kbps       -  Port rate (0~1526), unit 64Kbps
 *      direction      -  Ingress or Egress bandwidth control
 *      enabled       -  enable bandwidth control
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      For each port, both input and output bandwidth could be configured, 
 *      RTL8306_PORT_RX represents port input bandwidth control, 
 *      RTL8306_PORT_TX represents port output bandwidth control.
 *      port rate unit is 64Kbps. For output rate control, enable/disable 
 *      is configured per port, but for input rate control, it is for all port.
 */
int32 rtl8306e_qos_portRate_set(uint32 port, uint32 n64Kbps, uint32 direction, uint32 enabled)
{
    uint32 regValue;
    
    if ((port > RTL8306_PORT5) || (n64Kbps > 0x5F6) || (direction > 1))
        return FAILED;
    
    /*Port 5 corresponding PHY6*/
    if (RTL8306_PORT5 == port)  
        port ++ ;  
    
    if (RTL8306_PORT_RX == direction) 
    {  
        /*configure port Rx rate*/
        if (!enabled ) 
        {
            rtl8306e_regbit_set(0, 21, 15, 3, 1);
        } 
        else 
        {
            rtl8306e_regbit_set(0, 21, 15, 3, 0);
            rtl8306e_reg_get(port, 21, 2, &regValue);
            regValue = (regValue & 0xF800) | (n64Kbps & 0x7FF);
            rtl8306e_reg_set(port, 21, 2, regValue);
        }
    } 
    else 
    {  
        /*configure port Tx rate*/
        if (!enabled) 
        {
            rtl8306e_regbit_set(port, 18, 15, 2, 0);
        } 
        else 
        {
            rtl8306e_regbit_set(port, 18, 15, 2, 1);
            rtl8306e_reg_get(port, 18, 2, &regValue);
            regValue = (regValue & 0xF800) | (n64Kbps & 0x7FF);
            rtl8306e_reg_set(port, 18, 2, regValue);
        }
    }
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_qos_portRate_get
 * Description:
 *      Get port bandwidth control rate
 * Input:
 *      port                 -  Port number (0~5)
 * Output:
 *      *pN64Kbps        -  Port rate (0~1526), unit 64Kbps
 *      direction           -  Input or output bandwidth control
 *      *enabled           -  enabled or disabled bandwidth control
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      For each port, both input and output bandwidth could be configured, 
 *      RTL8306_PORT_RX represents port input bandwidth control, 
 *      RTL8306_PORT_TX represents port output bandwidth control.
 *      port rate unit is 64Kbps. For output rate control, enable/disable 
 *      is configured per port, but for input rate control, it is for all port.
 */
int32 rtl8306e_qos_portRate_get(uint32 port, uint32 *pN64Kbps, uint32 direction, uint32 *pEnabled) 
{
    uint32 regValue;

    if ((port > RTL8306_PORT5) || (NULL == pN64Kbps) || (direction > RTL8306_PORT_TX) || (NULL == pEnabled))
        return FAILED;
    
    /*Port 5 corresponding PHY6*/
    if (RTL8306_PORT5 == port)  
        port ++ ;  

    if (RTL8306_PORT_RX == direction)
    {
        /*Get port Rx rate*/
        rtl8306e_regbit_get(0, 21, 15, 3, &regValue);
        *pEnabled = (regValue ? FALSE:TRUE);
        rtl8306e_reg_get(port, 21, 2, &regValue);
        *pN64Kbps = regValue & 0x7FF;
    } 
    else 
    { 
        /*Get port Tx rate*/
        rtl8306e_regbit_get(port, 18, 15, 2, pEnabled);
        rtl8306e_reg_get(port, 18, 2, &regValue);
        *pN64Kbps = regValue & 0x7FF;
    }
    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_qos_1pRemarkEnable_set
 * Description:
 *      Set 802.1P remarking ability
 * Input:
 *      port       -  port number (0~5)
 *      enabled  -  TRUE or FALSE
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      
 */
int32 rtl8306e_qos_1pRemarkEnable_set(uint32 port, uint32 enabled)
{

    if (port > RTL8306_PORT5)
        return FAILED;
    
    /*Port 5 corresponding PHY6*/
    if (RTL8306_PORT5 == port)  
        port ++ ;
    /*enable new 1p remarking function*/
    rtl8306e_regbit_set(0, 28, 13, 3, 1);
    rtl8306e_regbit_set(port, 17, 0, 2, enabled ? 1:0);
    
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_qos_1pRemarkEnable_set
 * Description:
 *      Get 802.1P remarking ability
 * Input:
 *      port        -  port number (0~5)
 * Output:
 *      pEnabled  -  pointer of the ability status
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      
 */
int32 rtl8306e_qos_1pRemarkEnable_get(uint32 port, uint32 *pEnabled) 
{
    uint32 bitValue;
    
    if (port > RTL8306_PORT5 || (NULL == pEnabled))
        return FAILED;
    
    /*Port 5 corresponding PHY6*/
    if (RTL8306_PORT5 == port)  
        port ++ ;
    
    rtl8306e_regbit_get(port, 17, 0, 2, &bitValue);
    *pEnabled = (bitValue ? TRUE:FALSE);
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_qos_1pRemark_set
 * Description:
 *      Set 802.1P remarking priority
 * Input:
 *      priority       -  Packet priority(0~4)
 *      priority1p    -  802.1P priority(0~7)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch determines packet priority, the priority souce could
 *      be port-based, 1Q-based, dscp-based, vid-based, ip address,
 *      cpu tag.
 */
int32 rtl8306e_qos_1pRemark_set(uint32 priority, uint32 priority1p)
{
    uint32 regValue;

    if ( (priority > 3) || (priority1p > 0x7) ) 
        return FAILED;
    
    rtl8306e_reg_get(0, 24, 3, &regValue);
    switch(priority) 
    {
        case 0:
            regValue = (regValue & 0xFFF8) | priority1p;
            break;
        case 1:
            regValue = (regValue & 0xFFC7) | (priority1p << 3);
            break;
        case 2:
            regValue = (regValue & 0xFE3F) | (priority1p <<6);
            break;
        case 3:
            regValue = (regValue & 0xF1FF) | (priority1p <<9);
            break;
        default:
            return FAILED;
    }    
    rtl8306e_reg_set(0, 24, 3, regValue);
    
    return SUCCESS;
}

/* Function Name:
 *      rtl8306_getAsic1pRemarkingPriority
 * Description:
 *      Get 802.1P remarking priority
 * Input:
 *      priority       -  Packet priority(0~4)
 * Output:
 *      pPriority1p  -  the pointer of 802.1P priority(0~7)
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch determines packet priority, the priority souce could
 *      be port-based, 1Q-based, dscp-based, vid-based, ip address,
 *      cpu tag.
 */
int32 rtl8306e_qos_1pRemark_get(uint32 priority, uint32 *pPriority1p) 
{
    uint32 regValue;

    if ( (priority > 3) || (NULL == pPriority1p) ) 
        return FAILED;
    
    rtl8306e_reg_get(0, 24, 3, &regValue);
    switch(priority)
    {
        case 0:
            *pPriority1p = (regValue & 0x7);
            break;
        case 1:
            *pPriority1p = (regValue & 0x0038) >> 3;
            break;
        case 2:
            *pPriority1p = (regValue & 0x01C0) >> 6;
            break;
        case 3:
            *pPriority1p = (regValue & 0x0E00) >> 9;
            break;
        default:
            return FAILED;
    }

    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_qos_portPri_set
 * Description:
 *      Set port-based priority
 * Input:
 *      port          -  port number (0~5)
 *      priority      -  Packet port-based priority(0~3)
 * Output:
*       none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      packet will be assigned a port-based priority correspond to the ingress port.
 */
int32 rtl8306e_qos_portPri_set(uint32 port, uint32 priority)
{
    uint32 regValue;

    if ((port > RTL8306_PORT5) ||(priority > 3))
        return FAILED;
    
    /*Port 5 corresponding PHY6*/
    if (RTL8306_PORT5 == port)  
        port ++ ;  
    
    rtl8306e_reg_get(port, 17, 2, &regValue);
    regValue = (regValue & 0xE7FF) | (priority << 11);
    rtl8306e_reg_set(port, 17, 2, regValue);
    
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_qos_portPri_get
 * Description:
 *      Get port-based priority
 * Input:
 *      port          -  port number (0~5)
 * Output:
 *      pPriority    -   pointer of packet port-based priority(0~4)
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      packet will be assigned a port-based priority correspond to the ingress port.
 */ 
int32 rtl8306e_qos_portPri_get(uint32 port, uint32 *pPriority) 
{
    uint32 regValue;

    if ((port > RTL8306_PORT5) ||(NULL == pPriority))
        return FAILED;
    
    if (port < RTL8306_PORT5) 
        rtl8306e_reg_get(port, 17, 2, &regValue);
    else
        rtl8306e_reg_get(6, 17, 2, &regValue);
    
    *pPriority = (regValue & 0x1800) >> 11;

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_qos_1pPriRemap_set
 * Description:
 *      Set Asic 1Q-tag priority mapping to 2-bit priority
 * Input:
 *      tagprio  -  1Q-tag proirty (0~7, 3 bit value)
 *      prio      -   internal use 2-bit priority
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch internal use 2-bit priority, so it should map 3-bit 1Q-tag priority
 *      to 2-bit priority
 */ 
int32 rtl8306e_qos_1pPriRemap_set(uint32 tagprio, uint32 prio) 
{
    uint32 regValue;

    if ((tagprio > RTL8306_1QTAG_PRIO7) || (prio > RTL8306_PRIO3 ))
        return FAILED;
    
    rtl8306e_reg_get(2, 24, 3, &regValue);
    switch(tagprio) 
    {
        case RTL8306_1QTAG_PRIO0:
            regValue = (regValue & 0xFFFC) | prio;
            break;
        case RTL8306_1QTAG_PRIO1:
            regValue = (regValue & 0xFFF3) |(prio << 2);
            break;
        case RTL8306_1QTAG_PRIO2:
            regValue = (regValue & 0xFFCF) | (prio << 4);
            break;
        case RTL8306_1QTAG_PRIO3:
            regValue = (regValue & 0xFF3F) | (prio << 6);
            break;
        case RTL8306_1QTAG_PRIO4:
            regValue = (regValue & 0xFCFF) | (prio << 8);
            break;
        case RTL8306_1QTAG_PRIO5:
            regValue = (regValue & 0xF3FF) | (prio << 10);
            break;
        case RTL8306_1QTAG_PRIO6:
            regValue = (regValue & 0xCFFF) | (prio << 12);
            break;
        case RTL8306_1QTAG_PRIO7:
            regValue = (regValue & 0x3FFF) | (prio << 14);
            break;
        default:
            return FAILED;
    }    

    rtl8306e_reg_set(2, 24, 3, regValue);    
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_qos_1pPriRemap_get
 * Description:
 *      Get Asic 1Q-tag priority mapping to 2-bit priority
 * Input:
 *      tagprio  -  1Q-tag proirty (0~7, 3 bit value)
 * Output:
 *      pPrio     -  pointer of  internal use 2-bit priority
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch internal use 2-bit priority, so it should map 3-bit 1Q-tag priority
 *      to 2-bit priority
 */ 
int32 rtl8306e_qos_1pPriRemap_get(uint32 tagprio, uint32 *pPrio) 
{
    uint32 regValue;
    
    if ((tagprio > RTL8306_1QTAG_PRIO7) || (NULL == pPrio))
        return FAILED;
    
    rtl8306e_reg_get(2, 24, 3, &regValue);
    switch(tagprio) 
    {
        case RTL8306_1QTAG_PRIO0:
            *pPrio = regValue & 0x3;
            break;
        case RTL8306_1QTAG_PRIO1:
            *pPrio = (regValue & 0xC) >> 2;
            break;
        case RTL8306_1QTAG_PRIO2:
            *pPrio = (regValue & 0x30) >> 4;
            break;
        case RTL8306_1QTAG_PRIO3:
            *pPrio = (regValue & 0xC0) >> 6;
            break;
        case RTL8306_1QTAG_PRIO4:
            *pPrio = (regValue & 0x300) >> 8;
            break;
        case RTL8306_1QTAG_PRIO5:
            *pPrio = (regValue & 0xC00) >> 10;
            break;
        case RTL8306_1QTAG_PRIO6:
            *pPrio = (regValue & 0x3000) >> 12;
            break;
        case RTL8306_1QTAG_PRIO7:
            *pPrio = (regValue & 0xC000) >> 14;
            break;
        default:
            return FAILED;
    }

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_dscpPriRemap_set
 * Description:
 *      Set DSCP-based priority
 * Input:
 *      code      -  dscp code
 *      priority   -  dscp-based priority
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch support 16 kinds of dscp code:
 *          RTL8306_DSCP_EF          
 *                 - DSCP for the Expedited forwarding PHB, 101110   
 *          RTL8306_DSCP_AFL1         
 *                 - DSCP for AF PHB Class 1 low drop, 001010
 *          RTL8306_DSCP_AFM1     
 *                 - DSCP for AF PHB Class 1 medium drop, 001100
 *          RTL8306_DSCP_AFH1      
 *                 - DSCP for AF PHB Class 1 high drop, 001110
 *          RTL8306_DSCP_AFL2       
 *                 - DSCP for AF PHB Class 2 low drop, 01001
 *          RTL8306_DSCP_AFM2       
 *                 - DSCP for AF PHB Class 2 medium drop, 010100
 *          RTL8306_DSCP_AFH2   
 *                 - DSCP for AF PHB Class 2 high drop, 010110
 *          RTL8306_DSCP_AFL3    
 *                 - DSCP for AF PHB Class 3 low drop, 011010
 *          RTL8306_DSCP_AFM3      
 *                 - DSCP for AF PHB Class 3 medium drop, 011100
 *          RTL8306_DSCP_AFH3    
 *                 - DSCP for AF PHB Class 3 high drop, 0111
 *          RTL8306_DSCP_AFL4     
 *                 - DSCP for AF PHB Class 4 low drop, 100010
 *          RTL8306_DSCP_AFM4    
 *                 - DSCP for AF PHB Class 4 medium drop, 100100
 *          RTL8306_DSCP_AFH4     
 *                 - DSCP for AF PHB Class 4 high drop, 100110
 *          RTL8306_DSCP_NC        
 *                 - DSCP for network control, 110000 or 111000
 *          RTL8306_DSCP_REG_PRI 
 *                 - DSCP Register match priority, user could define two dscp code
 *          RTL8306_DSCP_BF        
 *                 - DSCP Default PHB
 *     
 */ 
int32 rtl8306e_qos_dscpPriRemap_set(uint32 code, uint32 priority) 
{
    uint32 regValue1, regValue2;

    if ((code > RTL8306_DSCP_BF) ||(priority > RTL8306_PRIO3))
        return FAILED;

    rtl8306e_reg_get(1, 23, 3, &regValue1);
    rtl8306e_reg_get(1, 24, 3, &regValue2);
    switch(code) 
    {
        case RTL8306_DSCP_EF:
            regValue1 = (regValue1 & 0xFFFC) | priority;
            rtl8306e_reg_set(1, 23, 3, regValue1);
            break;            
        case RTL8306_DSCP_AFL1:
            regValue1 = (regValue1 & 0xFFF3) | (priority << 2);
            rtl8306e_reg_set(1, 23, 3, regValue1);
            break;
        case RTL8306_DSCP_AFM1:
            regValue1 = (regValue1 & 0xFFCF) | (priority << 4);
            rtl8306e_reg_set(1, 23, 3, regValue1);
            break;
        case RTL8306_DSCP_AFH1:
            regValue1 = (regValue1 & 0xFF3F) | (priority << 6);
            rtl8306e_reg_set(1, 23, 3, regValue1);
            break;
        case RTL8306_DSCP_AFL2:
            regValue1 = (regValue1 & 0xFCFF) | (priority << 8);
            rtl8306e_reg_set(1, 23, 3, regValue1);
            break;
        case RTL8306_DSCP_AFM2:
            regValue1 = (regValue1 & 0xF3FF) | (priority << 10);
            rtl8306e_reg_set(1, 23, 3, regValue1);
            break;
        case RTL8306_DSCP_AFH2:
            regValue1 = (regValue1 & 0xCFFF) |(priority << 12);
            rtl8306e_reg_set(1, 23, 3, regValue1);
            break;
        case RTL8306_DSCP_AFL3:
            regValue1 = (regValue1 & 0x3FFF) | (priority << 14);
            rtl8306e_reg_set(1, 23, 3, regValue1);
            break;
        case RTL8306_DSCP_AFM3:
            regValue2 = (regValue2 & 0xFFFC) | priority;
            rtl8306e_reg_set(1, 24, 3, regValue2);
            break;
        case RTL8306_DSCP_AFH3:
            regValue2 = (regValue2 & 0xFFF3) | (priority <<2);
            rtl8306e_reg_set(1, 24, 3, regValue2);
            break;
        case RTL8306_DSCP_AFL4:
            regValue2 = (regValue2 & 0xFFCF) | (priority <<4);
            rtl8306e_reg_set(1, 24, 3, regValue2);
            break;
        case RTL8306_DSCP_AFM4:
            regValue2 = (regValue2 & 0xFF3F) | (priority << 6);
            rtl8306e_reg_set(1, 24, 3, regValue2);
            break;
        case RTL8306_DSCP_AFH4:
            regValue2 = (regValue2 & 0xFCFF) | (priority << 8);
            rtl8306e_reg_set(1, 24, 3, regValue2);
            break;
        case RTL8306_DSCP_NC:
            regValue2 = (regValue2 & 0xF3FF) | (priority << 10);
            rtl8306e_reg_set(1, 24, 3, regValue2);
            break;
        case RTL8306_DSCP_REG_PRI:
            regValue2 = (regValue2 & 0xCFFF) | (priority << 12);
            rtl8306e_reg_set(1, 24, 3, regValue2);
            break;
        case RTL8306_DSCP_BF:
            regValue2 = (regValue2 & 0x3FFF) | (priority << 14);
            rtl8306e_reg_set(1, 24, 3, regValue2);
            break;
        default:
            return FAILED;
    }
    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_dscpPriRemap_set
 * Description:
 *      Get DSCP-based priority
 * Input:
 *      code      -  dscp code
 * Output:
 *      pPriority  -  the pointer of dscp-based priority
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch support 16 kinds of dscp code:
 *          RTL8306_DSCP_EF          
 *                 - DSCP for the Expedited forwarding PHB, 101110   
 *          RTL8306_DSCP_AFL1         
 *                 - DSCP for AF PHB Class 1 low drop, 001010
 *          RTL8306_DSCP_AFM1     
 *                 - DSCP for AF PHB Class 1 medium drop, 001100
 *          RTL8306_DSCP_AFH1      
 *                 - DSCP for AF PHB Class 1 high drop, 001110
 *          RTL8306_DSCP_AFL2       
 *                 - DSCP for AF PHB Class 2 low drop, 01001
 *          RTL8306_DSCP_AFM2       
 *                 - DSCP for AF PHB Class 2 medium drop, 010100
 *          RTL8306_DSCP_AFH2   
 *                 - DSCP for AF PHB Class 2 high drop, 010110
 *          RTL8306_DSCP_AFL3    
 *                 - DSCP for AF PHB Class 3 low drop, 011010
 *          RTL8306_DSCP_AFM3      
 *                 - DSCP for AF PHB Class 3 medium drop, 011100
 *          RTL8306_DSCP_AFH3    
 *                 - DSCP for AF PHB Class 3 high drop, 0111
 *          RTL8306_DSCP_AFL4     
 *                 - DSCP for AF PHB Class 4 low drop, 100010
 *          RTL8306_DSCP_AFM4    
 *                 - DSCP for AF PHB Class 4 medium drop, 100100
 *          RTL8306_DSCP_AFH4     
 *                 - DSCP for AF PHB Class 4 high drop, 100110
 *          RTL8306_DSCP_NC        
 *                 - DSCP for network control, 110000 or 111000
 *          RTL8306_DSCP_REG_PRI 
 *                 - DSCP Register match priority, user could define two dscp code
 *          RTL8306_DSCP_BF        
 *                 - DSCP Default PHB
 *     
 */ 
int32 rtl8306e_qos_dscpPriRemap_get(uint32 code, uint32 *pPriority) 
{
    uint32  regValue1, regValue2;

    if ((code > RTL8306_DSCP_BF) || (NULL == pPriority))
        return FAILED;
    
    rtl8306e_reg_get(1, 23, 3, &regValue1);
    rtl8306e_reg_get(1, 24, 3, &regValue2);
    switch(code) 
    {
        case RTL8306_DSCP_EF:
            *pPriority = regValue1 & 0x3;
            break;
        case RTL8306_DSCP_AFL1:
            *pPriority = (regValue1 & 0xC)  >> 2;
            break;
        case RTL8306_DSCP_AFM1:
            *pPriority = (regValue1 & 0x30) >> 4;
            break;
        case RTL8306_DSCP_AFH1:
            *pPriority = (regValue1 & 0xC0) >> 6;
            break;
        case RTL8306_DSCP_AFL2:
            *pPriority = (regValue1 & 0x300) >> 8;
            break;
        case RTL8306_DSCP_AFM2:
            *pPriority = (regValue1 & 0xC00) >> 10;
            break;
        case RTL8306_DSCP_AFH2:
            *pPriority = (regValue1 & 0x3000) >> 12;
            break;
        case RTL8306_DSCP_AFL3:
            *pPriority = (regValue1 & 0xC000) >> 14;
            break;
        case RTL8306_DSCP_AFM3:
            *pPriority = regValue2 & 0x3;
            break;
        case RTL8306_DSCP_AFH3:
            *pPriority = (regValue2 & 0xC) >> 2;
            break;
        case RTL8306_DSCP_AFL4:
            *pPriority = (regValue2 & 0x30) >> 4;
            break;
        case RTL8306_DSCP_AFM4:
            *pPriority = (regValue2 & 0xC0) >> 6;
            break;
        case RTL8306_DSCP_AFH4:
            *pPriority = (regValue2 & 0x300) >> 8;
            break;
        case RTL8306_DSCP_NC:
            *pPriority = (regValue2 & 0xC00) >>10;
            break;
        case RTL8306_DSCP_REG_PRI:
            *pPriority = (regValue2 & 0x3000) >> 12;
            break;
        case RTL8306_DSCP_BF:
            *pPriority = (regValue2 & 0xC000) >> 14;
            break;
        default:
            return FAILED;
    }

    return SUCCESS;
} 


/* Function Name:
 *      rtl8306e_qos_priSrcArbit_set
 * Description:
 *      Set priority source arbitration level
 * Input:
 *      priArbit  - The structure describe levels of 5 kinds of priority 
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch could recognize 7 types of priority source at most, 
 *      and a packet properly has all of them. among them, there 
 *      are 5 type priorities could be set priority level, they are 
 *      ACL-based  priority, DSCP-based priority, 1Q-based priority,
 *      Port-based priority, VID- based priority.each one could be 
 *      set level from 0 to 5, arbitration module will decide their sequece 
 *      to take, the highest level priority will be adopted at first, 
 *      then  priority type of the sencond highest level. priority with level 0
 *      will not be recognized any more. 
 */

int32 rtl8306e_qos_priSrcArbit_set(rtl8306e_qosPriArbitPara_t priArbit) 
{
    uint32 regval;

    if ((priArbit.acl_pri_lev > 5) || (priArbit.dscp_pri_lev > 5) ||
        (priArbit.dot1q_pri_lev > 5)   || (priArbit.port_pri_lev > 5)  ||
        (priArbit.vid_pri_lev > 5))
        return FAILED;
    
    /*vid based priority selection*/
    rtl8306e_reg_get(0, 28, 3, &regval);
    regval &= ~(0x1F << 8);
    if (priArbit.vid_pri_lev)
        regval |= (1 << (priArbit.vid_pri_lev -1 + 8));
    rtl8306e_reg_set(0, 28, 3, regval);

    /*acl based priority*/
    rtl8306e_reg_get(1, 21, 3, &regval);
    regval &= ~(0xF << 12);
    rtl8306e_reg_set(1, 21, 3, regval);    
    rtl8306e_regbit_set(0, 28, 7, 3, 0);
    if (5 == priArbit.acl_pri_lev)
    {
        rtl8306e_regbit_set(0, 28, 7, 3, 1);
    }
    else if (priArbit.acl_pri_lev > 0)
    {
        rtl8306e_reg_get(1, 21, 3, &regval); 
        regval |= (1 << (priArbit.acl_pri_lev -1 + 12));
        rtl8306e_reg_set(1, 21, 3, regval);
    }
        
    /*dscp based priority*/
    rtl8306e_reg_get(1, 21, 3, &regval);
    regval &= ~(0xF << 8);
    rtl8306e_reg_set(1, 21, 3, regval);    
    rtl8306e_regbit_set(0, 26, 15, 3, 0);
    if (5 == priArbit.dscp_pri_lev)
    {
        rtl8306e_regbit_set(0, 26, 15, 3, 1);
    }
    else if (priArbit.dscp_pri_lev > 0)
    {
        rtl8306e_reg_get(1, 21, 3, &regval);
        regval |= (1 << (priArbit.dscp_pri_lev -1 + 8));
        rtl8306e_reg_set(1, 21, 3, regval);
    }
    
    /*1Q based priority*/
    rtl8306e_reg_get(1, 21, 3, &regval);
    regval &= ~(0xF << 4);
    rtl8306e_reg_set(1, 21, 3, regval);
    rtl8306e_regbit_set(0, 26, 14, 3, 0);
    if (5 == priArbit.dot1q_pri_lev)
    {
        rtl8306e_regbit_set(0, 26, 14, 3, 1);                    
    }
    else if (priArbit.dot1q_pri_lev > 0)
    {
        rtl8306e_reg_get(1, 21, 3, &regval);        
        regval |= ( 1 << (priArbit.dot1q_pri_lev -1 + 4));
        rtl8306e_reg_set(1, 21, 3, regval);
    }

    /*port based priority*/
    rtl8306e_reg_get(1, 21, 3, &regval);
    regval &= ~0xF;
    rtl8306e_reg_set(1, 21, 3, regval);
    rtl8306e_regbit_set(0, 26, 13, 3, 0);
    if (5 == priArbit.port_pri_lev)
    {
        rtl8306e_regbit_set(0, 26, 13, 3, 1);        
    }
    else if (priArbit.port_pri_lev > 0)
    {
        rtl8306e_reg_get(1, 21, 3, &regval); 
        regval |= (1 << (priArbit.port_pri_lev -1));
        rtl8306e_reg_set(1, 21, 3, regval);
    }
    
    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_qos_priSrcArbit_get
 * Description:
 *      Get priority source arbitration level
 * Input:
 *      none 
 * Output:
 *      pPriArbit  - The structure describe levels of 5 kinds of priority 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch could recognize 7 types of priority source at most, 
 *      and a packet properly has all of them. among them, there 
 *      are 5 type priorities could be set priority level, they are 
 *      ACL-based  priority, DSCP-based priority, 1Q-based priority,
 *      Port-based priority, VID- based priority.each one could be 
 *      set level from 0 to 5, arbitration module will decide their sequece 
 *      to take, the highest level priority will be adopted at first, 
 *      then  priority type of the sencond highest level. priority with level 0
 *      will not be recognized any more. 
 */
int32 rtl8306e_qos_priSrcArbit_get(rtl8306e_qosPriArbitPara_t *pPriArbit)
{
    uint32 regval;
    uint32 i ;
        
    if (NULL == pPriArbit)
        return FAILED;

    pPriArbit ->acl_pri_lev      = 0;
    pPriArbit ->dot1q_pri_lev  = 0;
    pPriArbit ->dscp_pri_lev   = 0;
    pPriArbit ->port_pri_lev    = 0;
    pPriArbit ->vid_pri_lev      = 0;

    /*vid based priority selection*/
    rtl8306e_reg_get(0, 28, 3, &regval);
    regval &= (0x1F << 8);
    regval >>= 8;
    for (i = 5; i > 0; i --)
    {
        if(regval >> (i-1))
        {
            pPriArbit ->vid_pri_lev = i;
            break;
        }
    }

    /*acl based priority*/
    rtl8306e_regbit_get(0, 28, 7, 3, &regval);
    if(regval)
    {
        pPriArbit->acl_pri_lev = 5;
    }
    else
    {
        rtl8306e_reg_get(1, 21, 3, &regval);
        regval &= (0xF << 12);
        regval >>= 12;
        for (i = 4; i > 0; i --)
        {
            if(regval >> (i-1))
            {
                pPriArbit ->acl_pri_lev = i;
                break;
            }
        }        
    }

    /*dscp based priority*/
    rtl8306e_regbit_get(0, 26, 15, 3, &regval);
    if (regval)
    {
        pPriArbit ->dscp_pri_lev = 5;
    }
    else
    {
        rtl8306e_reg_get(1, 21, 3, &regval);
        regval &= (0xF << 8);
        regval >>= 8;
        for (i = 4; i > 0; i --)
        {
            if(regval >> (i-1))
            {
                pPriArbit ->dscp_pri_lev =  i;
                break;
            }
        }                
    }

    /*1Q based priority*/
    rtl8306e_regbit_get(0, 26, 14, 3, &regval);
    if (regval)
    {
        pPriArbit ->dot1q_pri_lev = 5;
    }
    else
    {
        rtl8306e_reg_get(1, 21, 3, &regval);
        regval &= (0xF << 4);
        regval >>= 4;
        for (i = 4; i > 0; i --)
        {
            if(regval >> (i-1))
            {
                pPriArbit ->dot1q_pri_lev = i;
                break;
            }
        }                        
    }

    /*port based priority*/
    rtl8306e_regbit_get(0, 26, 13, 3, &regval);
    if (regval)
    {
        pPriArbit ->port_pri_lev = 5;
    }
    else 
    {
        rtl8306e_reg_get(1, 21, 3, &regval);
        regval &= 0xF;

        for (i = 4; i > 0; i --)
        {
            if(regval >> (i-1))
            {
                pPriArbit ->port_pri_lev = i;
                break;
            }
        }                                
    }
        
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_qos_priSrcEnable_set
 * Description:
 *      enable/disable Qos priority source for ingress port
 * Input:
 *      port      -  Specify port number (0 ~5)
 *      priSrc    -  Specify priority source  
 *      enabled -   TRUE of FALSE  
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 4 kind of priority source for each port which could
 *     be enabled ordisabled:
 *          RTL8306_DSCP_PRIO     - DSCP-based priority
 *          RTL8306_1QBP_PRIO     - 1Q-based priority
 *          RTL8306_PBP_PRIO        - port-based priority
 *          RTL8306_CPUTAG_PRIO  - cpu tag priority
 */
int32 rtl8306e_qos_priSrcEnable_set(uint32 port, uint32 priSrc, uint32 enabled) 
{
    uint32 duplex, speed, nway;
      
    if (port > RTL8306_PORT5)
        return FAILED;

    /*save mac 4 or port status when operate reg.22*/    
    if (4 == port) 
    {
        rtl8306e_regbit_get(5, 0, 13, 0, &speed);
        rtl8306e_regbit_get(5, 0, 12, 0, &nway);
        rtl8306e_regbit_get(5, 0, 8, 0, &duplex);            
    } 
    else if (5 == port) 
    {
        rtl8306e_regbit_get(6, 0, 13, 0, &speed);
        rtl8306e_regbit_get(6, 0, 12, 0, &nway);
        rtl8306e_regbit_get(6, 0, 8, 0, &duplex);            
    }

    /*Port 5 corresponding PHY6*/
    if (RTL8306_PORT5 == port)  
        port ++ ;  
    
    switch(priSrc) 
    {
        case RTL8306_DSCP_PRIO:
            rtl8306e_regbit_set(port, 22, 9, 0, enabled ? 0:1);
            break;
        case RTL8306_1QBP_PRIO:
            rtl8306e_regbit_set(port, 22, 10, 0, enabled ? 0:1);
            break;
        case RTL8306_PBP_PRIO:
            rtl8306e_regbit_set(port, 22, 8, 0, enabled ? 0:1);
            break;
        case RTL8306_CPUTAG_PRIO:
            rtl8306e_regbit_set(port, 17, 1, 2, enabled ? 1:0);
            break;
        default:
            return FAILED;
    }
    
    /*restore mac 4 or port status when operate reg.22*/    
    if (4 == port) 
    {
        rtl8306e_regbit_set(5, 0, 13, 0, speed);
        rtl8306e_regbit_set(5, 0, 12, 0, nway);
        rtl8306e_regbit_set(5, 0, 8, 0, duplex);            
    }
    else if (6 == port) 
    { 
        /*for port++ when port 5*/
        rtl8306e_regbit_set(6, 0, 13, 0, speed);
        rtl8306e_regbit_set(6, 0, 12, 0, nway);
        rtl8306e_regbit_set(6, 0, 8, 0, duplex);
    }      
      return SUCCESS;
}


/* Function Name:
 *      rtl8306e_qos_priSrcEnable_set
 * Description:
 *      enable/disable Qos priority source for  ingress port
 * Input:
 *      port       -  Specify port number (0 ~5)
 *      priSrc     -  Specify priority source  
 * Output:
 *      pEnabled -  the pointer of priority source status  
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 4 kind of priority source for each port which could
 *     be enabled ordisabled:
 *          RTL8306_DSCP_PRIO     - DSCP-based priority
 *          RTL8306_1QBP_PRIO     - 1Q-based priority
 *          RTL8306_PBP_PRIO        - port-based priority
 *          RTL8306_CPUTAG_PRIO  - cpu tag priority
 */
int32 rtl8306e_qos_priSrcEnable_get(uint32 port, uint32 priSrc, uint32 *pEnabled) 
{
    uint32 bitValue;

    if ((port > RTL8306_PORT5) || (NULL == pEnabled))
        return FAILED;
    
    /*Port 5 corresponding PHY6*/
    if (port == RTL8306_PORT5 )  
        port ++ ;  
    
    switch(priSrc)
    {
        case RTL8306_DSCP_PRIO:
            rtl8306e_regbit_get(port, 22, 9, 0, &bitValue);
            *pEnabled = (bitValue ? FALSE : TRUE);
            break;
        case RTL8306_1QBP_PRIO:
            rtl8306e_regbit_get(port, 22, 10, 0, &bitValue);
            *pEnabled = (bitValue ? FALSE : TRUE);
            break;
        case RTL8306_PBP_PRIO:
            rtl8306e_regbit_get(port, 22, 8, 0, &bitValue);
            *pEnabled = (bitValue ? FALSE : TRUE);
            break;
        case RTL8306_CPUTAG_PRIO:
            rtl8306e_regbit_get(port, 17, 1, 2, &bitValue);
            *pEnabled = (bitValue ? TRUE : FALSE);
            break;
        default:
            return FAILED;
    }

    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_qos_ipAddrPri_set
 * Description:
 *      Set IP address priority
 * Input:
 *      priority  -  internal use 2-bit priority value (0~3)  
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */
int32 rtl8306e_qos_ipAddrPri_set(uint32 priority) 
{
    uint32 regValue;

    if (priority > 3)
        return FAILED;
    
    rtl8306e_reg_get(2, 22, 3, &regValue);
    rtl8306e_reg_set(2, 22, 3, (regValue & 0xFFFC) |priority);

    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_qos_ipAddrPri_get
 * Description:
 *      Get IP address priority
 * Input:
 *      priority  -  internal use 2-bit priority value (0~3)  
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */
int32 rtl8306e_qos_ipAddrPri_get(uint32 *priority)
{
    uint32 regValue;

    if (priority == NULL)
        return FAILED;
    
    rtl8306e_reg_get(2, 22, 3, &regValue);
    *priority =  regValue & 0x3;

    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_qos_ipAddr_set
 * Description:
 *      Set IP address
 * Input:
 *      entry        -   specify entry
         ip            -   ip address
         mask        -  ip mask
         enabled    -   enable the entry
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are two entries RTL8306_IPADD_A and RTL8306_IPADD_B
 *      for user setting ip address, if ip address of packet matches
 *      the entry, the packet will be assign the priority of ip address
 *      priority which is configured by rtl8306e_qos_ipAddrPri_set.
 */
int32 rtl8306e_qos_ipAddr_set(uint32 entry, uint32 ip, uint32 mask, uint32 enabled) 
{
    uint32 regValue;

    if (entry > 1) 
        return FAILED;

    switch(entry) 
    {
        case RTL8306_IPADD_A:        
            if (enabled == TRUE) 
            {
                rtl8306e_regbit_set(0, 17, 14, 0, 1);
                regValue = ip & 0xFFFF;
                rtl8306e_reg_set(1, 17, 0, regValue);
                regValue = (ip & 0xFFFF0000) >> 16;
                rtl8306e_reg_set(1, 16, 0, regValue);
                regValue = mask & 0xFFFF;
                rtl8306e_reg_set(2, 17, 0, regValue);
                regValue = (mask & 0xFFFF0000) >> 16;
                rtl8306e_reg_set(2, 16, 0, regValue);
            }    
            else 
                rtl8306e_regbit_set(0, 17, 14, 0, 0);
            break;
        case RTL8306_IPADD_B:        
            if (enabled == TRUE)
            {
                rtl8306e_regbit_set(0, 17, 6, 0, 1);
                regValue = ip & 0xFFFF;
                rtl8306e_reg_set(1, 19, 0, regValue);
                regValue = (ip & 0xFFFF0000) >> 16;
                rtl8306e_reg_set(1, 18, 0, regValue);
                regValue = mask & 0xFFFF;
                rtl8306e_reg_set(2, 19, 0, regValue);
                regValue = (mask & 0xFFFF0000) >> 16;
                rtl8306e_reg_set(2, 18, 0, regValue);
            }
            else 
                rtl8306e_regbit_set(0, 17, 6, 0, 0);        
            break;
        default:
            return FAILED;
    }

    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_qos_ipAddr_get
 * Description:
 *      Get IP address user seting
 * Input:
 *      entry       -   specify entry
 * Output:
 *      pIp            -   ip address
         pMask        -   ip mask
         pEnabled    -  enabled or disabled the entry for IP Priority
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */
int32 rtl8306e_qos_ipAddr_get(uint32 entry, uint32 *pIp, uint32 *pMask , uint32 *pEnabled) 
{
    uint32 hi, lo;
    uint32 bitValue;

    if ((entry > 1) || (pIp == NULL) || (pMask == NULL) || (pEnabled == NULL))
        return FAILED;
    
    switch (entry) 
    {
        case RTL8306_IPADD_A :
            rtl8306e_reg_get(1, 17, 0, &lo);
            rtl8306e_reg_get(1, 16, 0, &hi);
            *pIp = lo + (hi << 16);
            rtl8306e_reg_get(2, 17, 0, &lo);
            rtl8306e_reg_get(2, 16, 0, &hi);
            *pMask = lo + (hi << 16);
            rtl8306e_regbit_get(0, 17, 14, 0, &bitValue);
            *pEnabled = (bitValue == 1 ? TRUE : FALSE);
            break;
        case RTL8306_IPADD_B :
            rtl8306e_reg_get(1, 19, 0, &lo);
            rtl8306e_reg_get(1, 18, 0, &hi);
            *pIp = lo + (hi << 16);
            rtl8306e_reg_get(2, 19, 0, &lo);
            rtl8306e_reg_get(2, 18, 0, &hi);
            *pMask = lo + (hi << 16); 
            rtl8306e_regbit_get(0, 17, 6, 0, &bitValue);
            *pEnabled = (bitValue == 1 ? TRUE : FALSE);

            break;
        default :
            return FAILED;
    }

    
    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_qos_schedulingPara_set
 * Description:
 *      Set qos scheduling parameter
 * Input:
 *      set          -  RTL8306_QOS_SET0 or RTL8306_QOS_SET1
 *      sch_para  -  The structure describe schedule parameter
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch supports 4 queue per egress port, scheduling algorithm could be WRR(Weight Round Robin) or 
 *      SP(Strict Priority). Only high queue Q3 and Q2 could be set as strict priority queue. There two set of 
 *      scheduling parameter in whole system(RTL8306_QOS_SET0/RTL8306_QOS_SET1), each egress port select
 *      one of them. The parameter defined by rtl8306e_qosSchPara_t, q0_wt and q1_wt should between 1~127,
 *      that means Q0 and Q1 WRR weight, q2_wt and  q3_wt could be 0~127, 0 means strict priority. q2_n64Kbps 
 *      and q3_n64Kbps means Q2/Q3 queue bandwidth control, unit is 64Kbps.
 */
int32 rtl8306e_qos_schedulingPara_set(uint32 set, rtl8306e_qosSchPara_t sch_para)
{
    uint32 regValue;
    if ((sch_para.q0_wt > 127) || (sch_para.q1_wt > 127) || (sch_para.q2_wt > 127) ||
         (sch_para.q3_wt > 127) || (0 == sch_para.q0_wt ) || (0 == sch_para.q1_wt)  ||
         (sch_para.q2_n64Kbps > 0x7FF) || (sch_para.q3_n64Kbps > 0x7FF))
         return FAILED;

    switch(set)
    {
        case RTL8306_QOS_SET0:   
            rtl8306e_reg_get(5, 20, 3, &regValue);
            regValue &= ~0x7F;
            regValue &= ~(0x7F << 8); 
            regValue |= (sch_para.q0_wt | (sch_para.q1_wt << 8));
            rtl8306e_reg_set(5, 20, 3, regValue);

            regValue = 0;
            regValue |=  (!sch_para.q3_wt) ? (1 << 15) :0;
            regValue |= (sch_para.q3_wt << 8);
            regValue |= (!sch_para.q2_wt) ? (1 << 7):0;
            regValue |= sch_para.q2_wt;
            rtl8306e_reg_set(5, 21, 3, regValue);
            
            rtl8306e_reg_get(5, 18, 3, &regValue);
            regValue &= ~0x7FF;
            regValue |= sch_para.q2_n64Kbps;
            rtl8306e_reg_set(5, 18, 3, regValue);
            
            rtl8306e_reg_get(5, 19, 3, &regValue);
            regValue &= ~0x7FF;
            regValue |= sch_para.q3_n64Kbps;
            rtl8306e_reg_set(5, 19, 3, regValue);                        
            break;

        case RTL8306_QOS_SET1:
            rtl8306e_reg_get(5, 25, 3, &regValue);
            regValue &= ~0x7F;
            regValue &= ~(0x7F << 8); 
            regValue |= (sch_para.q0_wt | (sch_para.q1_wt << 8));
            rtl8306e_reg_set(5, 25, 3, regValue);

            regValue = 0;
            regValue |=  (!sch_para.q3_wt) ? (1 << 15) :0;
            regValue |= (sch_para.q3_wt << 8);
            regValue |= (!sch_para.q2_wt) ? (1 << 7):0;
            regValue |= sch_para.q2_wt;
            rtl8306e_reg_set(5, 26, 3, regValue);

            rtl8306e_reg_get(5, 23, 3, &regValue);
            regValue &= ~0x7FF;
            regValue |= sch_para.q2_n64Kbps;
            rtl8306e_reg_set(5, 23, 3, regValue);

            rtl8306e_reg_get(5, 24, 3, &regValue);
            regValue &= ~0x7FF;
            regValue |= sch_para.q3_n64Kbps;
            rtl8306e_reg_set(5, 24, 3, regValue);                                                                       
            break;

         default:
            return FAILED;
    }
                    
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_qos_schedulingPara_get
 * Description:
 *      Set qos scheduling parameter
 * Input:
 *      set           -  RTL8306_QOS_SET0 or RTL8306_QOS_SET1
 * Output:
 *      pSch_para  - the pointer of schedule parameter
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch supports 4 queue per egress port, scheduling algorithm could be WRR(Weight Round Robin) or 
 *      SP(Strict Priority). Only high queue Q3 and Q3 could be set as strict priority queue. There two set of 
 *      scheduling parameter in whole system(RTL8306_QOS_SET0/RTL8306_QOS_SET1), each egress port select
 *      one of them. The parameter defined by rtl8306e_qosSchPara_t, q0_wt and q1_wt should between 1~127,
 *      that means Q0 and Q1 WRR weight, q2_wt and  q3_wt could be 0~127, 0 means strict priority. q2_n64Kbps 
 *      and q3_n64Kbps means Q2/Q3 queue bandwidth control, unit is 64Kbps.
 */
int32 rtl8306e_qos_schedulingPara_get(uint32 set, rtl8306e_qosSchPara_t *pSch_para)
{
    uint32 regValue;
    
    if (NULL == pSch_para)
        return FAILED;

    switch(set)
    {
        case RTL8306_QOS_SET0:
            rtl8306e_reg_get(5, 20, 3, &regValue);            
            pSch_para->q0_wt = regValue & 0x7F;
            pSch_para->q1_wt = (regValue & (0x7F << 8)) >> 8;

            rtl8306e_reg_get(5, 21, 3, &regValue);
            pSch_para->q2_wt = regValue & 0x7F;
            pSch_para->q3_wt = (regValue & (0x7F << 8)) >> 8;
            if (regValue & (1 << 7))
            {
                pSch_para->q2_wt = 0;
            }
            
            if (regValue & (1 << 15))
            {
                pSch_para->q3_wt = 0;
            }

            rtl8306e_reg_get(5, 18, 3, &regValue);
            pSch_para->q2_n64Kbps = regValue & 0x7FF;
            rtl8306e_reg_get(5, 19, 3, &regValue);
            pSch_para->q3_n64Kbps = regValue & 0x7FF;
                        
            break;
            
        case RTL8306_QOS_SET1:
            rtl8306e_reg_get(5, 25, 3, &regValue);            
            pSch_para->q0_wt = regValue & 0x7F;
            pSch_para->q1_wt = (regValue & (0x7F << 8)) >> 8;
            
            rtl8306e_reg_get(5, 26, 3, &regValue);
            pSch_para->q2_wt = regValue & 0x7F;
            pSch_para->q3_wt = (regValue & (0x7F << 8)) >> 8;

            if (regValue & (1 << 7))
            {
                pSch_para->q2_wt = 0;
            }
            
            if (regValue & (1 << 15))
            {
                pSch_para->q3_wt = 0;
            }
            
            rtl8306e_reg_get(5, 23, 3, &regValue);
            pSch_para->q2_n64Kbps = regValue & 0x7FF;
            rtl8306e_reg_get(5, 24, 3, &regValue);
            pSch_para->q3_n64Kbps = regValue & 0x7FF;            
            
            break;
        default:
            return FAILED;
    }
    
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_qos_portSchedulingMode_set
 * Description:
 *      Select port schedule algorithm  from two sets.
 * Input:
 *      port    -   port number (0 ~ 5)
 *      set     -   RTL8306_QOS_SET0 or RTL8306_QOS_SET1
 *      lbmsk  -   Queue mask for enable queue leaky buckt
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are two sets configuration for schedule mode including strict priority 
 *      enable/disable, queue weight and queue leaky bucket, every port could select
 *      one of them. Queue leaky bucket of each port could be enable separately, so 
 *      you can set queue mask to enable/disable them, because only queue 3 and queue 2
 *      have leaky bucket, only bit 3 and bit 2 of quemask have effect, bit 3 represents
 *      queue 3 and set 1 to enable it.
 */
int32 rtl8306e_qos_portSchedulingMode_set(uint32 port, uint32 set, uint32 lbmsk)
{
    uint32 regValue;
    
    if ((port > RTL8306_PORT5) ||(set > 1))
        return FAILED;
    
    /*Port 5 corresponding PHY6*/
    if (RTL8306_PORT5 == port)  
        port ++ ; 
    
    lbmsk = ((lbmsk & 0x8) >> 3 ) | ((lbmsk & 0x4) >> 1);
    rtl8306e_reg_get(port, 18, 2, &regValue);
    regValue = (regValue & 0x97FF) | (lbmsk << 13) | (set & 0x1) << 11;
    rtl8306e_reg_set(port, 18, 2, regValue);
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_qos_portSchedulingMode_get
 * Description:
 *      Get which set of schedule algorithm  for the specified port 
 * Input:
 *      port    -   port number (0 ~ 5)
 *      set     -   RTL8306_QOS_SET0 or RTL8306_QOS_SET1
 *      lbmsk  -   Queue mask for enable queue leaky buckt
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are two sets configuration for schedule mode including strict priority 
 *      enable/disable, queue weight and queue leaky bucket, every port could select
 *      one of them. Queue leaky bucket of each port could be enable separately, so 
 *      you can set queue mask to enable/disable them, because only queue 3 and queue 2
 *      have leaky bucket, only bit 3 and bit 2 of quemask have effect, bit 3 represents
 *      queue 3 and set 1 to enable it.
 */
int32 rtl8306e_qos_portSchedulingMode_get(uint32 port, uint32 *pSet, uint32 *pLbmsk) 
{
    uint32 regValue;

    if ((port > RTL8306_PORT5) ||(NULL == pSet) || (NULL == pLbmsk))
        return FAILED;

    /*Port 5 corresponding PHY6*/
    if (RTL8306_PORT5 == port)  
        port ++ ;  

    rtl8306e_reg_get(port, 18, 2, &regValue );
    *pSet = (regValue >> 11) & 0x1;
    *pLbmsk = (regValue >> 13) & 0x3;
    *pLbmsk = ((*pLbmsk & 0x1) << 3) | ((*pLbmsk & 0x2) << 1);
    
    return SUCCESS;
}


int32 rtl8306e_qos_queFlcThr_set(uint32 queue, uint32 type, uint32 onoff, uint32 set, uint32 value)
{
    uint32 regValue, mask;
    uint32 selection;
    uint32 reg, shift;

    if ((queue > RTL8306_QUEUE3) || (type > RTL8306_FCO_QLEN) ||
        (onoff > RTL8306_FCON) || (set > RTL8306_FCO_SET1) || (value > 127))
        return FAILED;
    
    selection = (set << 2) | (onoff <<1) |type;
    value &= 0x7f;
    switch (selection) 
    {
        case 0 :  /*set 0, turn off, DSC*/
            if (RTL8306_QUEUE0 == queue)
            {
                reg = 17;
                mask = 0xFFF0;
                shift = 0;
            } 
            else if (RTL8306_QUEUE1 == queue) 
            {
                reg = 17;
                mask = 0xF0FF;
                shift = 8;
            } 
            else if (RTL8306_QUEUE2 == queue) 
            {
                reg = 20;
                mask = 0xFFF0;
                shift = 0;
            } 
            else 
            {
                reg = 20;
                mask = 0xF0FF;
                shift = 8;
            }
            rtl8306e_reg_get(5, reg, 2, &regValue);
            regValue = (regValue & mask) | ((value & 0xf)  << shift);
            rtl8306e_reg_set(5, reg, 2, regValue);
        
             /*flc bit[6:4]*/
            if (RTL8306_QUEUE0 == queue) 
            {
                rtl8306e_reg_get(1, 26, 3, &regValue);  
                regValue &= 0xfff8;
                regValue |= (value & 0x70) >> 4;
                rtl8306e_reg_set(1, 26, 3, regValue);                
            } 
            else if (RTL8306_QUEUE1 == queue) 
            {
                rtl8306e_reg_get(1, 26, 3, &regValue);  
                regValue &= 0xfe3f;
                regValue |= ((value & 0x70) >> 4) << 6;
                rtl8306e_reg_set(1, 26, 3, regValue);
            }
            else if (RTL8306_QUEUE2 == queue) 
            {
                rtl8306e_reg_get(1, 26, 3, &regValue);  
                regValue &= 0x8fff;
                regValue |=   ((value & 0x70) >> 4) << 12;
                rtl8306e_reg_set(1, 26, 3, regValue);                
            } 
            else  
            {
                rtl8306e_reg_get(1, 27, 3, &regValue);  
                regValue &= 0xffe3;
                regValue |=   ((value & 0x70) >> 4) << 2;
                rtl8306e_reg_set(1, 27, 3, regValue);
            }             
            break;
        
        case 1 :     /*set 0, turn off, QLEN*/
            if (RTL8306_QUEUE0 == queue) 
            {
                reg = 17;
                mask = 0xFF0F;
                shift = 4;
            } 
            else if (RTL8306_QUEUE1 == queue) 
            {
                reg = 17;
                mask = 0x0FFF;
                shift = 12;
            } 
            else if (RTL8306_QUEUE2 == queue) 
            {
                reg = 20;
                mask = 0xFF0F;
                shift = 4;
            } 
            else  
            {
                reg = 20;
                mask = 0x0FFF;
                shift = 12;
            }
            rtl8306e_reg_get(5, reg, 2, &regValue);
            regValue = (regValue & mask) | ((value & 0xf) << shift);
            rtl8306e_reg_set(5, reg, 2, regValue);

            /*bit[6:4]*/
            if (RTL8306_QUEUE0 == queue) 
            {
                rtl8306e_reg_get(1, 26, 3, &regValue);  
                regValue &= 0xffc7;
                regValue |= ((value & 0x70) >> 4) << 3;
                rtl8306e_reg_set(1, 26, 3, regValue);                                   
             } 
             else if (RTL8306_QUEUE1 == queue) 
             {
                rtl8306e_reg_get(1, 26, 3, &regValue);  
                regValue &= 0xf1ff;
                regValue |= ((value & 0x70) >> 4) << 9;
                rtl8306e_reg_set(1, 26, 3, regValue);                                                   
             }
             else if (RTL8306_QUEUE2 == queue) 
             {
                /*bit[4]*/
                rtl8306e_reg_get(1, 26, 3, &regValue);                  
                regValue &= 0x7fff;
                regValue |= (((value & 0x10) >> 4) << 15);
                rtl8306e_reg_set(1, 26, 3, regValue);   

                /*bit[6:5]*/
                rtl8306e_reg_get(1, 27, 3, &regValue);         
                regValue &= 0xfffc;
                regValue |= ((value & 0x60) >> 5) ;
                rtl8306e_reg_set(1, 27, 3, regValue);
             } 
            else  
            {
                rtl8306e_reg_get(1, 27, 3, &regValue);       
                regValue &= 0xff1f;
                regValue |= ((value & 0x70) >> 4) << 5;
                rtl8306e_reg_set(1, 27, 3, regValue);
            }             
                     
            break;
        case 2 :    /*set 0, turn on, DSC*/
            if (RTL8306_QUEUE0 == queue) 
                reg = 18;
            else if (RTL8306_QUEUE1 == queue) 
                reg = 19;
            else if (RTL8306_QUEUE2 == queue)
                reg = 21;
            else  
                reg = 22;
            rtl8306e_reg_get(5, reg, 2, &regValue);
            regValue = (regValue & 0xFFC0) | (value & 0x3f);
            rtl8306e_reg_set(5, reg, 2, regValue);

             /*bit[6]*/
            if (RTL8306_QUEUE0 == queue) 
            {
                rtl8306e_regbit_set(5, 21, 7, 2, (value & 0x40) ? 1:0);            
            }      
            else if (RTL8306_QUEUE1 == queue) 
            {
                rtl8306e_regbit_set(5, 21, 15, 2, (value & 0x40) ? 1:0);            
            }
            else if (RTL8306_QUEUE2 == queue)
            {
                rtl8306e_regbit_set(5, 24, 7, 2, (value & 0x40) ? 1:0);                        
            }
            else 
            {
                rtl8306e_regbit_set(5, 24, 15, 2, (value & 0x40) ? 1:0);                        
            }            
            break;
            
        case 3:   /*set 0, turn  on, QLEN*/
            if (RTL8306_QUEUE0 == queue) 
                reg = 18;
            else if (RTL8306_QUEUE1 == queue) 
                reg = 19;
            else if (RTL8306_QUEUE2 == queue) 
                reg = 21;
            else  
                reg = 22;
            if (queue != RTL8306_QUEUE3)  
            {
                rtl8306e_reg_get(5, reg, 2, &regValue);
                regValue = (regValue & 0xC0FF) | ((value & 0x3f) << 8);
                rtl8306e_reg_set(5, reg, 2, regValue);
            }  
            else 
            {
                rtl8306e_reg_get(5, reg, 2, &regValue);
                regValue = (regValue & 0x3FF) | ((value & 0x3f) << 10);
                rtl8306e_reg_set(5, reg, 2, regValue);
            }
        
             /*bit[6]*/
            if (RTL8306_QUEUE0 == queue) 
            {
                rtl8306e_regbit_set(5, 21, 6, 2, (value & 0x40) ? 1:0);    
            }      
            else if (RTL8306_QUEUE1 == queue) 
            {
                rtl8306e_regbit_set(5, 21, 14, 2, (value & 0x40) ? 1:0);                      
            }
            else if (RTL8306_QUEUE2 == queue)
            {
                rtl8306e_regbit_set(5, 24, 6, 2, (value & 0x40) ? 1:0);                        
            }
            else 
            {
                rtl8306e_regbit_set(5, 24, 14, 2, (value & 0x40) ? 1:0);                        
            }                   
            break;
        
        case 4:     /*set 1, turn off, DSC*/
            if (RTL8306_QUEUE0 == queue) 
            {
                reg = 23;
                mask = 0xFFF0;
                shift =0;
            } 
            else if (RTL8306_QUEUE1 == queue) 
            {
                reg = 23;
                mask = 0xF0FF;
                shift =8;
            } 
            else if (RTL8306_QUEUE2 == queue)
            {
                reg = 26;
                mask = 0xFFF0;
                shift =0;
            }
            else 
            {
                reg = 26;
                mask = 0xF0FF;
                shift =8;
            }
            rtl8306e_reg_get(5, reg, 2, &regValue);
            regValue = (regValue & mask) | ((value & 0xf) << shift);
            rtl8306e_reg_set(5, reg, 2, regValue);

            /*flc bit[6:4]*/
            if (RTL8306_QUEUE0 == queue) 
            {
                rtl8306e_reg_get(1, 27, 3, &regValue);  
                regValue &= 0xf8ff;
                regValue |= ((value & 0x70) >> 4) << 8;
                rtl8306e_reg_set(1, 27, 3, regValue);                
             } 
             else if (RTL8306_QUEUE1 == queue) 
             {                
                rtl8306e_reg_get(1, 27, 3, &regValue);  
                regValue &= 0x3fff;
                regValue |= ((value & 0x30) >> 4) << 14;
                rtl8306e_reg_set(1, 27, 3, regValue);           
                rtl8306e_regbit_set(1, 28, 0, 3, (value & 0x40) ? 1:0);
                    
            }
            else if (RTL8306_QUEUE2 == queue) 
            {
                rtl8306e_reg_get(1, 28, 3, &regValue);
                regValue &= 0xff8f;
                regValue |= ((value & 0x70) >> 4) << 4;
                rtl8306e_reg_set(1, 28, 3, regValue);
            } 
            else  
            {
                rtl8306e_reg_get(1, 28, 3, &regValue);
                regValue &= 0xe3ff;
                regValue |= ((value & 0x70) >> 4) << 10;
                rtl8306e_reg_set(1, 28, 3, regValue);                
            }                        
            break;
        
        case 5:     /*set 1, turn off, QLEN*/
            if (RTL8306_QUEUE0 == queue) 
            {
                reg = 23;
                mask = 0xFF0F;
                shift = 4;
            } 
            else if (RTL8306_QUEUE1 == queue) 
            {
                reg = 23;
                mask = 0x0FFF;
                shift = 12;
            } 
            else if (RTL8306_QUEUE2 == queue) 
            {
                reg = 26;
                mask = 0xFF0F;
                shift = 4;
            } 
            else 
            {
                reg = 26;
                mask = 0x0FFF;
                shift = 12;
            }
            rtl8306e_reg_get(5, reg, 2, &regValue);
            regValue = (regValue & mask) | ((value & 0xf) << shift);
            rtl8306e_reg_set(5, reg, 2, regValue);

             /*bit[6:4]*/
            if (RTL8306_QUEUE0 == queue) 
            {
                rtl8306e_reg_get(1, 27, 3, &regValue);  
                regValue &= 0xc7ff;
                regValue |= ((value & 0x70) >> 4) << 11;
                rtl8306e_reg_set(1, 27, 3, regValue);
            } 
            else if (RTL8306_QUEUE1 == queue) 
            {
                rtl8306e_reg_get(1, 28, 3, &regValue);  
                regValue &= 0xfff1;
                regValue |= ((value & 0x70) >> 4) << 1;
                rtl8306e_reg_set(1, 28, 3, regValue);                                
            }
            else if (RTL8306_QUEUE2 == queue) 
            {
                rtl8306e_reg_get(1, 28, 3, &regValue);  
                regValue &= 0xfc7f;
                regValue |= ((value & 0x70) >> 4) << 7;
                rtl8306e_reg_set(1, 28, 3, regValue);                                
            } 
            else  
            {
                rtl8306e_reg_get(1, 28, 3, &regValue);
                regValue &= 0x1fff;
                regValue |= ((value & 0x70) >> 4) << 13;
                rtl8306e_reg_set(1, 28, 3, regValue);                                                
            }                                 
            break;
        
        case 6:    /*set 1, turn on, DSC*/
            if (RTL8306_QUEUE0 == queue) 
                reg = 24;
            else if  (RTL8306_QUEUE1 == queue) 
                reg =25;
            else if (RTL8306_QUEUE2 == queue) 
                reg = 27;
            else  
                reg = 28;
            rtl8306e_reg_get(5, reg, 2, &regValue);
            regValue = (regValue & 0xFFC0) | (value & 0x3f);
            rtl8306e_reg_set(5, reg, 2, regValue);

             /*bit[6]*/
            if (RTL8306_QUEUE0 == queue) 
            {
                rtl8306e_regbit_set(5, 25, 7, 2, (value & 0x40) ? 1:0);            
            }      
            else if(RTL8306_QUEUE1 == queue) 
            {
                rtl8306e_regbit_set(5, 25, 15, 2, (value & 0x40) ? 1:0);            
            }
            else if(RTL8306_QUEUE2 == queue)
            {
                rtl8306e_regbit_set(5, 27, 7, 2, (value & 0x40) ? 1:0);            
            }
            else 
            {
                rtl8306e_regbit_set(5, 27, 15, 2, (value & 0x40) ? 1:0);            
            }              
            break;
        
        case 7:     /*set 1, turn  on, QLEN*/
            if (RTL8306_QUEUE0 == queue) 
                reg = 24;
            else if (RTL8306_QUEUE1 == queue) 
                reg =25;
            else if (RTL8306_QUEUE2 == queue) 
                reg = 27;
            else  
                reg = 28;
            if (queue != RTL8306_QUEUE3)  
            {
                rtl8306e_reg_get(5, reg, 2, &regValue);
                regValue = (regValue & 0xC0FF) | ((value & 0x3f) << 8);
                rtl8306e_reg_set(5, reg, 2, regValue);
            } 
            else 
            {
                rtl8306e_reg_get(5, reg, 2, &regValue);
                regValue = (regValue & 0x3FF) | ((value & 0x3f) << 10);
                rtl8306e_reg_set(5, reg, 2, regValue);
            }

            /*bit[6]*/
            if (RTL8306_QUEUE0 == queue) 
            {
                rtl8306e_regbit_set(5, 25, 6, 2, (value & 0x40) ? 1:0);            
            }      
            else if(RTL8306_QUEUE1 == queue) 
            {
                rtl8306e_regbit_set(5, 25, 14, 2, (value & 0x40) ? 1:0);            
            } 
            else if (RTL8306_QUEUE2 == queue)
            {
                rtl8306e_regbit_set(5, 27, 6, 2, (value & 0x40) ? 1:0);                        
            } 
            else 
            {
                rtl8306e_regbit_set(5, 27, 14, 2, (value & 0x40) ? 1:0);                        
            }              
            break;
            
        default:
            return FAILED;
       }

    return SUCCESS;
}

int32 rtl8306e_qos_queFlcThr_get(uint32 queue, uint32 type, uint32 onoff, uint32 set, uint32* pValue) 
{
    uint32 regValue, mask;
    uint32 selection;
    uint32 reg, shift = 0;

    if ((queue > RTL8306_QUEUE3) || (type > RTL8306_FCO_QLEN) || (onoff > RTL8306_FCON) ||
        (set > RTL8306_FCO_SET1) || (NULL == pValue))
        return FAILED;
    
    selection = (set << 2) | (onoff <<1) |type;
    *pValue = 0;
    switch (selection) 
    {
        case 0 :    /*set 0, turn off, DSC*/
            if (RTL8306_QUEUE0 == queue) 
            {
                reg = 17;
                mask = 0xF;
                shift = 0;
            } 
            else if (RTL8306_QUEUE1 == queue) 
            {
                reg = 17;
                mask = 0x0F00;
                shift = 8;
            }
            else if (RTL8306_QUEUE2 == queue) 
            {
                reg = 20;
                mask = 0xF;
                shift = 0;
            } else 
            {
                reg = 20;
                mask = 0x0F00;
                shift = 8;
            }
            rtl8306e_reg_get(5, reg, 2, &regValue);
            *pValue = (regValue & mask) >> shift;

            if (RTL8306_QUEUE0 == queue) 
            {
                rtl8306e_reg_get(1, 26, 3, &regValue);
                regValue &= 0x7;
                *pValue |= (regValue << 4);                    
             } 
            else if (RTL8306_QUEUE1 == queue) 
            {
                rtl8306e_reg_get(1, 26, 3, &regValue);
                regValue &= 0x1c0;
                regValue = regValue >> 6;
                *pValue |= (regValue << 4);                     
            } 
            else if (RTL8306_QUEUE2 == queue) 
            {
                rtl8306e_reg_get(1, 26, 3, &regValue);
                regValue &= 0x7000;
                regValue = regValue >> 12;
                *pValue |= (regValue << 4);                                
            } 
            else  
            {
                rtl8306e_reg_get(1, 27, 3, &regValue);
                regValue &= 0x1c;
                regValue = regValue >> 2;
                *pValue |= (regValue << 4);                                                  
            }                     
            break;
        
        case 1 :    /*set 0, turn off, QLEN*/
            if (RTL8306_QUEUE0 == queue) 
            {
                reg = 17;
                mask = 0x00F0;
                shift = 4;
            } 
            else if (RTL8306_QUEUE1 == queue) 
            {
                reg = 17;
                mask = 0xF000;
                shift = 12;
            } 
            else if (RTL8306_QUEUE2 == queue) 
            {
                reg = 20;
                mask = 0x00F0;
                shift = 4;
            }
            else  
            {
                reg = 20;
                mask = 0xF000;
                shift = 12;
            }
            rtl8306e_reg_get(5, reg, 2, &regValue);
            *pValue = (regValue & mask) >> shift;

            if (RTL8306_QUEUE0 == queue) 
            {
                rtl8306e_reg_get(1, 26, 3, &regValue);
                regValue &= 0x38;
                regValue = regValue >> 3;
                *pValue |= (regValue << 4);                                                                           
             } 
            else if (RTL8306_QUEUE1 == queue) 
            {
                rtl8306e_reg_get(1, 26, 3, &regValue);
                regValue &= 0xe00;
                regValue = regValue >> 9;
                *pValue |= (regValue << 4);                                                                                               
            }
            else if (RTL8306_QUEUE2 == queue)
            {
                rtl8306e_reg_get(1, 26, 3, &regValue);
                regValue &= 0x8000;
                regValue = regValue >> 15;
                *pValue |= (regValue << 4);                                                                                               
                  
                rtl8306e_reg_get(1, 27, 3, &regValue);
                regValue &= 0x3;
                *pValue |= (regValue << 5);                                                                                                       
            } 
            else  
            {
                rtl8306e_reg_get(1, 27, 3, &regValue);
                regValue &= 0xe0;
                regValue = regValue >> 5;
                *pValue |= (regValue << 4);  
            }
        
        break;
        
        case 2 :    /*set 0, turn on, DSC*/
            if (RTL8306_QUEUE0 == queue) 
                reg = 18;
            else if (RTL8306_QUEUE1 == queue) 
                reg = 19;
            else if (RTL8306_QUEUE2 == queue)
                reg = 21;
            else  
                reg = 22;
            rtl8306e_reg_get(5, reg, 2, &regValue);
            *pValue = regValue & 0x3F;

            if (RTL8306_QUEUE0 == queue) 
            {
                rtl8306e_regbit_get(5, 21, 7, 2, &regValue);
                *pValue |= (regValue << 6);
            }      
            else if (RTL8306_QUEUE1 == queue) 
            {
                rtl8306e_regbit_get(5, 21, 15, 2, &regValue);
                *pValue |= (regValue << 6);                
            }      
            else if (RTL8306_QUEUE2 == queue)
            {
                rtl8306e_regbit_get(5, 24, 7, 2, &regValue);
                *pValue |= (regValue << 6);                             
            }      
            else  
            {
                rtl8306e_regbit_get(5, 24, 15, 2, &regValue);
                *pValue |= (regValue << 6);                             
            }              
            break;
            
    case 3:   /*set 0, turn  on, QLEN*/
        if (RTL8306_QUEUE0 == queue) 
            reg = 18;
        else if  (RTL8306_QUEUE1 == queue) 
            reg = 19;
        else if (RTL8306_QUEUE2 == queue) 
            reg = 21;
        else  
            reg = 22;
        if (queue != RTL8306_QUEUE3) 
        {
            rtl8306e_reg_get(5, reg, 2, &regValue);
            *pValue = (regValue & 0x3F00) >> 8 ;
        } else 
        {
            rtl8306e_reg_get(5, reg, 2, &regValue);
            *pValue = (regValue & 0xFC00) >> 10;
        }

        if (RTL8306_QUEUE0 == queue) 
        {
            rtl8306e_regbit_get(5, 21, 6, 2, &regValue);
            *pValue |= (regValue << 6);
        }      
        else if (RTL8306_QUEUE1 == queue) 
        {
            rtl8306e_regbit_get(5, 21, 14, 2, &regValue);
            *pValue |= (regValue << 6);                
        }      
        else if (RTL8306_QUEUE2 == queue)
        {
            rtl8306e_regbit_get(5, 24, 6, 2, &regValue);
            *pValue |= (regValue << 6);                             
        }      
        else  
        {
            rtl8306e_regbit_get(5, 24, 14, 2, &regValue);
            *pValue |= (regValue << 6);                             
        }               
        break;
        
        case 4:     /*set 1, turn off, DSC*/
            if (RTL8306_QUEUE0 == queue) 
            {
                reg = 23;
                mask = 0x000F;
                shift =0;
            } 
            else if (RTL8306_QUEUE1 == queue) 
            {
                reg = 23;
                mask = 0x0F00;
                shift =8;
            }
            else if (RTL8306_QUEUE2 == queue)
            {
                reg = 26;
                mask = 0x000F;
                shift =0;
            }
            else 
            {
                reg = 26;
                mask = 0x0F00;
                shift =8;
            }
            rtl8306e_reg_get(5, reg, 2, &regValue);
            *pValue = (regValue & mask) >> shift;
        
            if (RTL8306_QUEUE0 == queue) 
            {
                rtl8306e_reg_get(1, 27, 3, &regValue);
                regValue &= 0x700;
                regValue = regValue >> 8;                 
                *pValue |= (regValue << 4);                    
            }
            else if (RTL8306_QUEUE1 == queue) 
            {
                rtl8306e_reg_get(1, 27, 3, &regValue);
                regValue &= 0xc000;
                regValue = regValue >> 14;                 
                *pValue |= (regValue << 4);
                rtl8306e_regbit_get(1, 28, 0, 3, &regValue);
                *pValue |= (regValue << 6);                 
            }
            else if (RTL8306_QUEUE2 == queue) 
            {
                rtl8306e_reg_get(1, 28, 3, &regValue);
                regValue &= 0x70;
                 regValue = regValue >> 4;                 
                 *pValue |= (regValue << 4);  
            } else  
            {
                rtl8306e_reg_get(1, 28, 3, &regValue);
                regValue &= 0x1c00;
                regValue = regValue >> 10;                 
                *pValue |= (regValue << 4);  
            }                             
            break;
        
    case 5:     /*set 1, turn off, QLEN*/
        if (RTL8306_QUEUE0 == queue) 
        {
            reg = 23;
            mask = 0xF0;
            shift = 4;
        } 
        else if (RTL8306_QUEUE1 == queue)
        {
            reg = 23;
            mask = 0xF000;
            shift = 12;
        }
        else if (RTL8306_QUEUE2 == queue)
        {
            reg = 26;
            mask = 0xF0;
            shift = 4;
        } 
        else  
        {
            reg = 26;
            mask = 0xF000;
            shift = 12;
        }
        rtl8306e_reg_get(5, reg, 2, &regValue);
        *pValue = (regValue & mask) >> shift;

        if (RTL8306_QUEUE0 == queue) 
        {
            rtl8306e_reg_get(1, 27, 3, &regValue);
            regValue &= 0x3800;
            regValue = regValue >> 11;                 
            *pValue |= (regValue << 4);                    
        } 
        else if (RTL8306_QUEUE1 == queue) 
        {
            rtl8306e_reg_get(1, 28, 3, &regValue);
            regValue &= 0xe;
            regValue = regValue >> 1;                 
            *pValue |= (regValue << 4);                    
        } 
        else if (RTL8306_QUEUE2 == queue)   
        {
            rtl8306e_reg_get(1, 28, 3, &regValue);
            regValue &= 0x380;
            regValue = regValue >> 7;                 
            *pValue |= (regValue << 4);
        }
        else  
        {
            rtl8306e_reg_get(1, 28, 3, &regValue);
            regValue &= 0xe000;
            regValue = regValue >> 13;                 
            *pValue |= (regValue << 4);
        }                             
        break;
        
        case 6:     /*set 1, turn on, DSC*/
            if (RTL8306_QUEUE0 == queue) 
                reg = 24;
            else if (RTL8306_QUEUE1 == queue) 
                reg =25;
            else if (RTL8306_QUEUE2 == queue) 
                reg = 27;
            else  
                reg = 28;
            rtl8306e_reg_get(5, reg, 2, &regValue);
            *pValue = (regValue & 0x3F) >> shift;
        
             /*bit 6*/
            if (RTL8306_QUEUE0 == queue) 
            {
                rtl8306e_regbit_get(5, 25, 7, 2, &regValue);
                *pValue |= (regValue << 6);
            }      
            else if (RTL8306_QUEUE1 == queue) 
            {
                 rtl8306e_regbit_get(5, 25, 15, 2, &regValue);
                 *pValue |= (regValue << 6);                
             }      
            else if (RTL8306_QUEUE2 == queue)
            {
                rtl8306e_regbit_get(5, 27, 7, 2, &regValue);
                *pValue |= (regValue << 6);                             
            }      
            else  
            {
                rtl8306e_regbit_get(5, 27, 15, 2, &regValue);
                *pValue |= (regValue << 6);                             
            }      
        
            break;
        
        case 7:      /*set 1, turn  on, QLEN*/
            if (RTL8306_QUEUE0 == queue) 
                reg = 24;
            else if (RTL8306_QUEUE1 == queue) 
                reg =25;
            else if (RTL8306_QUEUE2 == queue) 
                reg = 27;
            else  
                reg = 28;
            if (queue != RTL8306_QUEUE3) 
            {
                rtl8306e_reg_get(5, reg, 2, &regValue);
                *pValue = (regValue & 0x3F00) >> 8 ;
            } 
            else 
            {
                rtl8306e_reg_get(5, reg, 2, &regValue);
                *pValue = (regValue & 0xFC00) >> 10;
            }
             /*bit 6*/
            if (RTL8306_QUEUE0 == queue) 
             {
                rtl8306e_regbit_get(5, 25, 6, 2, &regValue);
                *pValue |= (regValue << 6);
             }      
             else if (RTL8306_QUEUE1 == queue) 
             {
                 rtl8306e_regbit_get(5, 25, 14, 2, &regValue);
                 *pValue |= (regValue << 6);                
             }      
            else if (RTL8306_QUEUE2 == queue)
            {
                rtl8306e_regbit_get(5, 27, 6, 2, &regValue);
                *pValue |= (regValue << 6);                             
            }      
            else  
            {
                rtl8306e_regbit_get(5, 27, 14, 2, &regValue);
                 *pValue |= (regValue << 6);                             
            }               

        default:
            return FAILED;
    }

    return SUCCESS;
}

int32 rtl8306e_qos_portFlcThr_set(uint32 port, uint32 onthr, uint32 offthr, uint32 direction ) 
{
    uint32 regValue;

    if ((port > RTL8306_PORT5) || (direction > 1))
        return FAILED;
    
    regValue = (offthr << 8) + onthr;

    /*Port 5 corresponding PHY6*/
    if (RTL8306_PORT5 == port)  
        port ++ ;
    
    if (RTL8306_PORT_TX == direction) 
        rtl8306e_reg_set(port, 20, 2, regValue);
    else 
        rtl8306e_reg_set(port, 19, 3, regValue);
    return SUCCESS;
}

int32 rtl8306e_qos_portFlcThr_get(uint32 port, uint32 *pOnthr, uint32 *pOffthr, uint32 direction) 
{
    uint32 regValue;

    if ((port > RTL8306_PORT5) || (NULL == pOnthr) || (NULL == pOffthr) || (direction > 1))
        return FAILED;
    
    /*Port 5 corresponding PHY6*/
    if (RTL8306_PORT5 == port)  
        port ++ ; 
    
    if (RTL8306_PORT_TX == direction) 
        rtl8306e_reg_get(port, 20, 2, &regValue);
    else 
        rtl8306e_reg_get(port, 19, 3, &regValue);
    *pOnthr = regValue & 0xFF;
    *pOffthr = (regValue & 0xFF00) >> 8;
    return SUCCESS;
}


int32 rtl8306e_qos_portFlcMode_set(uint32 port, uint32 set) 
{
    if ((port > RTL8306_PORT5) || (set > RTL8306_FCO_SET1))
        return FAILED;
    
    if (port < RTL8306_PORT5) 
        rtl8306e_regbit_set(port, 18, 12, 2, set);
    else 
        rtl8306e_regbit_set(6, 18, 12, 2, set);
    return SUCCESS;
}


int32 rtl8306e_qos_portFlcMode_get(uint32 port , uint32 *set) 
{
    if ((port > RTL8306_PORT5) || (set == NULL))
        return FAILED;
    
    if (port < RTL8306_PORT5)
        rtl8306e_regbit_get(port, 18, 12, 2, set);
    else
        rtl8306e_regbit_get(6, 18, 12, 2, set);
        return SUCCESS;
}

int32 rtl8306e_qos_queFlcEnable_set( uint32 port, uint32 queue, uint32 enabled) 
{

    if ((port > RTL8306_PORT5) ||(queue > RTL8306_QUEUE3))
        return FAILED;
    
    /*Enable/Disable Flow control of the specified queue*/
    switch (port) 
    {
        case RTL8306_PORT0:
            rtl8306e_regbit_set(6, 17, (4 + queue), 0, enabled ? 0:1);
            break;
        case RTL8306_PORT1:
            rtl8306e_regbit_set(6, 18, (4 + queue), 0, enabled ? 0:1);
            break;
        case RTL8306_PORT2:
            rtl8306e_regbit_set(6, 19, (4 + queue), 0, enabled ? 0:1);
            break;
        case RTL8306_PORT3:
            rtl8306e_regbit_set(6, 20, (4 + queue), 0, enabled ? 0:1);
            break;
        case RTL8306_PORT4:
            rtl8306e_regbit_set(6, 21, (4 + queue), 0, enabled ? 0:1);
            break;
        case RTL8306_PORT5:
            rtl8306e_regbit_set(6, 23, (4 + queue), 0, enabled ? 0:1);
            break;    
        default:
            return FAILED;
    }
    return SUCCESS;
}

int32 rtl8306e_qos_queFlcEnable_get(uint32 port, uint32 queue, uint32 *pEnabled) 
{
    uint32 bitValue;

    if ((port > RTL8306_PORT5) || (queue > RTL8306_QUEUE3) || (NULL == pEnabled))
        return FAILED;
    
    switch (port) 
    {
        case RTL8306_PORT0:
            rtl8306e_regbit_get(6, 17, (4 + queue), 0, &bitValue);
            break;
        case RTL8306_PORT1:
            rtl8306e_regbit_get(6, 18, (4 + queue), 0, &bitValue);
            break;
        case RTL8306_PORT2:
            rtl8306e_regbit_get(6, 19, (4 + queue), 0, &bitValue);
            break;
        case RTL8306_PORT3:
            rtl8306e_regbit_get(6, 20, (4 + queue), 0, &bitValue);
            break;
        case RTL8306_PORT4:
            rtl8306e_regbit_get(6, 21, (4 + queue), 0, &bitValue);
            break;
        case RTL8306_PORT5:
            rtl8306e_regbit_get(6, 23, (4 + queue), 0, &bitValue);
            break;    
        default:
            return FAILED;
    }
    *pEnabled = (bitValue ? FALSE : TRUE);
    
    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_acl_entry_set
 * Description:
 *      Set Asic ACL table
 * Input:
 *      entryadd   - Acl entry address (0~15)
 *      phyport    -  Acl physical port
 *      action      -  Acl action 
 *      protocol   -  Acl protocol
 *      data        -  ether type value or TCP/UDP port
 *      priority     -  Acl priority
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      phyport could be 
 *            0~5:       port number, 
 *            RTL8306_ACL_INVALID: invalid entry,
 *            RTL8306_ACL_ANYPORT: any port.
 *      Acl action could be
 *          RTL8306_ACT_DROP,
 *          RTL8306_ACT_PERMIT, 
 *          RTL8306_ACT_TRAPCPU, 
 *          RTL8306_ACT_MIRROR
 *      Acl protocol could be
 *          RTL8306_ACL_ETHER(ether type), 
 *          RTL8306_ACL_TCP(TCP), 
 *          RTL8306_ACL_UDP(UDP),
 *          RTL8306_ACL_TCPUDP(TCP or UDP)
 *
 *      Acl priority:
 *          RTL8306_PRIO0~RTL8306_PRIO3
 *       
 */

int32 rtl8306e_acl_entry_set(uint32 entryadd, uint32 phyport, uint32 action, uint32 protocol, uint32 data, uint32 priority) 
{
    uint32 regValue, value;
    uint32 pollcnt  ;
    uint32 bitValue;

    if ((entryadd > RTL8306_ACL_ENTRYNUM - 1) || (phyport > RTL8306_ACL_ANYPORT) || 
        (action > RTL8306_ACT_MIRROR) ||(protocol > RTL8306_ACL_TCPUDP) ||
        (priority > RTL8306_PRIO3))
        return FAILED;

    /*Enable CPU port function, Enable inserting CPU TAG, Enable removing CPU TAG */
    rtl8306e_reg_get(2, 21, 3, &regValue);
    regValue = (regValue & 0x7FFF) | (1 << 11) | (1<<12);
    rtl8306e_reg_set(2, 21, 3, regValue);
    
    /*set EtherType or TCP/UDP Ports, ACL entry access register 0*/
    rtl8306e_reg_set(3, 21, 3, data);
    
    /*set ACL entry access register 1*/
    rtl8306e_reg_get(3, 22, 3, &regValue);
    value = (1 << 14) | (entryadd << 9)  | (priority << 7) | (action << 5) | (phyport << 2) | protocol ;
    regValue = (regValue & 0x8000) | value  ;
    rtl8306e_reg_set(3, 22, 3, regValue);
    
    /*Polling whether the command is done*/
    for (pollcnt = 0; pollcnt < RTL8306_IDLE_TIMEOUT; pollcnt++) 
    {
        rtl8306e_regbit_get(3, 22, 14, 3, &bitValue);
        if (!bitValue)
            break;
    }
    if (pollcnt == RTL8306_IDLE_TIMEOUT)
        return FAILED;

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_acl_entry_get
 * Description:
 *      Get Asic ACL entry
 * Input:
 *      entryadd   - Acl entry address (0~15)
 * Output:
 *      pPhyport    -  Acl physical port 
 *      pAction      -  Acl action 
 *      pProtocol   -  Acl protocol
 *      pData        -  ether type value or TCP/UDP port
 *      pPriority     -  Acl priority
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      phyport could be 
 *            0~5:       port number, 
 *            RTL8306_ACL_INVALID: invalid entry,
 *            RTL8306_ACL_ANYPORT: any port.
 *      Acl action could be
 *          RTL8306_ACT_DROP,
 *          RTL8306_ACT_PERMIT, 
 *          RTL8306_ACT_TRAPCPU, 
 *          RTL8306_ACT_MIRROR
 *      Acl protocol could be
 *          RTL8306_ACL_ETHER(ether type), 
 *          RTL8306_ACL_TCP(TCP), 
 *          RTL8306_ACL_UDP(UDP),
 *          RTL8306_ACL_TCPUDP(TCP or UDP)
 *
 *      Acl priority:
 *          RTL8306_PRIO0~RTL8306_PRIO3
 *       
 */
int32 rtl8306e_acl_entry_get(uint32 entryadd, uint32 *pPhyport, uint32 *pAction, uint32 *pProtocol, uint32  *pData, uint32 *pPriority)
{
    uint32 regValue;
    uint32 pollcnt  ;
    uint32 bitValue;

    if ((entryadd > RTL8306_ACL_ENTRYNUM - 1) || (NULL == pPhyport) || (NULL == pAction) || 
        (NULL == pProtocol) || (NULL == pData) || (NULL == pPriority))
        return FAILED;

    /*trigger a command to read ACL entry*/
    rtl8306e_reg_get(3, 22, 3, &regValue);
    regValue = (regValue & 0x81FF) | (0x3 << 13) | (entryadd << 9);
    rtl8306e_reg_set(3, 22, 3, regValue);
    
    /*Polling whether the command is done*/
    for (pollcnt = 0; pollcnt < 100 ; pollcnt++) 
    {
        rtl8306e_regbit_get(3, 22, 14, 3, &bitValue);
        if (!bitValue)
            break;
    }
    if (pollcnt > 50)
        return FAILED;
    
    rtl8306e_reg_get(3, 21, 3, &regValue);
    *pData = regValue;
    rtl8306e_reg_get(3, 22, 3, &regValue);
    *pPriority = (regValue  >> 7) & 0x3;
    *pAction  = (regValue  >> 5) & 0x3;
    *pPhyport = (regValue >> 2) & 0x7;
    *pProtocol = regValue & 0x3;

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_mib_get
 * Description:
 *      Get the MIB counter for the specified port
 * Input:
 *      port         -  port number (0 ~ 5)
 *      counter    -  Specify counter type  
 * Output:
 *      pValue -  the pointer of counter value 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are five MIB counter for each port, they are:
 *      RTL8306_MIB_CNT1 - TX count
 *      RTL8306_MIB_CNT2 - RX count
 *      RTL8306_MIB_CNT3 - RX Drop Count<nl>
 *      RTL8306_MIB_CNT4 - RX CRC error Count
 *      RTL8306_MIB_CNT5 - RX Fragment Count<nl>
 */
int32 rtl8306e_mib_get(uint32 port, uint32 counter, uint32 *pValue) 
{
    uint32 regValue1, regValue2;

    if ((port > RTL8306_PORT5) || (counter > RTL8306_MIB_CNT5) ||
        (NULL == pValue))
        return FAILED;
    
    /*Port 5 corresponding PHY6*/
    if (RTL8306_PORT5 == port)  
        port ++ ;  
    
    switch(counter) 
    {
        case RTL8306_MIB_CNT1:            
            /*Must read low 16 bit first, then hight 16 bit*/
            rtl8306e_reg_get(port, 22, 2, &regValue1);
            rtl8306e_reg_get(port, 23, 2, &regValue2);
            *pValue = (regValue2 << 16) + regValue1;
            break;
            
        case RTL8306_MIB_CNT2:
            /*Must read low 16 bit first, then hight 16 bit*/
            rtl8306e_reg_get(port, 24, 2, &regValue1);
            rtl8306e_reg_get(port, 25, 2, &regValue2);
            *pValue = (regValue2 << 16) + regValue1;
            break;
            
        case RTL8306_MIB_CNT3:
            /*Must read low 16 bit first, then hight 16 bit*/
            rtl8306e_reg_get(port, 26, 2, &regValue1);
            rtl8306e_reg_get(port, 27, 2, &regValue2);
            *pValue = (regValue2 << 16) + regValue1;            
            break;
            
        case RTL8306_MIB_CNT4:
            /*Must read low 16 bit first, then hight 16 bit*/
            rtl8306e_reg_get(port, 28, 2, &regValue1);
            rtl8306e_reg_get(port, 29, 2, &regValue2);
            *pValue = (regValue2 << 16) + regValue1;            
            break;
            
        case RTL8306_MIB_CNT5:
            /*Must read low 16 bit first, then hight 16 bit*/
            rtl8306e_reg_get(port, 30, 2, &regValue1);
            rtl8306e_reg_get(port, 31, 2, &regValue2);
            *pValue = (regValue2 << 16) + regValue1;            
            break;
        default:
            return FAILED;
    }
    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_mibUnit_set
 * Description:
 *      Set RX/Tx Mib counting unit: byte or packet
 * Input:
 *      port         -  port number (0 ~ 5)
 *      counter    -  Specify counter type  
 *      uint         -  Specify counting unit
 * Output:
 *      none  
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      1.There are five MIB counter for each port, they are:
 *      RTL8306_MIB_CNT1 - TX count
 *      RTL8306_MIB_CNT2 - RX count
 *      RTL8306_MIB_CNT3 - RX Drop Count<nl>
 *      RTL8306_MIB_CNT4 - RX CRC error Count
 *      RTL8306_MIB_CNT5 - RX Fragment Count<nl>
 *      2.Only RTL8306_MIB_CNT1 and RTL8306_MIB_CNT2 could set counting unit  
 *      RTL8306_MIB_PKT or RTL8306_MIB_BYTE, default is RTL8306_MIB_PKT.
 *      the other counters' counting uint is RTL8306_MIB_PKT
 */
int32 rtl8306e_mibUnit_set(uint32 port, uint32 counter, uint32 unit) 
{

    if ((port > RTL8306_PORT5) ||(unit > RTL8306_MIB_PKT))
        return FAILED;
    /*Port 5 corresponding PHY6*/
    if (port == RTL8306_PORT5 )  
        port ++ ;  
    switch(counter) 
    {
        case RTL8306_MIB_CNT1:
            rtl8306e_regbit_set(port, 17, 3, 2, unit);
            break;
        case RTL8306_MIB_CNT2:
            rtl8306e_regbit_set(port, 17, 4, 2, unit);
            break;
        default :
            return FAILED;
    }
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_mibUnit_get
 * Description:
 *      Get Rx/Tx Mib counting unit
 * Input:
 *      port         -  port number (0 ~ 5)
 *      counter    -  Specify counter type  
 * Output:
 *      pUnit         -  the pointer of counting unit
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      1.There are five MIB counter for each port, they are:
 *      RTL8306_MIB_CNT1 - TX count
 *      RTL8306_MIB_CNT2 - RX count
 *      RTL8306_MIB_CNT3 - RX Drop Count<nl>
 *      RTL8306_MIB_CNT4 - RX CRC error Count
 *      RTL8306_MIB_CNT5 - RX Fragment Count<nl>
 *      2.Only RTL8306_MIB_CNT1 and RTL8306_MIB_CNT2 could set counting unit  
 *      RTL8306_MIB_PKT or RTL8306_MIB_BYTE, default is RTL8306_MIB_PKT.
 *      the other counters' counting uint is RTL8306_MIB_PKT
 */
int32 rtl8306e_mibUnit_get(uint32 port, uint32 counter, uint32 *pUnit) 
{ 
    uint32 bitValue;
    if ((port > RTL8306_PORT5) ||(pUnit == NULL))
        return FAILED;
    
    /*Port 5 corresponding PHY6*/
    if (port == RTL8306_PORT5 )  
        port ++ ;  
    switch(counter) 
    {
        case RTL8306_MIB_CNT1:
            rtl8306e_regbit_get(port, 17, 3, 2, &bitValue);
            break;
        case RTL8306_MIB_CNT2:
            rtl8306e_regbit_get(port, 17, 4, 2, &bitValue);
            break;
        default:
            return FAILED;
    }
    *pUnit = (bitValue ? RTL8306_MIB_PKT : RTL8306_MIB_BYTE);
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_mib_reset
 * Description:
 *      reset MIB counter
 * Input:
 *      port         -  port number (0 ~ 5)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */
int32 rtl8306e_mib_reset(uint32 port) 
{

    if ((port > RTL8306_PORT5))
        return FAILED;
    
    /*Port 5 corresponding PHY6*/
    if (RTL8306_PORT5 == port)  
        port ++ ;  

    /*stop counting and clear Mib counter to 0*/
    rtl8306e_regbit_set(port, 17, 2, 2, 1);    
    
    /*Start counting*/
    rtl8306e_regbit_set(port, 17, 2, 2, 0);    
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_mirror_portBased_set
 * Description:
 *      Set asic Mirror port
 * Input:
 *      mirport         -  Specify mirror port 
 *      rxmbr           -  Specify Rx mirror port mask
 *      txmbr           -  Specify Tx mirror port mask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      mirport could be 0 ~ 5, represent physical port number, 
 *      7 means that no port has mirror ability. rxmbr and txmbr
 *      is 6 bit value, each bit corresponds one port. ingress packet
 *      of port in rxmbr will be mirrored to mirport, egress packet 
 *      of port in txmbr will be mirrored to mirport.
 */
int32 rtl8306e_mirror_portBased_set(uint32 mirport, uint32 rxmbr, uint32 txmbr) 
{
    uint32 regValue;

    if ((mirport > 7) ||(rxmbr > 0x3F) || (txmbr > 0x3F) )
        return FAILED;

    /*Set Mirror Port*/
    rtl8306e_reg_get(2, 22, 3, &regValue);
    regValue = (regValue & 0xC7FF) | (mirport << 11);
    rtl8306e_reg_set(2, 22, 3, regValue);
    
    /*enable mirror port to filter the mirrored packet sent from itself */
    rtl8306e_regbit_set(6, 21, 7, 3, 1);
        
    /*Set Ports Whose RX Data are Mirrored */
    rtl8306e_reg_get(6, 21, 3, &regValue);
    regValue = (regValue & 0xFFC0) | rxmbr ;
    rtl8306e_reg_set(6, 21, 3, regValue);
    
    /*Set Ports Whose TX Data are Mirrored */
    rtl8306e_reg_get(6, 21, 3, &regValue);
    regValue = (regValue & 0xC0FF) | (txmbr << 8);
    rtl8306e_reg_set(6, 21, 3, regValue);

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_mirror_portBased_get
 * Description:
 *      Get asic Mirror port
 * Input:
 *      none 
 * Output:
 *      pMirport     -  the pointer of mirror port
 *      pRxmbr       -  the pointer of  Rx mirror port mask
 *      pTxmbr       -  the pointer of Tx mirror port mask 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      mirport could be 0 ~ 5, represent physical port number, 
 *      7 means that no port has mirror ability. rxmbr and txmbr
 *      is 6 bit value, each bit corresponds one port. ingress packet
 *      of port in rxmbr will be mirrored to mirport, egress packet 
 *      of port in txmbr will be mirrored to mirport.
 */
int32 rtl8306e_mirror_portBased_get(uint32 *pMirport, uint32 *pRxmbr, uint32* pTxmbr) 
{
    uint32 regValue;

    if ((NULL == pMirport) ||(NULL == pRxmbr) || (NULL == pTxmbr)) 
        return FAILED;

    /*Get Mirror Port*/
    rtl8306e_reg_get(2, 22, 3, &regValue);
    *pMirport = (regValue & 0x3800) >> 11;
    
    /*Get Ports Whose RX Data are Mirrored*/
    rtl8306e_reg_get(6, 21, 3, &regValue);
    *pRxmbr = regValue & 0x3F;
    
    /*Get Ports Whose TX Data are Mirrored */
    rtl8306e_reg_get(6, 21, 3, &regValue);
    *pTxmbr = (regValue & 0x3F00) >> 8;
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_mirror_macBased_set
 * Description:
 *      Set Mac address for mirror packet
 * Input:
 *      macAddr  - mirrored mac address, it could be SA or DA of the packet 
 *      enabled   - enable mirror packet by mac address
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *
 */
int32 rtl8306e_mirror_macBased_set(uint8 *macAddr, uint32 enabled) 
{

    if (NULL == macAddr)
        return FAILED;

    if (!enabled) 
    {
        rtl8306e_regbit_set(6, 21, 14, 3, 0);
    } 
    else 
    {
        rtl8306e_regbit_set(6, 21, 14, 3, 1);
        rtl8306e_reg_set(6, 22, 3, (macAddr[1] << 8) | macAddr[0]);
        rtl8306e_reg_set(6, 23, 3, (macAddr[3] << 8) | macAddr[2]);
        rtl8306e_reg_set(6, 24, 3, (macAddr[5] << 8) | macAddr[4]);
    }

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_mirror_macBased_set
 * Description:
 *      Set Mac address for mirror packet
 * Input:
 *      none 
 * Output:
 *      macAddr   - mirrored mac address, it could be SA or DA of the packet 
 *      pEnabled   - the pointer of enable mirror packet by mac address 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 * 
 */
int32 rtl8306e_mirror_macBased_get(uint8 *macAddr, uint32 *pEnabled) 
{
    uint32 regValue;
    uint32 bitValue;

    if (NULL == macAddr)
        return FAILED;
    
    rtl8306e_regbit_get(6, 21, 14, 3, &bitValue);
    *pEnabled = (bitValue  ? TRUE : FALSE);
    rtl8306e_reg_get(6, 22, 3, &regValue);
    macAddr[0] = regValue & 0xFF;
    macAddr[1] = (regValue & 0xFF00) >> 8;
    rtl8306e_reg_get(6, 23, 3, &regValue);
    macAddr[2] = regValue & 0xFF;
    macAddr[3] = (regValue & 0xFF00) >> 8;
    rtl8306e_reg_get(6, 24, 3, &regValue);
    macAddr[4] = regValue & 0xFF;
    macAddr[5] = (regValue & 0xFF00) >> 8;
    return SUCCESS;
    
}


/* Function Name:
 *      rtl8306e_l2_MacToIdx_get
 * Description:
 *      get L2 table hash value from mac address
 * Input:
 *      macAddr        -  mac address
 * Output:
 *      pIndex           -  mac address table index   
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      when a mac address is learned into mac address table, 
 *      9 bit index value is got from the mac address by hashing 
 *      algorithm, each index corresponds to 4 entry, it means
 *      the table could save 4 mac addresses at the same time
 *      whose index value is equal, so switch mac address table 
 *      has 2048 entry. the API could get hash index from 
 *      a specified mac address.
 */
int32 rtl8306e_l2_MacToIdx_get(uint8 *macAddr, uint32* pIndex)
{
    uint32 tmp_index;
    uint32 status;

    if ((NULL == macAddr) || (NULL == pIndex))
        return FAILED;


    rtl8306e_regbit_get(1, 23, 12, 0, &status);    
    *pIndex = 0;
    
    if (status) 
    {  
        /* xor hash algorithm*/
        
        /* Index 0 = 4 ^ 11 ^ 18 ^ 25 ^ 32 */
        *pIndex |= ((macAddr[5] & 0x8) >> 3) ^ ((macAddr[4] & 0x10) >> 4) ^ ((macAddr[3] & 0x20) >> 5) ^ ((macAddr[2] & 0x40) >> 6) ^  ((macAddr[1] & 0x80) >> 7); 

        /* Index_1 = 3 ^ 10 ^ 17 ^ 24 ^ 47 */
        tmp_index = ((macAddr[5] & 0x10) >> 4) ^ ((macAddr[4] & 0x20) >> 5) ^ ((macAddr[3] & 0x40) >> 6) ^ ((macAddr[2] & 0x80) >> 7) ^ ((macAddr[0] & 0x1) >> 0);
        *pIndex |= tmp_index << 1;

        /* Index_2 = 2 ^ 9 ^ 16 ^ 39 ^ 46 */
        tmp_index = ((macAddr[5] & 0x20) >> 5) ^ ((macAddr[4] & 0x40) >> 6) ^ ((macAddr[3] & 0x80) >> 7) ^ ((macAddr[1] & 0x1) >> 0) ^ ((macAddr[0] & 0x2) >> 1);
        *pIndex |= tmp_index << 2;

        /* Index_3 = 1 ^ 8 ^ 31 ^ 38 ^ 45 */
        tmp_index = ((macAddr[5] & 0x40) >> 6) ^ ((macAddr[4] & 0x80) >> 7) ^ ((macAddr[2] & 0x1) >> 0) ^ ((macAddr[1] & 0x2) >> 1) ^ ((macAddr[0] & 0x4) >> 2);
        *pIndex |= tmp_index << 3;

        /* Index_4 = 0 ^ 23 ^ 30 ^ 37 ^ 44 */
        tmp_index = ((macAddr[5] & 0x80) >> 7) ^ ((macAddr[3] & 0x1) >> 0) ^ ((macAddr[2] & 0x2) >> 1) ^ ((macAddr[1] & 0x4) >> 2) ^ ((macAddr[0] & 0x8) >> 3);
        *pIndex |= tmp_index << 4;

        /* Index_5 = 15 ^ 22 ^ 29 ^ 36 ^ 43 */
        tmp_index = ((macAddr[4] & 0x1) >> 0) ^ ((macAddr[3] & 0x2) >> 1) ^ ((macAddr[2] & 0x4) >> 2) ^ ((macAddr[1] & 0x8) >> 3) ^ ((macAddr[0] & 0x10) >> 4);
        *pIndex |= tmp_index << 5;

        /* Index_6 = 7 ^  14 ^ 21 ^ 28 ^ 35 ^ 42 */
        tmp_index = ((macAddr[5] & 0x1) >> 0) ^ ((macAddr[4] & 0x2) >> 1) ^ ((macAddr[3] & 0x4) >> 2) ^ ((macAddr[2] & 0x8) >> 3) ^ ((macAddr[1] & 0x10) >> 4) ^ ((macAddr[0] & 0x20) >> 5);
        *pIndex |= tmp_index << 6;

        /* Index_7 = 6 ^ 13 ^ 20 ^ 27 ^ 34 ^ 41 */
        tmp_index = ((macAddr[5] & 0x2) >> 1) ^ ((macAddr[4] & 0x4) >> 2) ^ ((macAddr[3] & 0x8) >> 3) ^ ((macAddr[2] & 0x10) >> 4) ^ ((macAddr[1] & 0x20) >> 5) ^ ((macAddr[0] & 0x40) >> 6);
        *pIndex |= tmp_index << 7;

        /* Index_8 = 5 ^ 12 ^ 19 ^ 26 ^ 33 ^ 40 */
        tmp_index = ((macAddr[5] & 0x4) >> 2) ^ ((macAddr[4] & 0x8) >> 3) ^ ((macAddr[3] & 0x10) >> 4) ^ ((macAddr[2] & 0x20) >> 5) ^ ((macAddr[1] & 0x40) >> 6)  ^ ((macAddr[0] & 0x80) >> 7);
        *pIndex |= tmp_index << 8;
    }
    else  
    {
        /*index direct from mac: LUT index MAC[13:15] + MAC[0:5]*/
        *pIndex = ((macAddr[4] & 0x7) << 6) | ((macAddr[5] & 0xFC) >> 2);
    }

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_l2_IdxToMac_get
 * Description:
 *      reset MIB counter
 * Input:
 *      pIndex        -  the pointer of address table index
 * Output:
 *      macAddr     -   the mac address   
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *   
 */ 
static int32 rtl8306e_l2_IdxToMac_get(uint8 *macAddr, uint32* pIndex)
{
    uint32 tmp_index, regValue, status;

    if ((NULL == macAddr) || (NULL == pIndex))
        return FAILED;

    rtl8306e_regbit_get(1, 23, 12, 0, &status);
    
    if (status) 
    {
        /* xor hash algorithm*/

        /*Read Data[47:32]*/
        rtl8306e_reg_get(4, 18, 0, &regValue);
        macAddr[5] = (regValue & 0x300) >> 8; 
        macAddr[4] = regValue & 0xF8;            
  
        /*Read Data[31:16]*/
        rtl8306e_reg_get(4, 19, 0, &regValue);
        macAddr[3] = (regValue & 0xFF00) >> 8; 
        macAddr[2] = regValue & 0xFF;

        /*Read Data[15:0]*/
        rtl8306e_reg_get(4, 20, 0, &regValue);
        macAddr[1] = (regValue & 0xFF00) >> 8;
        macAddr[0] = regValue & 0xFF;

        /* bit 4 =  Index_0 ^ 11 ^ 18 ^ 25 ^ 32; */
        tmp_index = ((*pIndex & 0x4) >> 2) ^ ((macAddr[4] & 0x10) >> 4) ^ ((macAddr[3] & 0x20) >> 5) ^ ((macAddr[2] & 0x40) >> 6) ^  ((macAddr[1] & 0x80) >> 7); 
        macAddr[5] |= tmp_index << 3;
        /* bit 3 =  Index_1 ^ 10 ^ 17 ^ 24 ^ 47 */
        tmp_index = ((*pIndex & 0x8) >> 3) ^ ((macAddr[4] & 0x20) >> 5) ^ ((macAddr[3] & 0x40) >> 6) ^ ((macAddr[2] & 0x80) >> 7) ^ ((macAddr[0] & 0x1) >> 0);
        macAddr[5] |= tmp_index << 4;
        /* bit 2 =  Index_2 ^ 9 ^ 16 ^ 39 ^ 46 */
        tmp_index = ((*pIndex & 0x10) >> 4) ^ ((macAddr[4] & 0x40) >> 6) ^ ((macAddr[3] & 0x80) >> 7) ^ ((macAddr[1] & 0x1) >> 0) ^ ((macAddr[0] & 0x2) >> 1);
        macAddr[5] |= tmp_index << 5;

        /* bit 1 = Index_3 ^ 8 ^ 31 ^ 38 ^ 45 */
        tmp_index = ((*pIndex & 0x20) >> 5) ^ ((macAddr[4] & 0x80) >> 7)  ^ ((macAddr[2] & 0x1) >> 0) ^ ((macAddr[1] & 0x2) >> 1) ^ ((macAddr[0] & 0x4) >> 2);
        macAddr[5] |= tmp_index << 6;

        /* bit 0 = Index_4 ^ 23 ^ 30 ^ 37 ^ 44 */
        tmp_index = ((*pIndex & 0x40) >> 6) ^ ((macAddr[3] & 0x1) >> 0)  ^ ((macAddr[2] & 0x2) >> 1) ^ ((macAddr[1] & 0x4) >> 2) ^ ((macAddr[0] & 0x8) >> 3);
        macAddr[5] |= tmp_index << 7;

        /* bit 15 = Index_5 ^ 22 ^ 29 ^ 36 ^ 43 */
        tmp_index = ((*pIndex & 0x80) >> 7) ^ ((macAddr[3] & 0x2) >> 1) ^ ((macAddr[2] & 0x4) >> 2) ^ ((macAddr[1] & 0x8) >> 3) ^ ((macAddr[0] & 0x10) >> 4);
        macAddr[4] |= tmp_index << 0;
        
        /* bit 14 =  Index_6 ^ 7 ^ 21 ^ 28 ^ 35 ^ 42 */
        tmp_index = ((*pIndex & 0x100) >> 8) ^ ((macAddr[5] & 0x1) >> 0) ^ ((macAddr[3] & 0x4) >> 2) ^ ((macAddr[2] & 0x8) >> 3) ^ ((macAddr[1] & 0x10) >> 4) ^ ((macAddr[0] & 0x20) >> 5);
        macAddr[4] |= tmp_index << 1;

        /* bit 13 =  Index_7 ^ 6 ^ 20 ^ 27 ^ 34 ^ 41 */
        tmp_index = ((*pIndex & 0x200) >> 9) ^ ((macAddr[5] & 0x2) >> 1) ^ ((macAddr[3] & 0x8) >> 3) ^ ((macAddr[2] & 0x10) >> 4) ^ ((macAddr[1] & 0x20) >> 5) ^ ((macAddr[0] & 0x40) >> 6);
        macAddr[4] |= tmp_index << 2;
        
        /* bit 5 =  Index_8 ^ 12 ^ 19 ^ 26 ^ 33 ^ 40 */
        tmp_index = ((*pIndex & 0x400) >> 10) ^ ((macAddr[4] & 0x8) >> 3) ^ ((macAddr[3] & 0x10) >> 4) ^ ((macAddr[2] & 0x20) >> 5) ^ ((macAddr[1] & 0x40) >> 6)  ^ ((macAddr[0] & 0x80) >> 7);
        macAddr[5] |= tmp_index << 2;
 
    }
    else  
    {
        /*Read Data[47:32]*/
      rtl8306e_reg_get(4, 18, 0, &regValue);
      macAddr[5] = ((regValue & 0x300) >> 8) | (*pIndex & 0xFC);
      macAddr[4] = (regValue & 0xF8) | ((*pIndex >> 8) & 0x7);

      /*Read Data[31:16]*/
      rtl8306e_reg_get(4, 19, 0, &regValue);
      macAddr[3] = (regValue & 0xFF00) >> 8;
      macAddr[2] = regValue & 0xFF;

      /*Read Data[15:0]*/
      rtl8306e_reg_get(4, 20, 0, &regValue);
      macAddr[1] = (regValue & 0xFF00) >> 8;
      macAddr[0] = regValue & 0xFF;
    }
    
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_l2_unicastEntry_set
 * Description:
 *      write an unicast mac address into L2 table
 * Input:
 *      macAddress        -  Specify the unicast Mac address(6 bytes) to be written into LUT
 *      entry                 -  Specify the 4-way entry to be written (0~3)
 *      age                   -  Specify age time
 *      isStatic              -  TRUE(static entry), FALSE(dynamic entry)
 *      isAuth                -  Whether the mac address is authorized by IEEE 802.1x
 *      port                  -   Specify the port number to be forwarded to  
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Age time has 4 value :
 *          RTL8306_LUT_AGEOUT, 
 *          RTL8306_LUT_AGE100(100s)
 *          RTL8306_LUT_AGE200(200s)
 *          RTL8306_LUT_AGE300(300s)
 */ 

int32 rtl8306e_l2_unicastEntry_set(uint8 *macAddress, uint32 entry, uint32 age, uint32 isStatic, uint32 isAuth, uint32 port) 
{
    uint32 regValue, index, pollcnt;
    uint32 bitValue;

    if ((NULL == macAddress) || (entry > RTL8306_LUT_ENTRY3) || (age > RTL8306_LUT_AGE300) ||
        (port > RTL8306_PORT5))
        return FAILED;

    /*For unicast entry, MAC[47] is 0  */
    if (macAddress[0] & 0x1)
        return FAILED;
    
    /*Enable lookup table access*/
    rtl8306e_regbit_set(0, 16, 13, 0, 1);
    
    /*Write Data[55:48]*/
    if (RTL8306_LUT_AGE300 == age) 
        age = 0x2;
    else if (RTL8306_LUT_AGE200 == age)
        age = 0x3;
    else if (RTL8306_LUT_AGE100 == age)
        age = 0x1;
    else 
        age = 0;

    regValue = ((isAuth ? 1:0 ) << 7) | ((isStatic ? 1:0) << 6) | (age << 4) | port;
    rtl8306e_reg_set(4, 17, 0, regValue & 0xFF);
    /*write Data[47:32]*/
    rtl8306e_reg_set(4, 18, 0, macAddress[5] << 8 | macAddress[4]);
    /*wrtie Data[31:16]*/
    rtl8306e_reg_set(4, 19, 0, macAddress[3] << 8 | macAddress[2]);
    /*wrtie Data[15:0]*/
    rtl8306e_reg_set(4, 20, 0, macAddress[1] << 8 | macAddress[0]);

    /*LUT index */
    rtl8306e_l2_MacToIdx_get(macAddress, &index);

    /*Write Command, 2-bit indicating four-way lies in highest bit of Entry_Addr[10:0]*/
    regValue = (entry << 13) | (index << 4) | 0x0;
    rtl8306e_reg_set(4, 16, 0, regValue);
    rtl8306e_regbit_set(4, 16, 1, 0, 1);
    
    /*Waiting for write command done and prevent polling dead loop*/
    for (pollcnt = 0; pollcnt < RTL8306_IDLE_TIMEOUT; pollcnt ++) 
    {
        rtl8306e_regbit_get(4, 16, 1, 0, &bitValue);
        if (!bitValue)
            break;
    }
    if (pollcnt == RTL8306_IDLE_TIMEOUT)
        return FAILED;

    /*Disable lookup table access*/
    rtl8306e_regbit_set(0, 16, 13, 0, 0);
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_l2_unicastEntry_get
 * Description:
 *      read an unicast mac address into L2 table
 * Input:
 *      entry               -  Specify the entry address to be read (0 ~ 2047), not four-way entry
 * Output:
 *      macAddress      -  the mac to be saved in the entry  
 *      pAge               -  the pointer of the age time
 *      pIsStatic         -   the pointer of static or dynamic entry
 *      pIsAuth           -   the pointer of IEEE 802.1x authorized status
 *      pPort              -   the pointer of the port the mac belongs to   
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Age time has 4 value :
 *          RTL8306_LUT_AGEOUT
 *          RTL8306_LUT_AGE100(100s)
 *          RTL8306_LUT_AGE200(200s)
 *          RTL8306_LUT_AGE300(300s)
 */ 
 
int32 rtl8306e_l2_unicastEntry_get(uint8 *macAddress, uint32 entryAddr, uint32 *pAge, uint32 *pIsStatic, uint32 *pIsAuth, uint32 *pPort) 
{
    uint32 entryAddrHd;
    uint32 regValue, pollcnt;
    uint32 bitValue;

    if ((NULL == macAddress) || (entryAddr > 0x7FF) || (NULL == pAge) || (NULL == pIsStatic) ||
        (pIsAuth == NULL) || (pPort == NULL))
        return FAILED;
    
    /*Hardware data format, four-way info is the highest 2 bits of 11-bit entry info*/
    entryAddrHd = (entryAddr >> 2) | ((entryAddr & 0x3) << 9);
    
    /*Enable lookup table access*/
    rtl8306e_regbit_set(0, 16, 13, 0, 1);
    /*Read Command*/
    regValue = (entryAddrHd << 4) | 0x1;
    rtl8306e_reg_set(4, 16, 0, regValue);
    rtl8306e_regbit_set(4, 16, 1, 0, 1);
    
    /*Waiting for Read command done and prevent polling dead loop*/
    for (pollcnt = 0; pollcnt < RTL8306_IDLE_TIMEOUT; pollcnt ++) 
   {
        rtl8306e_regbit_get(4, 16, 1, 0, &bitValue);
        if (!bitValue)
            break;
    }
    if (pollcnt == RTL8306_IDLE_TIMEOUT)
        return FAILED;
    
    /*Read Data[55:48]*/
    rtl8306e_reg_get(4, 17, 0, &regValue);
    *pIsAuth = (regValue & 0x80) ? TRUE: FALSE;
    *pIsStatic = (regValue & 0x40) ? TRUE:FALSE;
    *pAge = (regValue & 0x30) >> 4;
    if (*pAge == 0x2) 
        *pAge = RTL8306_LUT_AGE300;
    else if (*pAge == 0x3)
        *pAge = RTL8306_LUT_AGE200;
    else if (*pAge == 0x1 )
        *pAge = RTL8306_LUT_AGE100;
    else 
        *pAge = RTL8306_LUT_AGEOUT;
    *pPort = regValue & 0x7;

    rtl8306e_l2_IdxToMac_get(macAddress, &entryAddr);

    /*Disable lookup table access*/
    rtl8306e_regbit_set(0, 16, 13, 0, 0);
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_l2_multicastEntry_set
 * Description:
 *      write an multicast mac address into L2 table
 * Input:
 *      macAddress      -  the mac to be saved in the entry  
 *      entry               -  Specify the 4-way entry to be written (0~3)
 *      isAuth              -  IEEE 802.1x authorized status
 *      portMask          -  switch received thepacket with the specified macAddress, 
 *                                and forward it to the member port of portMask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      get 9-bit as index value of lookup table by hashing the mac address , for each index value, 
 *      there are 4-entry to save hash collision mac address, thus there are 2048 entries all together.
 *      the entry could save both unicast multicast mac address.  multicast entry has no age time and
 *      static bit, which has been set by software, unicast entry could be both auto learned and set by 
 *      software. you had better set isAuth TRUE for general application because  IEEE 802.1x is usually
 *      for unicast packet.portMask is 6-bit value, each bit represents one port, bit 0 corresponds to port 0
 *      and bit 5 corresponds port 5.
 */ 

int32 rtl8306e_l2_multicastEntry_set(uint8 *macAddress, uint32 entry, uint32 isAuth, uint32 portMask) 
{
    uint32 regValue, index, pollcnt;
    uint32 bitValue;

    if ((NULL == macAddress) || (entry > RTL8306_LUT_ENTRY3) || (portMask > 0x3F ))
        return FAILED;

    /*For Muticast entry, MAC[47] is 1  */
    if (!(macAddress[0] & 0x1))
        return FAILED;
    
    /*Enalbe Lookup table access*/
    rtl8306e_regbit_set(0, 16, 13, 0, 1);
    
    /*Write Data[55:48]*/
    /*Multicast entry portmask bits is Data[54:52], Data[50:48]*/
    regValue = ((isAuth ? 1: 0) << 7) | (portMask & 0x38) << 1 | (portMask & 0x7); 
    rtl8306e_reg_set(4, 17, 0, regValue);
    /*Write Data[47:32]*/
    rtl8306e_reg_set(4, 18, 0, (macAddress[5] << 8) |macAddress[4]);
    /*Write Data[31:16]*/
    rtl8306e_reg_set(4, 19, 0, (macAddress[3] << 8) |macAddress[2]);
    /*Write Data[15:0]*/
    rtl8306e_reg_set(4, 20, 0, (macAddress[1] << 8) |macAddress[0]);

    /*LUT index */
    rtl8306e_l2_MacToIdx_get(macAddress, &index);

    /*Write Command, 2-bit indicating four-way lies in highest bit of Entry_Addr[10:0]*/
    regValue = (entry << 13) | (index << 4) | 0x2;
    rtl8306e_reg_set(4, 16, 0, regValue);
    
    /*Waiting for write command done and prevent polling dead loop*/
    for (pollcnt = 0; pollcnt < RTL8306_IDLE_TIMEOUT; pollcnt ++)
   {
        rtl8306e_regbit_get(4, 16, 1, 0, &bitValue);
        if (!bitValue)
        break;
    }
    if (pollcnt == RTL8306_IDLE_TIMEOUT)
        return FAILED;
    
    /*Disable Lookup table access*/
    rtl8306e_regbit_set(0, 16, 13, 0, 0);
    return SUCCESS;
}

/* Function Name:
 *      rtl8306_getAsicLUTMulticastEntry
 * Description:
 *      Get LUT multicast entry
 * Input:
 *      entryAddr         -  Specify the LUT entry address(0~2047)

 * Output:
 *      macAddress      -  The read out multicast Mac address  
 *      pIsAuth            -  the pointer of IEEE 802.1x authorized status
 *      portMask          -  port mask 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Get multicast entry
 */ 

int32 rtl8306e_l2_multicastEntry_get(uint8 *macAddress, uint32 entryAddr, uint32 *pIsAuth, uint32 *pPortMask) 
{
    uint32 entryAddrHd;
    uint32 regValue, pollcnt;
    uint32 bitValue;

    if ((NULL == macAddress) || (entryAddr > 0x7FF) ||(NULL == pIsAuth) || (NULL == pPortMask))
        return FAILED;

    /*Hardware data format, four-way info is the highest 2 bits of 11-bit entry info*/
    entryAddrHd = (entryAddr >> 2) | ((entryAddr & 0x3) << 9);
    
    /*Enalbe Lookup table access*/
    rtl8306e_regbit_set(0, 16, 13, 0, 1);
    
    /*Write Command*/
    regValue = (entryAddrHd << 4) | 0x3;
    rtl8306e_reg_set(4, 16, 0, regValue);
    
    /*Waiting for Read command done and prevent polling dead loop*/
    for (pollcnt = 0; pollcnt < RTL8306_IDLE_TIMEOUT; pollcnt ++) 
   {
        rtl8306e_regbit_get(4, 16, 1, 0, &bitValue);
        if (!bitValue)
            break;
    }
    if (pollcnt == RTL8306_IDLE_TIMEOUT)
        return FAILED;
    
    /*Read Data[55:48]*/
    rtl8306e_reg_get(4, 17, 0, &regValue);
    *pIsAuth = (regValue & 0x80 ? 1:0);
    
    /*Multicast entry portmask bits is Data[54:52], Data[50:48]*/
    *pPortMask = ((regValue & 0x70) >> 4) << 3 | (regValue & 0x7);

    rtl8306e_l2_IdxToMac_get(macAddress, &entryAddr);

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_l2_unicastMac_add
 * Description:
 *     Add an unicast mac address, software will detect empty entry
 * Input:
 *      macAddress      -  the mac to be saved in the entry  
 *      age                 -  Specify age time
 *      isStatic            -  TRUE(static entry), FALSE(dynamic entry)
 *      isAuth              -  IEEE 802.1x authorized status
 *      port                 -  the port which the mac address belongs to  
 * Output:
 *      pEntryaddr        -   the entry address (0 ~2047) which the unicast mac address is written into
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Age time has 4 value :RTL8306_LUT_AGEOUT, RTL8306_LUT_AGE100(100s), 
 *      RTL8306_LUT_AGE200(200s), RTL8306_LUT_AGE300(300s)
 *      The lut has a 4-way entry of an index. If the macAddress has existed in the lut, it will update the entry,
 *      otherwise the function will find an empty entry to put it.
 *      When the index is full, it will find a dynamic & unauth unicast macAddress entry to replace with it. 
 *      If the mac address has been written into LUT, function return value is SUCCESS,  *pEntryaddr is recorded the 
 *      entry address of the Mac address stored.
 *      If all the four entries can not be replaced, it will return a  RTL8306_LUT_FULL error, you can delete one of them 
 *      and rewrite the unicast address.  
 */ 

int32 rtl8306e_l2_unicastMac_add(uint8 *macAddress, uint32 age, uint32 isStatic, uint32 isAuth, uint32 port, uint32 *pEntryaddr)
{
    int32 i;
    uint8 macAddr[6];
    uint32 index,entryaddr;
    uint32 isStatic1,isAuth1,age1,port1;

    /*check for macAddress. must be unicast address*/
    if( (NULL == macAddr) || (macAddr[0] & 0x1))
        return FAILED;

    /*check port*/
    if (port > 5)
        return FAILED;

    rtl8306e_l2_MacToIdx_get(macAddress, &index);

    /*
      *First scan four-ways, if the unicast entry has existed, only update the entry, that could 
      *prevent two same Mac in four-ways; if the mac was not written into entry before, then scan 
      *four-ways again, to Find an dynamic & unauthorized unicast entry which is auto learned, then  
      *replace it with the multicast Mac addr. scanning sequence is from entry 3 to entry 0, because 
      *priority of four way is entry 3 > entry 2 > entry 1 > entry 0
      */
    for (i = 3; i >= 0; i--) 
    {
        entryaddr = (index << 2) | i;
        if (rtl8306e_l2_unicastEntry_get(macAddr, entryaddr, &age1, &isStatic1, &isAuth1, &port1) != SUCCESS) 
        {
            return FAILED;
        }
        else if ((macAddress[0] == macAddr[0]) && (macAddress[1] == macAddr[1]) && 
                   (macAddress[2] == macAddr[2]) && (macAddress[3] == macAddr[3]) &&
                    (macAddress[4] == macAddr[4]) && (macAddress[5] == macAddr[5])) 
        {
            rtl8306e_l2_unicastEntry_set(macAddress, i , age, isStatic, isAuth, port);
            *pEntryaddr = entryaddr;
            return SUCCESS;
        }
    }
    
    for (i = 3; i >= 0; i--) 
    {
        entryaddr = (index << 2) | i ;
        if (rtl8306e_l2_unicastEntry_get(macAddr, entryaddr, &age1, &isStatic1, &isAuth1, &port1) != SUCCESS) 
        {
            return FAILED;
        }
        else if (((macAddr[0] & 0x1) == 0) && (!isStatic1) && (!isAuth1))  
        {
            rtl8306e_l2_unicastEntry_set(macAddress, i , age, isStatic, isAuth, port);
            *pEntryaddr = entryaddr;
            return SUCCESS;
        }
    }

    /* four way are all full, return RTL8306_LUT_FULL*/
    return RTL8306_LUT_FULL;
}


/* Function Name:
 *      rtl8306e_l2_multicastMac_add
 * Description:
 *     Add an multicast mac address, software will detect empty entry
 * Input:
 *      macAddress      -  the mac to be saved in the entry  
 *      isAuth              -  IEEE 802.1x authorized status
 *      portMask          -  switch received the packet with the specified macAddress, 
 *                                and forward it to the member port of portMask
 * Output:
 *      pEntryaddr        -   the entry address (0 ~2047) which the multicast mac address is written into
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Add an multicast entry, it need not specify the 4-way like function rtl8306e_l2_multicastEntry_set,
 *      if the mac address has written into LUT, function return value is SUCCESS,  *pEntryaddr is recorded the 
 *      entry address of the Mac address stored, if  4-way entries are all written by cpu, this mac address could 
 *      not written into LUT and the function return value is  RTL8306_LUT_FULL, but if the Mac address has exist, 
 *      the port mask will be updated. When function return value is RTL8306_LUT_FULL, you can delete one of them 
 *      and rewrite the multicast address. 
 */ 
int32  rtl8306e_l2_multicastMac_add(uint8 *macAddr,uint32 isAuth, uint32 portMask, uint32 *pEntryaddr) 
{
    int32  i;
    uint8 macAddress[4][6];
    uint32 index, entryAddress[4], age[4];
    uint32 isStatic[4], isAuthority[4];
    uint32 port[4];
    uint32 isFull = TRUE;


    if ((NULL == macAddr) || (!(macAddr[0] & 0x1)) || (NULL == pEntryaddr))
        return FAILED;

    rtl8306e_l2_MacToIdx_get(macAddr, &index);

    /*
      *First scan four-ways, if the multicast entry has existed, only update the entry, that could 
      *prevent two same Mac in four-ways; if the mac was not written into entry before, then scan 
      *four-ways again, to Find an dynamic & unauthorized unicast entry which is auto learned, then  
      *replace it with the multicast Mac addr. scanning sequence is from entry 3 to entry 0, because priority
      *of four way is entry 3 > entry 2 > entry 1 > entry 0
      */
    for (i = 3; i >= 0; i--) 
    {
        entryAddress[i] = (index << 2) | i;
        if (rtl8306e_l2_unicastEntry_get(macAddress[i], entryAddress[i], &age[i], &isStatic[i], &isAuthority[i], &port[i]) != SUCCESS) 
        {
            return FAILED;
        }
        else if ((macAddr[0] == macAddress[i][0]) && (macAddr[1] == macAddress[i][1]) && 
                    (macAddr[2] == macAddress[i][2]) && (macAddr[3] == macAddress[i][3]) &&
                    (macAddr[4] == macAddress[i][4]) && (macAddr[5] == macAddress[i][5])) 
        {
            rtl8306e_l2_multicastEntry_set(macAddr, i, isAuth, portMask);
            *pEntryaddr = entryAddress[i];
            isFull = FALSE;
            return SUCCESS;
        }
    }
    
    for (i = 3; i >= 0; i--) 
    {
        if (((macAddress[i][0] & 0x1) == 0) && (!isStatic[i]) && (!isAuthority[i])) 
        {
            rtl8306e_l2_multicastEntry_set(macAddr, i , isAuth, portMask);
            *pEntryaddr = entryAddress[i];
            isFull = FALSE;
            break;
        }
    }

    /*If four way are all full, return RTL8306_LUT_FULL*/
    if (isFull) 
    {
        *pEntryaddr = 10000;
        return RTL8306_LUT_FULL;
    }
    return SUCCESS;

}

/* Function Name:
 *      rtl8306e_l2_mac_get
 * Description:
 *      Get an mac address information
 * Input:
 *      macAddress         -   the mac to be find in LUT  
 * Output:
 *      pIsStatic             -   the pointer of static or dynamic entry, for unicast mac address
 *      pIsAuth               -   the pointer of IEEE 802.1x authorized status
 *      pPortInfo             -   for unicast mac, it is the pointer of the port the mac belongs to;
 *                                    for multicast mac, it is the pointer of portmask the mac forwarded to;
 *      pEntryaddr           -   the entry address (0 ~2047) which the mac address is written into
 * Return:
 *      SUCCESS
 *      FAILED
 *      RTL8306_LUT_NOTEXIST
 * Note:
 *      use this API to get a mac address information in LUT.
 */ 
int32 rtl8306e_l2_mac_get(uint8 *macAddr, uint32 *pIsStatic, uint32 *pIsAuth, uint32 *pPortInfo, uint32 *pEntryaddr) 
{
    int32  i;
    uint8 macAddress[6];
    uint32 index, entryAddress, age;
    uint32 isStatic, isAuth;
    uint32 port, portmask;
    uint32 isHit ;

    if((NULL == macAddr) || (NULL == pIsAuth) || (NULL == pPortInfo) || (NULL == pEntryaddr))
        return FAILED;
    
    isHit = FALSE;
    
    rtl8306e_l2_MacToIdx_get(macAddr, &index);

    /*scanning sequence is from entry 3 to entry 0, because priority
      *of four way is entry 3 > entry 2 > entry 1 > entry 0
      */
    for (i = 3; i >= 0; i--) 
    {
        entryAddress = (index << 2) | i;
        if (rtl8306e_l2_unicastEntry_get(macAddress, entryAddress, &age, &isStatic, &isAuth, &port) != SUCCESS) 
        {
            return FAILED;
        }
        else if ((macAddr[0] == macAddress[0]) && (macAddr[1] == macAddress[1]) && 
                    (macAddr[2] == macAddress[2]) && (macAddr[3] == macAddress[3]) &&
                    (macAddr[4] == macAddress[4]) && (macAddr[5] == macAddress[5])) 
        {
            if(macAddr[0] & 0x1)
            {
                rtl8306e_l2_multicastEntry_get(macAddress, entryAddress, &isAuth, &portmask);
                *pIsStatic = TRUE;
                *pIsAuth = isAuth;
                *pPortInfo = portmask;
            }
            else 
            {
                *pIsStatic = isStatic;
                *pIsAuth = isAuth;
                *pPortInfo = port;
                if ((RTL8306_LUT_AGEOUT == age) && !isStatic && !isAuth)
                    return  RTL8306_LUT_NOTEXIST;                                                                  
            }

            *pEntryaddr = entryAddress;
            isHit = TRUE;
            return SUCCESS;
        }
    }

    if(!isHit)
        return RTL8306_LUT_NOTEXIST;

    return SUCCESS;
}
    

/* Function Name:
 *      rtl8306e_l2_mac_del
 * Description:
 *     Delete the specified Mac address, could be both unicast and multicast 
 * Input:
 *      macAddress      -  the Mac address(unicast or multicast) to be delete  
 *                                and forward it to the member port of portMask
 * Output:
 *      pEntryaddr        -  entry address from which the Mac address is deleted
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function to delete a Mac address, it does not require to specify the 
 *      entry address, if the Mac has existed in the LUT, it will be deleted and function
 *      return value is SUCCESS, *pEntryaddr is recorded the entry address of the Mac 
 *      address stored. if the Mac does not existed in the LUT, function return value is
 *      RTL8306_LUT_NOTEXIST, and *pEntryaddr equals 10000.
 */ 
int32 rtl8306e_l2_mac_del(uint8 *macAddr, uint32 *pEntryaddr)
{
    uint32  i, port, portmask;
    uint8 macAddr1[6];
    uint32 index, entryaddr1, age;
    uint32 isStatic, isAuth;
    uint32 isHit = FALSE;
    uint32 hashStatus;
    
    if ((NULL == macAddr) || (NULL == pEntryaddr))
        return FAILED;

    rtl8306e_l2_MacToIdx_get(macAddr, &index);
    
    if (!(macAddr[0] & 0x1)) 
    {  
        /*Delete an unicast entry*/
        for (i = 4; i >= 1; i --) 
        {
            entryaddr1 = (index << 2) | (i - 1);
            if(rtl8306e_l2_unicastEntry_get(macAddr1, entryaddr1, &age, &isStatic, &isAuth, &port) != SUCCESS) 
                return FAILED;
            else if (macAddr[0] ==macAddr1[0] && macAddr[1] ==macAddr1[1] 
                       && macAddr[2] ==macAddr1[2] && macAddr[3] ==macAddr1[3]
                       && macAddr[4] ==macAddr1[4] && macAddr[5] ==macAddr1[5]) 
            {  
                rtl8306e_l2_unicastEntry_set(macAddr, i -1, 0, FALSE, FALSE, port);
                *pEntryaddr = entryaddr1;
                isHit = TRUE;
            } 
        }
    }
    else 
    {
        /*Delet an multicast entry*/
        for (i = 4; i >= 1; i --)
        {
            entryaddr1 = (index << 2) | (i - 1);
            if(rtl8306e_l2_multicastEntry_get(macAddr1, entryaddr1, &isAuth, &portmask) != SUCCESS)
                return FAILED;
            else if (macAddr[0] ==macAddr1[0] && macAddr[1] ==macAddr1[1] 
                       && macAddr[2] ==macAddr1[2] && macAddr[3] ==macAddr1[3]
                       && macAddr[4] ==macAddr1[4] && macAddr[5] ==macAddr1[5]) 
            {  
               /*
                 * Turn multicast address to unicast address. If new hash is used 
                 * bit 24 will be performed not operation to make the Index unchange
                 */
                rtl8306e_regbit_get(1, 23, 12, 0, &hashStatus);            

                if (hashStatus) 
                {
                    macAddr1[0] = macAddr1[0] & 0xFE; 
                    macAddr1[2] ^= 0x80;
                } 
                else 
                {
                    macAddr1[0] = macAddr1[0] & 0xFE; 
                }
                
                if (rtl8306e_l2_unicastEntry_set(macAddr1, i -1, 0, FALSE, FALSE, 0) != SUCCESS)
                    return FAILED;
                *pEntryaddr = entryaddr1;
                isHit = TRUE;
            } 
        }

    }

    if (!isHit) 
    {
        *pEntryaddr = 10000;
        return RTL8306_LUT_NOTEXIST;
    }
    else 
        return SUCCESS;

}

/* Function Name:
 *      rtl8306e_l2_portMacLimit_set
 * Description:
 *      Set per port mac limit ability and auto learning limit number
 * Input:
 *      port         -  port number (0 ~ 5)  
 *      enabled    - TRUE of FALSE  
 *      macCnt    - auto learning MAC limit number
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */ 
int32 rtl8306e_l2_portMacLimit_set(uint32 port, uint32 enabled, uint32 macCnt) 
{
    uint32 regValue;
    //shlee, unused variable    uint32 bitValue;

    if (port > RTL8306_PORT5)
        return FAILED;
    if (5 == port)
        port ++;

    if (0 == port)
    {
        rtl8306e_regbit_set(0, 26, 12, 3, enabled ? 1 : 0);
        rtl8306e_reg_get(0, 27, 3, &regValue);
        regValue &= ~(0x1F << 11);
        regValue |= ((macCnt & 0x1F) << 11);
        rtl8306e_reg_set(0, 27, 3, regValue);
    }
    else
    {
        rtl8306e_regbit_set(port, 30, 15, 1, enabled ? 1 : 0);
        rtl8306e_reg_get(port, 31, 1, &regValue);
        regValue &= ~(0x1F << 11);
        regValue |= ((macCnt & 0x1F) << 11);
        rtl8306e_reg_set(port, 31, 1, regValue);
    }
 
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_l2_portMacLimit_get
 * Description:
 *      Get per port mac limit ability and auto learning limit number
 * Input:
 *      port         -  port number (0 ~ 5)  
 * Output:
 *      pEnabled    -  enabled or disabled the port mac limit
 *      pMacCnt     -  auto learning MAC limit number
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */ 
int32 rtl8306e_l2_portMacLimit_get(uint32 port, uint32 *pEnabled, uint32 *pMacCnt) 
{
    uint32 regValue;
    uint32 bitValue;

    if (port > RTL8306_PORT5 || NULL == pEnabled || NULL == pMacCnt)
        return FAILED;
    if (5 == port)
        port ++;

    if (0 == port)
    {
        rtl8306e_regbit_get(0, 26, 12, 3, &bitValue);
        *pEnabled = bitValue ? TRUE : FALSE;
        rtl8306e_reg_get(0, 27, 3, &regValue);
        *pMacCnt = (regValue & (0x1F << 11)) >> 11;
    }
    else
    {
        rtl8306e_regbit_get(port, 30, 15, 1, &bitValue);
        *pEnabled = bitValue ? TRUE : FALSE;
        rtl8306e_reg_get(port, 31, 1, &regValue);
        *pMacCnt = (regValue & (0x1F << 11)) >> 11;
    }
 
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_l2_systemMacLimit_set
 * Description:
 *      Set the system mac limit ability and auto learning limit number
 * Input:
 *      enabled    - TRUE of FALSE  
 *      macCnt    - system auto learning MAC limit number
 *      mergMask  -  port mask for the ports merged to system mac limit
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Besides per port mac limit function, 8306E also supports system MAC limit function.
 *      mergMask is to decide whitch ports are limited by system MAC limit function.
 *      For example, when system mac limit is enabled, and mergMask is 0x15(0b010101),
 *      that means the auto learning MAC number of port 0, port 2 and port 4 will also be
 *      influenced by system MAC limit.    
 */ 
int32 rtl8306e_l2_systemMacLimit_set(uint32 enabled, uint32 macCnt, uint32 mergMask) 
{
    uint32 regValue;
    //shlee, unused variable    uint32 bitValue;

    rtl8306e_regbit_set(0, 26, 11, 3, enabled ? 1 : 0);
    
    rtl8306e_reg_get(0, 31, 0, &regValue);
    regValue &= ~(0x7F);
    regValue |= (macCnt & 0x7F) ;
    rtl8306e_reg_set(0, 31, 0, regValue);
    
    rtl8306e_reg_get(0, 31, 0, &regValue);
    regValue &= ~(0x3F << 10);
    regValue |= ((mergMask & 0x3F) << 10);
    rtl8306e_reg_set(0, 31, 0, regValue);
 
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_l2_systemMacLimit_get
 * Description:
 *      Get the system mac limit ability and auto learning limit number
 * Input:
 *      none
 * Output:
 *      pEnabled    -  enabled or disabled the system mac limit
 *      pMacCnt    -   system auto learning MAC limit number
 *      pMergMask  -  port mask for the ports merged to system mac limit
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */ 
int32 rtl8306e_l2_systemMacLimit_get(uint32 *pEnabled, uint32 *pMacCnt, uint32 *pMergMask) 
{
    uint32 regValue;
    uint32 bitValue;

    rtl8306e_regbit_get(0, 26, 11, 3, &bitValue);
    *pEnabled = bitValue ? TRUE : FALSE;
    
    rtl8306e_reg_get(0, 31, 0, &regValue);
    *pMacCnt = (regValue & 0x7F);
    
    rtl8306e_reg_get(0, 31, 0, &regValue);
    *pMergMask = (regValue & (0x3F << 10)) >> 10;
    
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_l2_macLimitAction_set
 * Description:
 *      Set the action taken by switch when auto learning MAC reach to the limit number
 * Input:
 *      action      -  the action taken when auto learning MAC reach to the max value 
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */ 
int32 rtl8306e_l2_macLimitAction_set(uint32 action) 
{
    if (RTL8306_ACT_DROP == action)
        rtl8306e_regbit_set(0, 31, 9, 0, 0);
    else if (RTL8306_ACT_TRAPCPU == action)
        rtl8306e_regbit_set(0, 31, 9, 0, 1);
    else
        return FAILED;

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_l2_macLimitAction_get
 * Description:
 *      Get the action taken by switch when auto learning MAC reach to the limit number
 * Input:
 *      pAction      -  the action taken when auto learning MAC reach to the max value
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */ 
int32 rtl8306e_l2_macLimitAction_get(uint32 *pAction) 
{
    uint32 bitValue;

    if (NULL == pAction)
        return FAILED;
    
    rtl8306e_regbit_get(0, 31, 9, 0, &bitValue);
    if (bitValue)
        *pAction = RTL8306_ACT_TRAPCPU;
    else
        *pAction = RTL8306_ACT_DROP;

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_stp_set
 * Description:
 *      Set IEEE 802.1d port state
 * Input:
 *      port   -  Specify port number (0 ~ 5)
 *      state -   Specify port state
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 4 port state:
 *         RTL8306_SPAN_DISABLE   - Disable state
 *         RTL8306_SPAN_BLOCK      - Blocking state
 *         RTL8306_SPAN_LEARN      - Learning state
 *         RTL8306_SPAN_FORWARD - Forwarding state
 */ 
int32 rtl8306e_stp_set(uint32 port, uint32 state) 
{
    uint32 regValue;

    if ((port > RTL8306_PORT5) || (state > RTL8306_SPAN_FORWARD))
        return FAILED;

    /*Enable BPDU to trap to cpu, BPDU could not be flooded to all port*/
    rtl8306e_regbit_set(2, 21, 6, 3, 1);
    rtl8306e_reg_get(4, 21, 3, &regValue);
    regValue = (regValue & ~(0x3 << (2*port))) | (state << (2*port));
    rtl8306e_reg_set(4, 21, 3, regValue);

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_stp_get
 * Description:
 *      Get IEEE 802.1d port state
 * Input:
 *      port    -  Specify port number (0 ~ 5)
 * Output:
 *      pState -  get port state
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 4 port state:
 *         RTL8306_SPAN_DISABLE   - Disable state
 *         RTL8306_SPAN_BLOCK      - Blocking state
 *         RTL8306_SPAN_LEARN      - Learning state
 *         RTL8306_SPAN_FORWARD - Forwarding state
 */ 
int32 rtl8306e_stp_get(uint32 port, uint32 *pState) 
{
    uint32 regValue;

    if ((port > RTL8306_PORT5) || (NULL == pState))
        return FAILED;
    
    rtl8306e_reg_get(4, 21, 3, &regValue);
    *pState = (regValue & (0x3 << 2*port)) >> (2*port);
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_dot1x_portBased_set
 * Description:
 *      Set IEEE802.1x port-based access control
 * Input:
 *      port         -  Specify port number (0 ~ 5)
 *      enabled    -   enable port-based access control
 *      isAuth      -   Authorized or unauthorized state 
 *      direction   -    set IEEE802.1x port-based control direction
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are two IEEE802.1x port state:
 *         RTL8306_PORT_AUTH      - authorized
 *         RTL8306_PORT_UNAUTH  - unauthorized
 *
 *     There are also two 802.1x port-based control direction:
 *         RTL8306_PORT_BOTHDIR - if port-base access control is enabled, 
 *                                              forbid forwarding this port's traffic to unauthorized port
 *         RTL8306_PORT_INDIR     - if port-base access control is enabled, permit forwarding this
 *                                              port's traffic to unauthorized port
 */ 
int32 rtl8306e_dot1x_portBased_set(uint32 port, uint32 enabled, uint32 isAuth, uint32 direction)
{

    if (port > RTL8306_PORT5)
        return FAILED;
    
    /*Port 5 corresponding PHY6*/
    if (RTL8306_PORT5 == port)  
        port ++ ;  
    
    if (!enabled) 
    {
        rtl8306e_regbit_set(port, 17, 8, 2, 0);
        rtl8306e_regbit_set(port, 17, 6, 2, isAuth == RTL8306_PORT_AUTH ? 1:0);
        rtl8306e_regbit_set(port, 17, 7, 2, direction == RTL8306_PORT_BOTHDIR ? 0:1);
    } 
    else 
    {
        rtl8306e_regbit_set(port, 17, 8, 2, 1);
        rtl8306e_regbit_set(port, 17, 6, 2, isAuth == RTL8306_PORT_AUTH ? 1:0);
        rtl8306e_regbit_set(port, 17, 7, 2, direction == RTL8306_PORT_BOTHDIR ? 0:1);
    }

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_dot1x_portBased_set
 * Description:
 *      Set IEEE802.1x port-based access control
 * Input:
 *      port         -  Specify port number (0 ~ 5)
 * Output:
 *      pEnabled    - the pointer of port-based access control status
 *      pIsAuth      - the pointer of authorized or unauthorized state 
 *      pDirection   - the pointer of IEEE802.1x port-based control direction
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are two IEEE802.1x port state:
 *         RTL8306_PORT_AUTH      - authorized
 *         RTL8306_PORT_UNAUTH  - unauthorized
 *
 *     There are also two 802.1x port-based control direction:
 *         RTL8306_PORT_BOTHDIR - if port-base access control is enabled, 
 *                                              forbid forwarding this port's traffic to unauthorized port
 *         RTL8306_PORT_INDIR     - if port-base access control is enabled, permit forwarding this
 *                                              port's traffic to unauthorized port
 */ 

int32 rtl8306e_dot1x_portBased_get(uint32 port, uint32 *pEnabled, uint32 *pIsAuth, uint32 *pDirection) 
{
    uint32 bitValue;

    if ((port > RTL8306_PORT5) || (NULL == pEnabled) || (NULL == pIsAuth) || (NULL == pDirection)) 
        return FAILED;
    
    /*Port 5 corresponding PHY6*/
    if (port == RTL8306_PORT5 )  
        port ++ ;  
    
    rtl8306e_regbit_get(port, 17, 8, 2, &bitValue);
    *pEnabled = (bitValue ? TRUE : FALSE);
    rtl8306e_regbit_get(port, 17, 6, 2, &bitValue);
    *pIsAuth = (bitValue ? RTL8306_PORT_AUTH : RTL8306_PORT_UNAUTH);
    rtl8306e_regbit_get(port, 17, 7, 2, &bitValue);
    *pDirection = (bitValue ? RTL8306_PORT_INDIR : RTL8306_PORT_BOTHDIR);
    return SUCCESS;
} 

/* Function Name:
 *      rtl8306e_dot1x_macBased_set
 * Description:
 *      Set IEEE802.1x port-based access control
 * Input:
 *      port         -  Specify port number (0 ~ 5)
 * Output:
 *      enabled    - Enable the port Mac-based access control ability
 *      direction   -  IEEE802.1x mac-based access control direction
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      the mac address authentication status is saved in L2 table entry,
 *      it should be set by software.
 *      there are also two mac-based control directions which are not per 
 *      port but global configurtion:
 *      RTL8306_MAC_BOTHDIR - if Mac-based access control is enabled, packet with 
 *      unauthorized DA will be dropped.
 *      RTL8306_MAC_INDIR   - if Mac-based access control is enabled, packet with 
 *      unauthorized DA will pass mac-based access control igress rule.
 */ 
int32 rtl8306e_dot1x_macBased_set(uint32 port, uint32 enabled, uint32 direction)
{

    if (port > RTL8306_PORT5)
        return FAILED;
    
    /*Port 5 corresponding PHY6*/
    if (RTL8306_PORT5 == port)  
        port ++ ;  
    
    if (!enabled) 
    {
        rtl8306e_regbit_set(port, 17, 9, 2, 0);
        return SUCCESS;
    } 
    else 
    {
        rtl8306e_regbit_set(port, 17, 9, 2, 1);
        rtl8306e_regbit_set(2, 22, 7, 3, direction == RTL8306_MAC_BOTHDIR ?  0 : 1 );
    }
    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_dot1x_macBased_set
 * Description:
 *      Set IEEE802.1x port-based access control
 * Input:
 *      port         -  Specify port number (0 ~ 5)
 * Output:
 *      enabled    - Enable the port Mac-based access control ability
 *      direction   -  IEEE802.1x mac-based access control direction
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      the mac address authentication status is saved in L2 table entry,
 *      it should be set by software.
 *      there are also two mac-based control directions which are not per 
 *      port but global configurtion:
 *      RTL8306_MAC_BOTHDIR - if Mac-based access control is enabled, packet with 
 *      unauthorized DA will be dropped.
 *      RTL8306_MAC_INDIR   - if Mac-based access control is enabled, packet with 
 *      unauthorized DA will pass mac-based access control igress rule.
 */ 
 
int32 rtl8306e_dot1x_macBased_get(uint32 port, uint32 *pEnabled, uint32 *pDirection) 
{
    uint32 bitValue;

    if ((port > RTL8306_PORT5) || (NULL == pEnabled) || (NULL == pDirection))
        return FAILED;
    
    if (port  < RTL8306_PORT5 ) 
        rtl8306e_regbit_get(port, 17, 9, 2, &bitValue);
    else 
        rtl8306e_regbit_get(6, 17, 9, 2, &bitValue);
    *pEnabled = (bitValue ? TRUE : FALSE) ; 
    rtl8306e_regbit_get(2, 22, 7, 3, &bitValue);
    *pDirection = (bitValue ? RTL8306_PORT_INDIR : RTL8306_PORT_BOTHDIR);
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_trap_igmpCtrlPktAction_set
 * Description:
 *      Set IGMP/MLD trap function
 * Input:
 *      type         -  Specify IGMP/MLD or PPPOE
 *      action       -  Action could be normal forward or trap
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     type could be:
 *          RTL8306_IGMP   - igmp packet without pppoe header 
 *          RTL8306_MLD    - mld packet  without pppoe header
 *          RTL8306_PPPOE - pppoe packet if enable trap RTL8306_IGMP 
 *                                   or RTL8306_MLD whether pppoe packet 
 *                                   should be trapped. In some application,
 *                                   igmp and mld message is encapsulated in pppoed
 *                                   packet.
 *         
 *      action could be:
 *          RTL8306_ACT_PERMIT    - normal forward
 *          RTL8306_ACT_TRAPCPU  - trap to cpu
 */ 

int32 rtl8306e_trap_igmpCtrlPktAction_set(uint32 type, uint32 action)
{    
    switch(type)
    {
        case RTL8306_IGMP:
            if (RTL8306_ACT_PERMIT == action)
            {
                rtl8306e_regbit_set(2, 21, 14, 3, 0);
            }
            else if (RTL8306_ACT_TRAPCPU == action)
            {
                rtl8306e_regbit_set(2, 21, 14, 3, 1);                
            }
            else
            {
                return FAILED;
            }            
            break;
            
        case RTL8306_MLD:
            if (RTL8306_ACT_PERMIT == action)
            {
                rtl8306e_regbit_set(2, 21, 13, 3, 0);
            }
            else if (RTL8306_ACT_TRAPCPU == action)
            {
                rtl8306e_regbit_set(2, 21, 13, 3, 1);                
            }
            else
            {
                return FAILED;
            }            
            
            break;

         case RTL8306_PPPOE:
            if (RTL8306_ACT_PERMIT == action)
            {
                rtl8306e_regbit_set(2, 22, 4, 3, 1);
            }
            else if (RTL8306_ACT_TRAPCPU == action)
            {
                rtl8306e_regbit_set(2, 22, 4, 3, 0);                
            }
            else
            {
                return FAILED;
            }                        
            break;
            
         default:
            return FAILED;
           
    }

    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_trap_igmpCtrlPktAction_get
 * Description:
 *      Get IGMP/MLD trap setting
 * Input:
 *      type         -  Specify IGMP/MLD or PPPOE
 * Output:
 *      pAction     -  the pointer of action could be normal forward or trap
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     type could be:
 *          RTL8306_IGMP   - igmp packet without pppoe header 
 *          RTL8306_MLD    - mld packet  without pppoe header
 *          RTL8306_PPPOE - pppoe packet if enable trap RTL8306_IGMP 
 *                                   or RTL8306_MLD whether pppoe packet 
 *                                   should be trapped. In some application,
 *                                   igmp and mld message is encapsulated in pppoed
 *                                   packet.
 *      action could be:
 *          RTL8306_ACT_PERMIT    - normal forward
 *          RTL8306_ACT_TRAPCPU  - trap to cpu
 */ 

int32 rtl8306e_trap_igmpCtrlPktAction_get(uint32 type, uint32 *pAction)
{
    uint32 regval;

    if (NULL == pAction)
        return FAILED;
    
    switch(type)
    {
        case RTL8306_IGMP:
            rtl8306e_regbit_get(2, 21, 14, 3, &regval);
            *pAction = regval ? RTL8306_ACT_TRAPCPU : RTL8306_ACT_PERMIT;
            break;            
        case RTL8306_MLD:
            rtl8306e_regbit_get(2, 21, 13, 3, &regval);
            *pAction = regval ? RTL8306_ACT_TRAPCPU : RTL8306_ACT_PERMIT;            
            break;
         case RTL8306_PPPOE:
            rtl8306e_regbit_get(2, 22, 4, 3, &regval);
            *pAction = regval ?  RTL8306_ACT_PERMIT : RTL8306_ACT_TRAPCPU;
            break;            
         default:
            return FAILED;           
    }
    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_trap_unknownIPMcastPktAction_set
 * Description:
 *      Set unknown ip multicast drop or normal forward
 * Input:
 *      type         -  Specify ipv4 or ipv6 unkown multicast
 *      action       -  drop or normal forward
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      type coulde be:
 *          RTL8306_IPV4_MULTICAST - ipv4 unknown multicast
 *          RTL8306_IPV6_MULTICAST - ipv6 unknown multicast
 *      action could be:
 *          RTL8306_ACT_DROP      - drop 
 *          RTL8306_ACT_PERMIT   - normal forward
 */  
int32 rtl8306e_trap_unknownIPMcastPktAction_set(uint32 type, uint32 action)
{    
    switch(type)
    {
        case RTL8306_IPV4_MULTICAST:
            if(RTL8306_ACT_DROP == action)
                rtl8306e_regbit_set(4, 21, 9, 0, 1);
            else if (RTL8306_ACT_PERMIT == action)
                rtl8306e_regbit_set(4, 21, 9, 0, 0);
            break;
            
        case RTL8306_IPV6_MULTICAST:
            if(RTL8306_ACT_DROP == action)
                rtl8306e_regbit_set(4, 21, 8, 0, 1);
            else if (RTL8306_ACT_PERMIT == action)
                rtl8306e_regbit_set(4, 21, 8, 0, 0);                                                
            break;
            
        default:
            return FAILED;
    }

    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_trap_unknownIPMcastPktAction_get
 * Description:
 *      Get unknown ip multicast drop or normal forward
 * Input:
 *      type         -  Specify ipv4 or ipv6 unkown multicast
 * Output:
 *      pAction     -  the pointer of drop or normal forward
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      type coulde be:
 *          RTL8306_IPV4_MULTICAST - ipv4 unknown multicast
 *          RTL8306_IPV6_MULTICAST - ipv6 unknown multicast
 *      action could be:
 *          RTL8306_ACT_DROP      - drop
 *          RTL8306_ACT_PERMIT   - normal forward
 */ 

int32 rtl8306e_trap_unknownIPMcastPktAction_get(uint32 type, uint32 *pAction)
{
    uint32 regval;
    
    if (NULL == pAction)
        return FAILED;

    switch(type)
    {
        case RTL8306_IPV4_MULTICAST:
            rtl8306e_regbit_get(4, 21, 9, 0, &regval);
            *pAction = regval ? RTL8306_ACT_DROP: RTL8306_ACT_PERMIT;
            break;
        case RTL8306_IPV6_MULTICAST:
            rtl8306e_regbit_get(4, 21, 8, 0, &regval);
            *pAction = regval ? RTL8306_ACT_DROP: RTL8306_ACT_PERMIT;            
            break;
         default:
            return FAILED;
    }

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_trap_abnormalPktAction_set
 * Description:
 *      set abnormal packet action 
 * Input:
 *      type         -  abnormal packet type
 *      action       -  drop or trap to cpu
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      type coulde be:
 *          RTL8306_UNMATCHVID   - vlan-tagged packet, vid dismatch vlan table 
 *          RTL8306_DOT1XUNAUTH - 802.1x authentication fail packet
 *      action could be:
 *          RTL8306_ACT_DROP       - drop 
 *          RTL8306_ACT_TRAPCPU  - trap to cpu
 */ 

int32 rtl8306e_trap_abnormalPktAction_set(uint32 type,  uint32 action)
{
    switch(type)
    {
        case RTL8306_UNMATCHVID:            
            if(RTL8306_ACT_DROP == action)
                rtl8306e_regbit_set(2, 22, 6, 3, 1);
            else if (RTL8306_ACT_TRAPCPU == action)
                rtl8306e_regbit_set(2, 22, 6, 3, 0);            
            break;
            
        case RTL8306_DOT1XUNAUTH:
            if(RTL8306_ACT_DROP == action)
                rtl8306e_regbit_set(2, 22, 5, 3, 0);
            else if (RTL8306_ACT_TRAPCPU == action)
                rtl8306e_regbit_set(2, 22, 5, 3, 1);                        
            break;
        default:
            return FAILED;
    }
    
    return SUCCESS;
}


/* Function Name:
 *      rtl8306e_trap_abnormalPktAction_get
 * Description:
 *      get abnormal packet action 
 * Input:
 *      type         -  abnormal packet type
 * Output:
 *      pAction     -  the pointer of action
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      type coulde be:
 *          RTL8306_UNMATCHVID   - vlan-tagged packet, vid dismatch vlan table 
 *          RTL8306_DOT1XUNAUTH - 802.1x authentication fail packet
 *      action could be:
 *          RTL8306_ACT_DROP       - drop 
 *          RTL8306_ACT_TRAPCPU  - trap to cpu
 */ 

int32 rtl8306e_trap_abnormalPktAction_get(uint32 type,  uint32 *pAction)
{
    uint32 regval;

    if (NULL ==pAction)
        return FAILED;
    
    switch(type)
    {
        case RTL8306_UNMATCHVID:
            rtl8306e_regbit_get(2, 22, 6, 3, &regval);
            *pAction = regval ? RTL8306_ACT_DROP: RTL8306_ACT_TRAPCPU;
            break;
        case RTL8306_DOT1XUNAUTH:
            rtl8306e_regbit_get(2, 22, 5, 3, &regval);
            *pAction = regval ? RTL8306_ACT_TRAPCPU : RTL8306_ACT_DROP;
            break;
        default:
            return FAILED;
    }

    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_trap_rmaAction_set
 * Description:
 *      Set reserved multicast Mac address forwarding behavior
 * Input:
 *      type         -  reserved Mac address type
 *      action       -  forwarding behavior for the specified mac address
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are eight types  reserved addresses which user can set asic to determine 
 *       how to forwarding them:
 *            RTL8306_RESADDRXX   - reserved address 01-80-c2-00-00-xx 
 *                                              (exclude 00, 01, 02, 03, 10, 20, 21)
 *            RTL8306_RESADDR21   -  reserved address 01-80-c2-00-00-21(GVRP address)
 *            RTL8306_RESADDR20   -  reserved address 01-80-c2-00-00-20(GMRP Address)
 *            RTL8306_RESADDR10   -  reserved address 01-80-c2-00-00-10(All LANs Bridge Management Group Address)
 *            RTL8306_RESADDR03   -  reserved address 01-80-c2-00-00-03(IEEE Std 802.1X PAE address)
 *            RTL8306_RESADDR02   -  reserved address 01-80-c2-00-00-02(IEEE Std 802.3ad Slow_Protocols-Multicast address)
 *            RTL8306_RESADDR00   -  reserved address 01-80-c2-00-00-00(Bridge Group Address)
 *            RTL8306_RESADDR01   -  reserved address 01-80-c2-00-00-01(Pause frame)
 *       Actions are :
 *            TL8306_ACT_DROP      - Drop the packet
 *            TL8306_ACT_TRAPCPU - Trap the packet to cpu
 *            RTL8306_ACT_FLOOD   - Flood the packet
 */ 

int32 rtl8306e_trap_rmaAction_set(uint32 type, uint32 action)
{
    if ((type > RTL8306_RESADDR01) || (action > RTL8306_ACT_FLOOD)) 
        return FAILED;
    
    switch (type) 
    {
        case RTL8306_RESADDR21:
        case RTL8306_RESADDR20:
        case RTL8306_RESADDR10:
        case RTL8306_RESADDR03:
        case RTL8306_RESADDR00: 
            
            /*Above cases have same action*/
            if (RTL8306_ACT_FLOOD == action) 
                rtl8306e_regbit_set(2, 21, type, 3, 0);
            else if (RTL8306_ACT_TRAPCPU == action)
                rtl8306e_regbit_set(2, 21, type, 3, 1);
            else 
                return FAILED;
            break;
            
        case RTL8306_RESADDR02:
            if (RTL8306_ACT_FLOOD == action) 
            {
                rtl8306e_regbit_set(1, 23, 4, 0, 0); 
                rtl8306e_regbit_set(2, 21, type, 3, 0);
            }
            else if (RTL8306_ACT_TRAPCPU == action) 
            {
                rtl8306e_regbit_set(1, 23, 4, 0, 0);
                rtl8306e_regbit_set(2, 21, type, 3, 1);
            }
            else if (RTL8306_ACT_DROP == action) 
                rtl8306e_regbit_set(1, 23, 4, 0, 1); 
            else 
                return FAILED;
            break;
        
        case RTL8306_RESADDRXX:
            if (RTL8306_ACT_FLOOD == action) 
            {
                rtl8306e_regbit_set(0, 18, 12, 0, 1); 
                rtl8306e_regbit_set(2, 21, type, 3, 0);
            } 
            else if (RTL8306_ACT_TRAPCPU == action) 
            {
                rtl8306e_regbit_set(0, 18, 12, 0, 1);
                rtl8306e_regbit_set(2, 21, type, 3, 1);
            }
            else if (RTL8306_ACT_DROP == action) 
                rtl8306e_regbit_set(0, 18, 12, 0, 0);
            else 
                return FAILED;
            break;
        
        case RTL8306_RESADDR01:
            if (RTL8306_ACT_FLOOD == action) 
                rtl8306e_regbit_set(0, 22, 14, 0, 1); 
            else if (RTL8306_ACT_DROP == action)
                rtl8306e_regbit_set(0, 22, 14, 0, 0);
            else 
                return FAILED;
            break;
        default :
            return FAILED;
    }
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_trap_rmaAction_get
 * Description:
 *      Get reserved multicast Mac address forwarding behavior
 * Input:
 *      type         -  reserved Mac address type
 * Output:
 *      pAction     -  the pointer of action
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are eight types  reserved addresses which user can set asic to determine 
 *       how to forwarding them:
 *            RTL8306_RESADDRXX   - reserved address 01-80-c2-00-00-xx 
 *                                              (exclude 00, 01, 02, 03, 10, 20, 21)
 *            RTL8306_RESADDR21   -  reserved address 01-80-c2-00-00-21(GVRP address)
 *            RTL8306_RESADDR20   -  reserved address 01-80-c2-00-00-20(GMRP Address)
 *            RTL8306_RESADDR10   -  reserved address 01-80-c2-00-00-10(All LANs Bridge Management Group Address)
 *            RTL8306_RESADDR03   -  reserved address 01-80-c2-00-00-03(IEEE Std 802.1X PAE address)
 *            RTL8306_RESADDR02   -  reserved address 01-80-c2-00-00-02(IEEE Std 802.3ad Slow_Protocols-Multicast address)
 *            RTL8306_RESADDR00   -  reserved address 01-80-c2-00-00-00(Bridge Group Address)
 *            RTL8306_RESADDR01   -  reserved address 01-80-c2-00-00-01(Pause frame)
 *       Actions are :
 *            TL8306_ACT_DROP      - Drop the packet
 *            TL8306_ACT_TRAPCPU - Trap the packet to cpu
 *            RTL8306_ACT_FLOOD   - Flood the packet
 */ 

int32 rtl8306e_trap_rmaAction_get(uint32 type, uint32 *pAction)
{
    uint32 bitValue1, bitValue2;

    if ((type > RTL8306_RESADDR01) || (NULL == pAction )) 
        return FAILED;

    switch (type)
    {
        case RTL8306_RESADDR21:
        case RTL8306_RESADDR20:
        case RTL8306_RESADDR10:
        case RTL8306_RESADDR03:
        case RTL8306_RESADDR00: 
            /*Above cases have same action*/
            rtl8306e_regbit_get(2, 21, type, 3, &bitValue1);
            if (bitValue1) 
                *pAction = RTL8306_ACT_TRAPCPU; 
            else
                *pAction = RTL8306_ACT_FLOOD; 
            break;
        case RTL8306_RESADDR02:
            rtl8306e_regbit_get(1, 23, 4, 0, &bitValue1);  
            rtl8306e_regbit_get(2, 21, type, 3, &bitValue2);
            if (bitValue1)
                *pAction = RTL8306_ACT_DROP;
            else if (bitValue2)
                *pAction = RTL8306_ACT_TRAPCPU;
            else
                *pAction = RTL8306_ACT_FLOOD;
            break;
        case RTL8306_RESADDRXX:
            rtl8306e_regbit_get(0, 18, 12, 0, &bitValue1); 
            rtl8306e_regbit_get(2, 21, type, 3, &bitValue2);
            if (!bitValue1)
                *pAction = RTL8306_ACT_DROP;
            else if (bitValue2)
                *pAction = RTL8306_ACT_TRAPCPU;
            else
                *pAction = RTL8306_ACT_FLOOD;
                break;
        case RTL8306_RESADDR01:
            rtl8306e_regbit_get(0, 22, 14, 0, &bitValue1); 
            if (bitValue1)
                *pAction = RTL8306_ACT_FLOOD;
            else
                *pAction = RTL8306_ACT_DROP;
            break;
       default :
            return FAILED;
    }
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_int_control_set
 * Description:
 *      Set asic interrupt 
 * Input:
 *      enInt        -  Enable interrupt cpu
 *      intmask     -  interrupt event  mask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      enInt is global setting and  intmask  has 12 bits totally, each bit 
 *      represents one interrupt event, 
 *      - bit0 ~bit4 represent port 0 ~ port 4 link change, 
 *      - bit 5 represents port 4 MAC link change
 *      - bit 6 represents port 5 link change, 
 *      - bit 7 represents storm filter interrupt,
 *      - bit 8 represents loop event 
 *      - bit 9 represents wake up frame interrupt
 *      - bit 10 represents unmatched SA interrupt
 *      - bit 11 represents Tx meter interrupt
 *      write 1 to the bit to enable the interrupt and 0 will disable the interrupt. 
 *
 */ 
int32 rtl8306e_int_control_set(uint32 enInt, uint32 intmask)
{
    uint32 regValue;

    if (intmask > 0xFFF)
        return FAILED;

    if (!enInt) 
    {
        /*CPU interrupt disable, do not change interrupt port mask*/
        rtl8306e_regbit_set(2, 22, 15, 3, 1);
        return SUCCESS;
    } 
    
    /*CPU interrupt enable*/
    rtl8306e_regbit_set(2, 22, 15, 3, 0);
    
    /*Set link change interrupt mask*/
    rtl8306e_reg_get(4, 23, 3, &regValue);
    regValue = (regValue & 0xF000) | (intmask & 0xFFF);
    rtl8306e_reg_set(4, 23, 3, regValue);
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_int_control_get
 * Description:
 *      Get Asic interrupt
 * Input:
 *      none 
 * Output:
 *      pEnInt       -  the pointer of  interrupt global enable bit
 *      pIntmask    -  the pointer of interrupt event  mask 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      enInt is global setting and  intmask  has 12 bits totally, each bit 
 *      represents one interrupt event, 
 *      - bit0 ~bit4 represent port 0 ~ port 4 link change, 
 *      - bit 5 represents port 4 MAC link change
 *      - bit 6 represents port 5 link change, 
 *      - bit 7 represents storm filter interrupt,
 *      - bit 8 represents loop event 
 *      - bit 9 represents wake up frame interrupt
 *      - bit 10 represents unmatched SA interrupt
 *      - bit 11 represents Tx meter interrupt
 *      write 1 to the bit to enable the interrupt and 0 will disable the interrupt. 
 *
 */ 
int32 rtl8306e_int_control_get(uint32 *pEnInt, uint32 *pIntmask) 
{
    uint32 regValue;
    uint32 bitValue;

    if ((NULL == pEnInt) || (NULL == pIntmask))
        return FAILED;
    
    rtl8306e_regbit_get(2, 22, 15, 3, &bitValue);
    *pEnInt = (bitValue ? FALSE : TRUE);
    rtl8306e_reg_get(4, 23, 3, &regValue);
    *pIntmask = regValue & 0xFFF;
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_int_control_get
 * Description:
 *      Get Asic interrupt
 * Input:
 *      none 
 * Output:
 *      pEnInt       -  the pointer of  interrupt global enable bit
 *      pIntmask    -  the pointer of interrupt event  mask 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      enInt is global setting and  intmask  has 12 bits totally, each bit 
 *      represents one interrupt event, 
 *      - bit0 ~bit4 represent port 0 ~ port 4 link change, 
 *      - bit 5 represents port 4 MAC link change
 *      - bit 6 represents port 5 link change, 
 *      - bit 7 represents storm filter interrupt,
 *      - bit 8 represents loop event 
 *      - bit 9 represents wake up frame interrupt
 *      - bit 10 represents unmatched SA interrupt
 *      - bit 11 represents Tx meter interrupt
 *      write 1 to the bit to enable the interrupt and 0 will disable the interrupt. 
 *
 */ 
int32 rtl8306e_int_status_get(uint32 *pStatusMask) 
{
    uint32 regValue;

    if (NULL == pStatusMask)
        return FAILED;
    
    rtl8306e_reg_get(4, 22, 3, &regValue);
    *pStatusMask = regValue & 0xFFF;
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_storm_filterEnable_set
 * Description:
 *      Enable Asic storm filter 
 * Input:
 *      type      -  specify storm filter type
 *      enabled  -  TRUE or FALSE
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are 3 kinds of storm filter:
 *      (1) RTL8306_BROADCASTPKT - storm filter for broadcast packet
 *      (2) RTL8306_MULTICASTPKT - storm filter for multicast packet
 *      (3) RTL8306_UDAPKT           - storm filter for unknown DA packet
 */ 
int32 rtl8306e_storm_filterEnable_set(uint32 type, uint32 enabled) 
{

   /*enable new storm filter*/
   rtl8306e_regbit_set(6, 17, 1, 1, enabled ? 1:0);
   
    switch(type) 
    {
        case RTL8306_BROADCASTPKT:
            rtl8306e_regbit_set(0, 18, 2, 0, enabled ? 0:1);
            break;
        case RTL8306_MULTICASTPKT:
            rtl8306e_regbit_set(2, 23, 9, 3, enabled ? 0:1);
            break;
        case RTL8306_UDAPKT:
            rtl8306e_regbit_set(2, 23, 8, 3, enabled ? 0:1);
            break;
        default:
            return FAILED;
    }
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_storm_filterEnable_get
 * Description:
 *      Get Asic storm filter enabled or disabled 
 * Input:
 *      type        -  specify storm filter type
 * Output:
 *      pEnabled  -  the pointer of enabled or disabled
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are 3 kinds of storm filter:
 *      (1) RTL8306_BROADCASTPKT - storm filter for broadcast packet
 *      (2) RTL8306_MULTICASTPKT - storm filter for multicast packet
 *      (3) RTL8306_UDAPKT           - storm filter for unknown DA packet
 */ 

int32 rtl8306e_storm_filterEnable_get(uint32 type, uint32 *pEnabled) 
{
    uint32 bitValue;

    if (pEnabled == NULL)
        return FAILED;
    
    switch(type) 
   {
        case RTL8306_BROADCASTPKT:
            rtl8306e_regbit_get(0, 18, 2, 0, &bitValue);
            break;
        case RTL8306_MULTICASTPKT:
            rtl8306e_regbit_get(2, 23, 9, 3, &bitValue);
            break;
        case RTL8306_UDAPKT:
            rtl8306e_regbit_get(2, 23, 8, 3, &bitValue);
            break;
        default:
            return FAILED;

    }

    *pEnabled = (bitValue ? FALSE : TRUE);
    return SUCCESS;
}

/* Function Name:
 *      rtl8306e_storm_filter_set
 * Description:
 *      Set storm filter parameter
 * Input:
 *      trigNum        -  set packet threshold which trigger storm filter
 *      filTime         -   set time window 
 *      enStmInt     -   enable storm filter to interrupt cpu
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      there are 6 value for trigNum:
 *      - RTL8306_STM_FILNUM64, 64 pkts
 *      - RTL8306_STM_FILNUM32, 32 pkts           
 *      - RTL8306_STM_FILNUM16, 16 pkts           
 *      - RTL8306_STM_FILNUM8,   8 pkts              
 *      - RTL8306_STM_FILNUM128, 128 pkts          
 *      - RTL8306_STM_FILNUM256, 256 pkts     
 *      there are 4 value for filTime:
 *      - RTL8306_STM_FIL800MS,  800ms 
 *      - RTL8306_STM_FIL400MS,  400ms 
 *      - RTL8306_STM_FIL200MS,  200ms
 *      - RTL8306_STM_FIL100MS,  100ms
 */ 

int32 rtl8306e_storm_filter_set(uint32 trigNum, uint32 filTime, uint32 enStmInt) 
{
    uint32 regval;

   if ( (trigNum > RTL8306_STM_FILNUM256) || (filTime > RTL8306_STM_FIL100MS))
        return FAILED;
    
    if(RTL8306_STM_FILNUM128 == trigNum)
    {
        rtl8306e_reg_get(6, 25, 0, &regval);
        regval &= 0xFC1F;
        regval |= (filTime << 5); 
        rtl8306e_reg_set(6, 25, 0 , regval);        
        rtl8306e_regbit_set(6, 17, 0, 1, 1);
    }
    else if(RTL8306_STM_FILNUM256 == trigNum)
    {
        rtl8306e_reg_get(6, 25, 0, &regval);
        regval &= 0xFC1F;
        regval |= (0x1 << 8) | (filTime << 5); 
        rtl8306e_reg_set(6, 25, 0 , regval);        
        rtl8306e_regbit_set(6, 17, 0, 1, 1);
    }
    else
    {
        rtl8306e_reg_get(6, 25, 0, &regval);
        regval &= 0xFC1F;
        regval |= (trigNum << 8) | (filTime << 5);
        rtl8306e_reg_set(6, 25, 0 , regval);
        rtl8306e_regbit_set(6, 17, 0, 1, 0);        
    }
        
    /*Set whether storm filter interrupt cpu*/
    rtl8306e_regbit_set(4, 23, 7, 3, enStmInt ? 1:0);
    
    /*CPU interrupt enable when enable storm filter interrupt*/
    rtl8306e_regbit_set(2, 22, 15, 3, enStmInt ? 0:1);

    return SUCCESS;

}


/* Function Name:
 *      rtl8306e_storm_filter_get
 * Description:
 *      Get storm filter parameter
 * Input:
 *      none 
 * Output:
 *      pTrigNum        -  the pointer of packet threshold which trigger storm filter
 *      pFilTime          -  the pointer of filter time window 
 *      pEnStmInt       -  the pointer of enable storm filter to interrupt cpu
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      there are 6 value for trigNum:
 *      - RTL8306_STM_FILNUM64, 64 pkts
 *      - RTL8306_STM_FILNUM32, 32 pkts           
 *      - RTL8306_STM_FILNUM16, 16 pkts           
 *      - RTL8306_STM_FILNUM8,   8 pkts              
 *      - RTL8306_STM_FILNUM128, 128 pkts          
 *      - RTL8306_STM_FILNUM256, 256 pkts     
 *      there are 4 value for filTime:
 *      - RTL8306_STM_FIL800MS,  800ms 
 *      - RTL8306_STM_FIL400MS,  400ms 
 *      - RTL8306_STM_FIL200MS,  200ms
 *      - RTL8306_STM_FIL100MS,  100ms
 */ 

int32 rtl8306e_storm_filter_get(uint32 *pTrigNum, uint32 *pFilTime, uint32 *pEnStmInt) 
{
    uint32 bitval, regval;
    
    if ((NULL == pTrigNum) || (NULL == pFilTime) || (NULL == pEnStmInt))
        return FAILED;

    rtl8306e_regbit_get(6, 17, 0, 1, &bitval);

    if(bitval)
    {
        rtl8306e_reg_get(6, 25, 0, &regval);
        *pTrigNum = ((regval >> 8) & 0x3) ? RTL8306_STM_FILNUM256 : RTL8306_STM_FILNUM128;
        *pFilTime = (regval >> 5) & 0x3;
    }
    else
    {
        rtl8306e_reg_get(6, 25, 0, &regval);
        *pTrigNum = (regval >> 8) & 0x3;
        *pFilTime = (regval >> 5) & 0x3;
    }
    rtl8306e_regbit_get(4, 23, 7, 3, pEnStmInt);
    
    return SUCCESS;
}






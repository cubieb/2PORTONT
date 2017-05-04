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
* Purpose : realtek common API
*
* Feature :  This file consists of following modules:
*                
*
*/

#include "rtl8306e_types.h"
#include "rtl8306e_asicdrv.h"
#include "rtk_api.h"
#include "rtk_api_ext.h"
#include <linux/kernel.h>
#include <asm/uaccess.h>


/* Function Name:
 *      rtk_switch_init
 * Description:
 *      Set chip to default configuration enviroment
 * Input:
 *      none
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                     -  Success
 *      RT_ERR_FAILED               -  Failure
 * Note:
 *     The API can set chip registers to default configuration for
 *     different release chip model.
 */
rtk_api_ret_t rtk_switch_init(void)
{
    rtl8306e_asic_init();

    return RT_ERR_OK;
}


#ifndef CONFIG_ETHWAN
/* Function Name:
 *      rtk_switch_maxPktLen_set
 * Description:
 *      Set the max packet length of the specific unit
 * Input:
 *      len       -       max packet length
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                    -  Success
 *      RT_ERR_FAILED               -  Failure
 *      RT_ERR_INPUT                -  Invalid input parameter
 * Note:
 *      The API can set max packet length of the specific unit to
 *          - MAXPKTLEN_1522B,
 *          - MAXPKTLEN_1536B,
 *          - MAXPKTLEN_1552B,
 *          - MAXPKTLEN_2000B, 
 */
rtk_api_ret_t rtk_switch_maxPktLen_set(rtk_switch_maxPktLen_t len)
{
    rtk_api_ret_t retVal;
    uint32          maxlen;

    if (len > MAXPKTLEN_2000B)
        return RT_ERR_INPUT;

    switch(len)
    {
        case MAXPKTLEN_1522B:
            maxlen = RTL8306_MAX_PKT_LEN_1518;
            break;            
        case MAXPKTLEN_1536B:
            maxlen = RTL8306_MAX_PKT_LEN_1536;
            break;
        case MAXPKTLEN_1552B:
            maxlen = RTL8306_MAX_PKT_LEN_1552;
            break;
        case MAXPKTLEN_2000B:
            maxlen = RTL8306_MAX_PKT_LEN_2000;
            break;
        default:
            return FAILED;
    }

    if((retVal = rtl8306e_switch_maxPktLen_set(maxlen)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_switch_maxPktLen_get
 * Description:
 *      Get the max packet length of the specific unit
 * Input:
 *      none
 * Output:
 *      pLen                             -    the pointer of max packet length
 * Return: 
 *      RT_ERR_OK                    -  Success
 *      RT_ERR_FAILED               -  Failure
 *      RT_ERR_NULL_POINTER     -  Input parameter is null pointer 
 * Note:
 *      The API can set max packet length of the specific unit to
 *          - MAXPKTLEN_1522B,
 *          - MAXPKTLEN_1536B,
 *          - MAXPKTLEN_1552B,
 *          - MAXPKTLEN_2000B, 
 */
rtk_api_ret_t rtk_switch_maxPktLen_get(rtk_switch_maxPktLen_t *pLen)
{
    rtk_api_ret_t retVal;
    uint32          maxlen;

    if (NULL == pLen)
        return RT_ERR_NULL_POINTER;
    
    if((retVal = rtl8306e_switch_maxPktLen_get(&maxlen)) != RT_ERR_OK)
        return retVal;

    switch(maxlen)
    {
        case RTL8306_MAX_PKT_LEN_1518:
            *pLen = MAXPKTLEN_1522B;
            break;

        case RTL8306_MAX_PKT_LEN_1536:
            *pLen = MAXPKTLEN_1536B;
            break;
            
        case RTL8306_MAX_PKT_LEN_1552:
            *pLen = MAXPKTLEN_1552B;
            break;
            
        case RTL8306_MAX_PKT_LEN_2000:
            *pLen = MAXPKTLEN_2000B;
            break;

        default:
            return FAILED;
    }
    
    return RT_ERR_OK;
}
#endif

/* Function Name:
 *      rtk_port_phyReg_set
 * Description:
 *      Set PHY register data of the specific port
 * Input:
 *      phy                - phy number, 0 ~ 6 
 *      reg                 - Register id
 *      regData           - Register data
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK         -  Success
 *      RT_ERR_FAILED   -   Failure
 * Note:
 *      This API can set PHY register data of the specific port.
 *      RTL8306E switch has 5 FE port, and port 4 could be set as
 *      phy mode or mac mode, port 5 is mac mode which connect
 *      with mii interface. so parameter phy 0 ~4 means port 0 ~4
 *      phy register, 5 means port 4 mac mode, 6 means port 5.
 *      
 */
 
rtk_api_ret_t rtk_port_phyReg_set(rtk_port_t phy, rtk_port_phy_reg_t reg, rtk_port_phy_data_t regData)
{
    rtk_api_ret_t retVal;
    
    if(phy > (RTL8306_PHY_NUMBER - 1))
        return RT_ERR_PORT_ID;

    retVal = rtl8306e_reg_set(phy, reg, 0, regData);
    if(retVal != SUCCESS)
        return RT_ERR_FAILED;
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_port_phyReg_get
 * Description:
 *      Get PHY register data of the specific port
 * Input:
 *      phy                  - phy number, 0 ~ 6
 *      reg                   - Register id
 * Output:
 *      pData               - the pointer of Register data
 * Return:
 *      RT_ERR_OK        -  Success
 *      RT_ERR_FAILED   -  Failure
 * Note:
 *      This API can set PHY register data of the specific port.
 *      RTL8306E switch has 5 FE port, and port 4 could be set as
 *      phy mode or mac mode, port 5 is mac mode which connect
 *      with mii interface. so parameter phy 0 ~4 means port 0 ~4
 *      phy register, 5 means port 4 mac mode, 6 means port 5.
 *      
 */
 
rtk_api_ret_t rtk_port_phyReg_get(rtk_port_t phy, rtk_port_phy_reg_t reg, rtk_port_phy_data_t *pData) 
{
    rtk_api_ret_t retVal;

    if(phy > (RTL8306_PHY_NUMBER - 1))
        return RT_ERR_PORT_ID;

    retVal = rtl8306e_reg_get(phy, reg, 0, pData);
    if(retVal != SUCCESS)
        return RT_ERR_FAILED;

    return RT_ERR_OK;
}
#ifndef CONFIG_ETHWAN
/* Function Name:
 *      rtk_port_phyAutoNegoAbility_set
 * Description:
 *      Set ethernet PHY auto-negotiation desired ability
 * Input:
 *      port       -  Port id
 *      pAbility   -  Ability structure
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                        
 *      RT_ERR_FAILED                  
 *      RT_ERR_SMI       
 *      RT_ERR_PORT_ID
 *      RT_ERR_PHY_REG_ID
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_BUSYWAIT_TIMEOUT
 *     
 * Note:
 *      (1) RTL8306E switch only has 5 phy, so the input port id should be 0~4.
 *      (2) In auto-negotiation mode, phy autoNegotiation ability must be enabled
 */ 
rtk_api_ret_t rtk_port_phyAutoNegoAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    rtk_api_ret_t retVal;     
    uint32 phyData;    
    uint32 phyEnMsk0;
    uint32 phyEnMsk4;

    if(port > 4)
        return RT_ERR_PORT_ID;         

    if(NULL == pAbility)
        return RT_ERR_NULL_POINTER;

    if(pAbility->Half_10 >= RTK_ENABLE_END  || pAbility->Full_10 >= RTK_ENABLE_END ||
       pAbility->Half_100 >= RTK_ENABLE_END || pAbility->Full_100 >= RTK_ENABLE_END ||
       pAbility->AutoNegotiation >= RTK_ENABLE_END ||pAbility->AsyFC >= RTK_ENABLE_END ||
       pAbility->FC >= RTK_ENABLE_END)
        return RT_ERR_INPUT; 

    /*for PHY auto mode setup*/
    pAbility->AutoNegotiation = 1; 

    phyEnMsk0 = 0;
    phyEnMsk4 = 0;
    
    if(1 == pAbility->Half_10)
    {
        /*10BASE-TX half duplex capable in reg 4.5*/
        phyEnMsk4 = phyEnMsk4 | (1<<5);
    }

    if(1 == pAbility->Full_10)
    {
        /*10BASE-TX full duplex capable in reg 4.6*/
        phyEnMsk4 = phyEnMsk4 | (1<<6);        
    }

    if(1 == pAbility->Half_100)
    {
        /*100BASE-TX half duplex capable in reg 4.7*/
        phyEnMsk4 = phyEnMsk4 | (1<<7);
    }

    if(1 == pAbility->Full_100)
    {
        /*100BASE-TX full duplex capable in reg 4.8*/
        phyEnMsk4 = phyEnMsk4 | (1<<8);
    }
    
    if(1 == pAbility->AutoNegotiation)
    {
        /*Auto-Negotiation setting in reg 0.12*/
        phyEnMsk0 = phyEnMsk0 | (1<<12);
    }

    if(1 == pAbility->AsyFC)
    {
        /*Asymetric flow control in reg 4.11*/
        phyEnMsk4 = phyEnMsk4 | (1<<11);
    }
    
    if(1 == pAbility->FC)
    {
        /*Flow control in reg 4.10*/
        phyEnMsk4 = phyEnMsk4 | (1<<10);
    }

    /*Auto-Negotiation control register setting*/
    if((retVal = rtk_port_phyReg_get(port, (rtk_port_phy_reg_t)PHY_AN_ADVERTISEMENT_REG, &phyData)) != RT_ERR_OK)
        return retVal;

    phyData = (phyData & (~0x0DE0)) | phyEnMsk4;
    if((retVal = rtk_port_phyReg_set(port, (rtk_port_phy_reg_t)PHY_AN_ADVERTISEMENT_REG, phyData)) != RT_ERR_OK)
        return retVal;

    /*Control register setting and restart auto*/
    if((retVal = rtk_port_phyReg_get(port, (rtk_port_phy_reg_t)PHY_CONTROL_REG, &phyData)) != RT_ERR_OK)
        return retVal;
    phyData = (phyData & (~0x3100)) | phyEnMsk0;
    
    /*If have auto-negotiation capable, then restart auto negotiation*/
    if(1 == pAbility->AutoNegotiation)
    {
        phyData = phyData | (1 << 9);
    }

    if((retVal = rtk_port_phyReg_set(port, (rtk_port_phy_reg_t)PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
        return retVal;    

    return RT_ERR_OK;
    
}

/* Function Name:
 *      rtk_port_phyAutoNegoAbility_get
 * Description:
 *      Get ethernet PHY auto-negotiation desired ability
 * Input:
 *      port       -  Port id
 * Output:
 *      pAbility   -  Ability structure
 * Return:
 *      RT_ERR_OK                        
 *      RT_ERR_FAILED                  
 *      RT_ERR_SMI       
 *      RT_ERR_PORT_ID
 *      RT_ERR_PHY_REG_ID
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_PHY_AUTO_NEGO_MODE
 *      RT_ERR_BUSYWAIT_TIMEOUT
 *     
 * Note:
 *      (1) RTL8306E switch only has 5 phy, so the input port id should be 0~4.
 *      (2) In auto-negotiation mode, phy autoNegotiation ability must be enabled
 */   
rtk_api_ret_t rtk_port_phyAutoNegoAbility_get(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    uint32 phyData0;
    uint32 phyData4;
    rtk_api_ret_t retVal;  
    
    if(port > 4)
        return RT_ERR_PORT_ID; 

    if(NULL == pAbility)
        return RT_ERR_NULL_POINTER;

    /*Control register setting and restart auto*/
    if((retVal = rtk_port_phyReg_get(port, (rtk_port_phy_reg_t)PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return retVal;

    /*Auto-Negotiation control register setting*/
    if((retVal = rtk_port_phyReg_get(port, (rtk_port_phy_reg_t)PHY_AN_ADVERTISEMENT_REG, &phyData4)) != RT_ERR_OK)
        return retVal;

    if(!(phyData0 & (1<<12)))
        return RT_ERR_PHY_AUTO_NEGO_MODE;

    if(phyData4 & (1<<11))
        pAbility->AsyFC = 1;
    else
        pAbility->AsyFC = 0;

    if(phyData4 & (1<<10))
        pAbility->FC = 1;
    else
        pAbility->FC = 0;
    
    
    if(phyData4 & (1<<8))
        pAbility->Full_100= 1;
    else
        pAbility->Full_100= 0;
    
    if(phyData4 & (1<<7))
        pAbility->Half_100= 1;
    else
        pAbility->Half_100= 0;

    if(phyData4 & (1<<6))
        pAbility->Full_10= 1;
    else
        pAbility->Full_10= 0;
    
    if(phyData4 & (1<<5))
        pAbility->Half_10= 1;
    else
        pAbility->Half_10= 0;


    if(phyData0 & (1<<12))
        pAbility->AutoNegotiation= 1;
    else
        pAbility->AutoNegotiation= 0;

    return RT_ERR_OK;
}
#endif
/* Function Name:
 *     rtk_port_phyForceModeAbility_set
 * Description:
 *      Set the port PHY force mode, config its speed/duplex mode/pause/asy_pause 
 * Input:
 *      port       -  Port id
 *      pAbility   -  Ability structure
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                        
 *      RT_ERR_FAILED                  
 *      RT_ERR_SMI       
 *      RT_ERR_PORT_ID
 *      RT_ERR_PHY_REG_ID
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_PHY_FORCE_1000
 *      RT_ERR_BUSYWAIT_TIMEOUT
 *     
 * Note:
 *      (1) RTL8306E switch only has 5 phy, so the input port id should be 0~4.
 *      (2) In force mode, phy autoNegotiation ability must be disabled.
 */
rtk_api_ret_t rtk_port_phyForceModeAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    rtk_api_ret_t retVal;           
    uint32 phyData;
    uint32 phyEnMsk0;
    uint32 phyEnMsk4;
    
    if(port > 4)
        return RT_ERR_PORT_ID;  
    
    if(NULL == pAbility)
        return RT_ERR_NULL_POINTER;

    if(pAbility->Half_10 >= RTK_ENABLE_END || pAbility->Full_10 >= RTK_ENABLE_END ||
       pAbility->Half_100 >= RTK_ENABLE_END || pAbility->Full_100 >= RTK_ENABLE_END ||
       pAbility->AutoNegotiation >= RTK_ENABLE_END ||pAbility->AsyFC >= RTK_ENABLE_END ||
       pAbility->FC >= RTK_ENABLE_END)
        return RT_ERR_INPUT; 

    /*for PHY force mode setup*/
    pAbility->AutoNegotiation = 0;
    
    phyEnMsk0 = 0;
    phyEnMsk4 = 0;
    
    if(1 == pAbility->Half_10)
    {
        /*Speed selection in reg 0.13 */
        /* 1= 100Mpbs*/
        /* 0= 10Mpbs*/        
        phyEnMsk0 = phyEnMsk0 & (~(1<<13));

        /*Half duplex mode in reg 0.8*/
        phyEnMsk0 = phyEnMsk0 & (~(1<<8));
    }

    if(1 == pAbility->Full_10)
    {
        /*Speed selection in reg 0.13 */
        /* 1= 100Mpbs*/
        /* 0= 10Mpbs*/        
        phyEnMsk0 = phyEnMsk0 & (~(1<<13));

        /*Full duplex mode in reg 0.8*/
        phyEnMsk0 = phyEnMsk0 | (1<<8);
        
    }

    if(1 == pAbility->Half_100)
    {
        /*Speed selection in reg 0.13 */
        /* 1= 100Mpbs*/
        /* 0= 10Mpbs*/       
        phyEnMsk0 = phyEnMsk0 | (1<<13);
        
        /*Half duplex mode in reg 0.8*/
        phyEnMsk0 = phyEnMsk0 & (~(1<<8));
    }


    if(1 == pAbility->Full_100)
    {
        /*Speed selection in reg 0.13 */
        /* 1= 100Mpbs*/
        /* 0= 10Mpbs*/       
        phyEnMsk0 = phyEnMsk0 | (1<<13);
        
        /*Full duplex mode in reg 0.8*/
        phyEnMsk0 = phyEnMsk0 | (1<<8);
    }
        
    if(1 == pAbility->AsyFC)
    {
        /*Asymetric flow control in reg 4.11*/
        phyEnMsk4 = phyEnMsk4 | (1<<11);
    }
    if(1 == pAbility->FC)
    {
        /*Flow control in reg 4.10*/
        phyEnMsk4 = phyEnMsk4 | (1<<10);
    }
    
    /*Auto-Negotiation control register setting*/
    if((retVal = rtk_port_phyReg_get(port, (rtk_port_phy_reg_t)PHY_AN_ADVERTISEMENT_REG, &phyData))!= RT_ERR_OK)
        return retVal;

    phyData = (phyData & (~0x0DE0)) | phyEnMsk4;
    if((retVal = rtk_port_phyReg_set(port, (rtk_port_phy_reg_t)PHY_AN_ADVERTISEMENT_REG, phyData)) != RT_ERR_OK)
        return retVal;

    /*Control register setting and restart auto*/
    if((retVal = rtk_port_phyReg_get(port, (rtk_port_phy_reg_t)PHY_CONTROL_REG, &phyData)) != RT_ERR_OK)
        return retVal;
    phyData = (phyData & (~0x3100)) | phyEnMsk0;
    if((retVal = rtk_port_phyReg_set(port, (rtk_port_phy_reg_t)PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
        return retVal;
    
       
    return RT_ERR_OK;
}
#ifndef CONFIG_ETHWAN

/* Function Name:
 *      rtk_port_phyForceModeAbility_get
 * Description:
 *      Get the port PHY speed/duplex mode/pause/asy_pause in force mode
 * Input:
 *      port       -  Port id
 * Output:
 *      pAbility   -  Ability structure
 * Return:
 *      RT_ERR_OK                        
 *      RT_ERR_FAILED                  
 *      RT_ERR_SMI       
 *      RT_ERR_PORT_ID
 *      RT_ERR_PHY_REG_ID
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_BUSYWAIT_TIMEOUT
 *     
 * Note:
 *      (1) RTL8306E switch only has 5 phy, so the input port id should be 0~4.
 *      (2) In force mode, phy autoNegotiation ability must be disabled.
 */
rtk_api_ret_t rtk_port_phyForceModeAbility_get(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    uint32 phyData0;
    uint32 phyData4;
    rtk_api_ret_t retVal;    
    
    if(port > 4)
        return RT_ERR_PORT_ID; 
    
    if(NULL == pAbility)
        return RT_ERR_NULL_POINTER;
                 

    /*Control register setting and restart auto*/
    if((retVal = rtk_port_phyReg_get(port, (rtk_port_phy_reg_t)PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return retVal;

    /*Auto-Negotiation control register setting*/
    if((retVal = rtk_port_phyReg_get(port, (rtk_port_phy_reg_t)PHY_AN_ADVERTISEMENT_REG, &phyData4)) != RT_ERR_OK)
        return retVal;

    if(phyData0 & (1<<12))
        return RT_ERR_PHY_FORCE_ABILITY;

    if(phyData4 & (1<<11))
        pAbility->AsyFC = 1;
    else
        pAbility->AsyFC = 0;

    if(phyData4 & (1<<10))
        pAbility->FC = 1;
    else
        pAbility->FC = 0;
    

    pAbility->Full_100 = 0;
    pAbility->Half_100 = 0;
    pAbility->Full_10 = 0;    
    pAbility->Half_10 = 0;
    pAbility->Full_1000 = 0;

    if(phyData0 & (1<<8) && phyData0 & (1<<13))
        pAbility->Full_100= 1;
    
    if(!(phyData0 & (1<<8)) && phyData0 & (1<<13))
        pAbility->Half_100= 1;

    if(phyData0 & (1<<8) && !(phyData0 & (1<<13)))
        pAbility->Full_10= 1;
    
    if(!(phyData0 & (1<<8)) && !(phyData0 & (1<<13)))
        pAbility->Half_10= 1;

    if(phyData0 & (1<<12))
        pAbility->AutoNegotiation= 1;
    else
        pAbility->AutoNegotiation= 0;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_phyStatus_get
 * Description:
 *      Get ethernet PHY linking status
 * Input:
 *      port             -  Port id
 * Output:
 *      pLinkStatus   -  the pointer of PHY link status 
 *      pSpeed         -  the pointer of PHY link speed
 *      pDuplex        -  the pointer of PHY duplex 
 * Return:
 *      RT_ERR_OK                        
 *      RT_ERR_FAILED                  
 *      RT_ERR_SMI       
 *      RT_ERR_PORT_ID
 *      RT_ERR_NULL_POINTER
 *     
 * Note:
 *      RTL8306E switch only has 5 phy, so the input port id should be 0~4.
 */
rtk_api_ret_t rtk_port_phyStatus_get(rtk_port_t port, rtk_port_linkStatus_t *pLinkStatus, rtk_port_speed_t *pSpeed, rtk_port_duplex_t *pDuplex)
{
    rtk_api_ret_t retVal;
    uint32 phyData0;
    uint32  linkUp;
    
    if(port > 4)
        return RT_ERR_PORT_ID; 
    
    if((NULL == pLinkStatus) || (NULL == pSpeed) || (NULL == pDuplex))
        return RT_ERR_NULL_POINTER;

    rtl8306e_port_phyLinkStatus_get(port, &linkUp);
    if (TRUE == linkUp)
    {
        *pLinkStatus = PORT_LINKUP;
        /*Control register setting and restart auto*/
        if((retVal = rtk_port_phyReg_get(port, (rtk_port_phy_reg_t)PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
            return retVal;
        *pSpeed = ((phyData0 & (0x1 << 13)) >> 13) ? PORT_SPEED_100M : PORT_SPEED_10M; 
        *pDuplex = ((phyData0 & (0x1 << 8)) >> 8) ? PORT_FULL_DUPLEX : PORT_HALF_DUPLEX;
    }
    else
    {
        *pLinkStatus = PORT_LINKDOWN;
        *pSpeed = PORT_SPEED_10M;
        *pDuplex = PORT_HALF_DUPLEX; 
    }
  
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_macForceLinkExt0_set
 * Description:
 *      Set external interface 0(MAC 5) force linking configuration.
 * Input:
 *      mode - external interface mode
 *      pPortability - port ability configuration
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - Success
 *      RT_ERR_FAILED          - Failure
 *      RT_ERR_INPUT       - Invalid input parameters.
 * Note:
 *      (1) This API can set external interface 0(MAC 5) force mode properties,
 *      including link status,speed,duplex,and tx pause and tx pause ability.
 *      In force mode, autoNegotiation ability must be disabled.
 *      (2) For 8306E, the external interface 0(MAC 5) operating mode can be
 *      MAC Mode (T)MII, PHY Mode (T)MII and RMII. The operating mode is 
 *      determined by strapping pin upon reset, and can not be configured 
 *      by software, except the selection of MII or TMII. 
 *      (3)The input parameter mode here is only used to select MII or TMII.
 *      When operating mode is configured as MAC Mode (T)MII or PHY Mode (T)MII
 *      via strapping pin, then the selection of MII or TMII can be done via software.
 *      For example, set mode MODE_EXT_TMII_MAC to select TMII, and set mode
 *      MODE_EXT_MII_MAC to select MII.
 */
rtk_api_ret_t rtk_port_macForceLinkExt0_set(rtk_mode_ext_t mode, rtk_port_mac_ability_t *pPortability)
{
    rtk_api_ret_t retVal;
    uint32 regVal;

    if (pPortability->speed > 1 || pPortability->duplex > 1 || pPortability->txpause > 1 ||
        pPortability->rxpause > 1 || pPortability->link > 1)
        return RT_ERR_INPUT;

    if (MODE_EXT_TMII_MAC == mode || MODE_EXT_TMII_PHY == mode)
    {
        rtl8306e_regbit_set (0, 16, 14, 0, 1);
        rtl8306e_regbit_set (4, 27, 12, 0, 1);    
    }
    else if (MODE_EXT_MII_MAC == mode || MODE_EXT_MII_PHY == mode)
    {
        rtl8306e_regbit_set (0, 16, 14, 0, 0);
        rtl8306e_regbit_set (4, 27, 12, 0, 0); 
    }

    /*must be in forcemode*/
    pPortability->forcemode = 1;

    /*Nway ability must be disabled*/
    pPortability->nway = 0;
    if ((retVal = rtl8306e_regbit_set (6, 22, 6, 0, 0)) != RT_ERR_OK)
        return retVal;
    
    if ((retVal = rtl8306e_regbit_set (6, 22, 15, 0, pPortability->link ? 1 : 0)) != RT_ERR_OK)
        return retVal;
    
    rtl8306e_reg_get(6, 22, 0, &regVal);
    regVal &= (~0x30);
    if ((pPortability->speed == PORT_SPEED_100M) && (pPortability->duplex == PORT_FULL_DUPLEX))
        regVal |= 0x30; 
    else if ((pPortability->speed == PORT_SPEED_10M) && (pPortability->duplex == PORT_FULL_DUPLEX))
        regVal |= 0x10;
    else if ((pPortability->speed == PORT_SPEED_100M) && (pPortability->duplex == PORT_HALF_DUPLEX ))
        regVal |= 0x20;
    else
        regVal |= 0x0;
    if ((retVal = rtl8306e_reg_set (6, 22, 0, regVal)) != RT_ERR_OK)
        return retVal;

    /*Enable asymmetric flow control of MAC 5*/
    if ((retVal = rtl8306e_regbit_set (6, 24, 12, 0, 1)) != RT_ERR_OK)
        return retVal;
    if ((retVal = rtl8306e_regbit_set (6, 24, 14, 0, pPortability->txpause ? 0 : 1)) != RT_ERR_OK)
        return retVal;
    if ((retVal = rtl8306e_regbit_set (6, 24, 13, 0, pPortability->rxpause ? 0 : 1)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_port_macForceLinkExt0_get
 * Description:
 *      Get external interface 0(MAC 5) force linking configuration.
 * Input:
 *      None
 * Output:
 *      pMode - external interface mode
 *      pPortability - port ability configuration
 * Return:
 *      RT_ERR_OK              - Success
 *      RT_ERR_FAILED          - Failure 
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This API can get external interface 0 (MAC 5) force mode properties.
 *      The external interface 0 operating mode can be:
 *      - MODE_EXT_MII_MAC,
 *      - MODE_EXT_MII_PHY, 
 *      - MODE_EXT_TMII_MAC,
 *      - MODE_EXT_TMII_PHY, 
 *      - MODE_EXT_RMII, 
 */
rtk_api_ret_t rtk_port_macForceLinkExt0_get(rtk_mode_ext_t *pMode, rtk_port_mac_ability_t *pPortability)
{
    uint32 regVal;
    uint32 modesel;

    rtl8306e_regbit_set(0, 16, 11, 0, 1);
    rtl8306e_reg_get(6, 26, 3, &regVal);
    modesel = regVal & 0x1F;
    if (0x1 == modesel || 0x2 == modesel || 0x9 == modesel || 0x12 == modesel || 0x10 == modesel)
        *pMode = MODE_EXT_MII_PHY;
    else if (0xA == modesel || 0xB == modesel)
        *pMode = MODE_EXT_MII_MAC;
    else if (0x0 == modesel || 0x8 == modesel || 0x11 == modesel)
        *pMode = MODE_EXT_RMII;
    else
        *pMode = MODE_EXT_END;
    
    rtl8306e_regbit_get (0, 16, 14, 0, &regVal);
    if (regVal && (MODE_EXT_MII_PHY == *pMode))
        *pMode = MODE_EXT_TMII_PHY;
    
    if (regVal && (MODE_EXT_MII_MAC == *pMode))
        *pMode = MODE_EXT_TMII_MAC;

    pPortability->forcemode = 1;
    rtl8306e_regbit_get (6, 22, 6, 0, &regVal);
    pPortability->nway = regVal ? 1 : 0;
    
    rtl8306e_regbit_get (6, 24, 13, 0, &regVal);
    pPortability->rxpause = regVal ? 0 : 1;

    rtl8306e_regbit_get (6, 24, 14, 0, &regVal);
    pPortability->txpause = regVal ? 0 : 1;

    rtl8306e_regbit_get (6, 22, 15, 0, &regVal);
    pPortability->link= regVal ? 1 : 0;

    rtl8306e_reg_get (6, 22, 0, &regVal);
    if ((regVal & (0x3 << 4)) >> 4 == 0x3)
    {
        pPortability->speed =1;
        pPortability->duplex =1;
    }
    else if ((regVal & (0x3 << 4)) >> 4 == 0x2)
    {
        pPortability->speed = 1;
        pPortability->duplex =0;
    }
    else if ((regVal & (0x3 << 4)) >> 4 == 0x1)
    {
        pPortability->speed = 0;
        pPortability->duplex =1;
    }
    else if ((regVal & (0x3 << 4)) >> 4 == 0x0)
    {
        pPortability->speed = 0;
        pPortability->duplex =0;
    }
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_isolation_set
 * Description:
 *      Set permitted port isolation portmask
 * Input:
 *      port                - port id, 0 ~ 5 
 *      portmask         - Permit port mask
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK             -   Success
 *      RT_ERR_PORT_ID     -   Invalid port number
 *      RT_ERR_PORT_MASK -   Invalid portmask
 *      RT_ERR_FAILED        -   Failure
 * Note:
 *      This API set the port mask that a port can trasmit packet to of each port.
 *      A port can only transmit packet to ports included in permitted portmask  
 */
rtk_api_ret_t rtk_port_isolation_set(rtk_port_t port, rtk_portmask_t portmask)
{
    rtk_api_ret_t retVal;
    uint32 isomask;
    uint32 regval;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if( portmask.bits[0] > RTK_MAX_PORT_MASK)
        return RT_ERR_PORT_MASK;
    
    isomask = (~ (portmask.bits[0])) & 0x3F;
    rtl8306e_reg_get(0, 29, 3, &regval);
    switch (port)
    {
        case 0:
            regval &= ~0x1F;
            if (isomask & 0x2)
                regval |=  (1 << 0); 
            if (isomask & 0x4)
                regval |=  (1 << 1); 
            if (isomask & 0x8)
                regval |=  (1 << 2); 
            if (isomask & 0x10)
                regval |=  (1 << 3); 
            if (isomask & 0x20)
                regval |=  (1 << 4); 
            break;
        case 1:
            regval &= ~0x1E1;
            if (isomask & 0x1)
                regval |=  (1 << 0); 
            if (isomask & 0x4)
                regval |=  (1 << 5); 
            if (isomask & 0x8)
                regval |=  (1 << 6); 
            if (isomask & 0x10)
                regval |=  (1 << 7); 
            if (isomask & 0x20)
                regval |=  (1 << 8); 
            break;
        case 2:
            regval &= ~0xE42;
            if (isomask & 0x1)
                regval |=  (1 << 1); 
            if (isomask & 0x2)
                regval |=  (1 << 5); 
            if (isomask & 0x8)
                regval |=  (1 << 9); 
            if (isomask & 0x10)
                regval |=  (1 << 10); 
            if (isomask & 0x20)
                regval |=  (1 << 11); 
            break;
        case 3:
            regval &= ~0x3244;
            if (isomask & 0x1)
                regval |=  (1 << 2); 
            if (isomask & 0x2)
                regval |=  (1 << 6); 
            if (isomask & 0x4)
                regval |=  (1 << 9);
            if (isomask & 0x10)
                regval |=  (1 << 12); 
            if (isomask & 0x20)
                regval |=  (1 << 13); 
            break;
        case 4:
            regval &= ~0x5488;
            if (isomask & 0x1)
                regval |=  (1 << 3); 
            if (isomask & 0x2)
                regval |=  (1 << 7); 
            if (isomask & 0x4)
                regval |=  (1 << 10);
            if (isomask & 0x8)
                regval |=  (1 << 12); 
            if (isomask & 0x20)
                regval |=  (1 << 14); 
            break;
        case 5:
            regval &= ~0x6910;
            if (isomask & 0x1)
                regval |=  (1 << 4); 
            if (isomask & 0x2)
                regval |=  (1 << 8); 
            if (isomask & 0x4)
                regval |=  (1 << 11);
            if (isomask & 0x8)
                regval |=  (1 << 13); 
            if (isomask & 0x10)
                regval |=  (1 << 14); 
            break;
        default:
            return RT_ERR_PORT_ID;
    }
    if((retVal= rtl8306e_reg_set(0, 29, 3, regval)) != RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_isolation_get
 * Description:
 *      Get permitted port isolation portmask
 * Input:
 *      port                - port id, 0 ~ 5 
 * Output:
 *      pPortmask       - the pointer of permit port mask
 * Return:
 *      RT_ERR_OK                  -   Success
 *      RT_ERR_PORT_ID          -   Invalid port number
 *      RT_ERR_NULL_POINTER  -   Input parameter is null pointer
 *      RT_ERR_FAILED             -   Failure
 * Note:
 *      This API get the port mask that a port can transmit packet to of each port.
 *      A port can only transmit packet to ports included in permitted portmask  
 */
rtk_api_ret_t rtk_port_isolation_get(rtk_port_t port, rtk_portmask_t *pPortmask)
{
    uint32 isomask;
    uint32 regval;
    
    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;
    if (NULL == pPortmask)
        return RT_ERR_NULL_POINTER;
    
    isomask = 0;
    rtl8306e_reg_get(0, 29, 3, &regval);
    switch (port)
    {
        case 0:
            isomask |= 0x1;
            if(regval & (1 << 0))
                isomask |= 0x2;
            if(regval & (1 << 1))
                isomask |= 0x4;
            if(regval & (1 << 2))
                isomask |= 0x8;
            if(regval & (1 << 3))
                isomask |= 0x10;
            if(regval & (1 << 4))
                isomask |= 0x20;
            break;
        case 1:
            isomask |= 0x2;
            if(regval & (1 << 0))
                isomask |= 0x1;
            if(regval & (1 << 5))
                isomask |= 0x4;
            if(regval & (1 << 6))
                isomask |= 0x8;
            if(regval & (1 << 7))
                isomask |= 0x10;
            if(regval & (1 << 8))
                isomask |= 0x20;
            break;
        case 2:
            isomask |= 0x4;
            if(regval & (1 << 1))
                isomask |= 0x1;
            if(regval & (1 << 5))
                isomask |= 0x2;
            if(regval & (1 << 9))
                isomask |= 0x8;
            if(regval & (1 << 10))
                isomask |= 0x10;
            if(regval & (1 << 11))
                isomask |= 0x20;
            break;
        case 3:
            isomask |= 0x8;
            if(regval & (1 << 2))
                isomask |= 0x1;
            if(regval & (1 << 6))
                isomask |= 0x2;
            if(regval & (1 << 9))
                isomask |= 0x4;
            if(regval & (1 << 12))
                isomask |= 0x10;
            if(regval & (1 << 13))
                isomask |= 0x20;
            break;
        case 4:
            isomask |= 0x10;
            if(regval & (1 << 3))
                isomask |= 0x1;
            if(regval & (1 << 7))
                isomask |= 0x2;
            if(regval & (1 << 10))
                isomask |= 0x4;
            if(regval & (1 << 12))
                isomask |= 0x8;
            if(regval & (1 << 14))
                isomask |= 0x20;
            break;
        case 5:
            isomask |= 0x20;
            if(regval & (1 << 4))
                isomask |= 0x1;
            if(regval & (1 << 8))
                isomask |= 0x2;
            if(regval & (1 << 11))
                isomask |= 0x4;
            if(regval & (1 << 13))
                isomask |= 0x8;
            if(regval & (1 << 14))
                isomask |= 0x10;
            break;
        default:
            return RT_ERR_PORT_ID;
    }
    pPortmask->bits[0] = (~isomask) & 0x3F;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_stat_port_reset
 * Description:
 *      Reset per port MIB counter by port.
 * Input:
 *      port - port id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - set shared meter successfully
 *      RT_ERR_FAILED          - FAILED to iset shared meter
 * Note:
 */
rtk_api_ret_t rtk_stat_port_reset(rtk_port_t port)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID; 
    
    if ((retVal = rtl8306e_mib_reset(port) ) != RT_ERR_OK)
        return retVal; 
        
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_igrBandwidthCtrlRate_set
 * Description:
 *      Set port ingress bandwidth control
 * Input:
 *      port            -  Port id
 *      rate            -  Rate of share meter
 *      ifg_include   -  Rate's calculation including IFG, ENABLED:include DISABLED:exclude
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK              -  Success
 *      RT_ERR_PORT_ID      -  Invalid port number
 *      RT_ERR_FAILED        -  Failure
 *      RT_ERR_ENABLE       -  Invalid IFG parameter
 *      RT_ERR_INBW_RATE -  Invalid ingress rate parameter
 * Note:
 *      The rate unit is 64Kbps and the range is from 64Kbps to 100Mbps. The granularity of rate is 64Kbps. 
 *      The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble. 
 */

rtk_api_ret_t rtk_rate_igrBandwidthCtrlRate_set( rtk_port_t port, rtk_rate_t rate,  rtk_enable_t ifg_include)
{
    rtk_api_ret_t retVal;

    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;    
    
    if (rate > RTL8306_QOS_RATE_INPUT_MAX) 
        return RT_ERR_INBW_RATE;

    if(ifg_include >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if((retVal = rtl8306e_qos_portRate_set(port, rate, RTL8306_PORT_RX , TRUE)) != RT_ERR_OK)
        return retVal;

    rtl8306e_regbit_set(0, 21, 14, 3, ifg_include ? 1:0);
    
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_rate_igrBandwidthCtrlRate_get
 * Description:
 *      Get port ingress bandwidth control
 * Input:
 *      port             -  Port id
 * Output:
 *      pRate           -  the pointer of rate of share meter
 *      pIfg_include   -  Rate's calculation including IFG, ENABLED:include DISABLED:exclude
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_PORT_ID              -  Invalid port number
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_ENABLE               -  Invalid IFG parameter
 *      RT_ERR_NULL_POINTER      -  null pointer
 * Note:
 *      The rate unit is 64Kbps and the range is from 64Kbps to 100Mbps. The granularity of rate is 64Kbps. 
 *      The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble. 
 */
 
rtk_api_ret_t rtk_rate_igrBandwidthCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pIfg_include)
{
    rtk_api_ret_t retVal;
    uint32 enabled;

    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;    

    if((NULL == pRate) || (NULL == pIfg_include))
        return RT_ERR_NULL_POINTER;
    
    if((retVal = rtl8306e_qos_portRate_get(port, pRate, RTL8306_PORT_RX, &enabled)) != SUCCESS)
        return retVal;

    if(!enabled)
        *pRate = RTL8306_QOS_RATE_INPUT_MAX;

    rtl8306e_regbit_get(0, 21, 14, 3, &enabled);
    *pIfg_include = enabled ? ENABLED : DISABLED;
        
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_egrBandwidthCtrlRate_set
 * Description:
 *      Set port egress bandwidth control
 * Input:
 *      port            -  Port id
 *      rate            -  Rate of bandwidth control
 *      ifg_include   -  Rate's calculation including IFG, ENABLED:include DISABLED:exclude
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                   -  Success
 *      RT_ERR_PORT_ID           -  Invalid port number
 *      RT_ERR_FAILED             -  Failure
 *      RT_ERR_ENABLE             -  Invalid IFG parameter
 *      RT_ERR_QOS_EBW_RATE -  Invalid egress rate parameter
 * Note:
 *      The rate unit is 64Kbps and the range is from 64Kbps to 100Mbps. The granularity of rate is 64Kbps. 
 *      The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble. 
 */

rtk_api_ret_t rtk_rate_egrBandwidthCtrlRate_set(rtk_port_t port, rtk_rate_t rate,  rtk_enable_t ifg_include)
{
    rtk_api_ret_t retVal;

    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;    
    
    if (rate > RTL8306_QOS_RATE_INPUT_MAX) 
        return RT_ERR_QOS_EBW_RATE;

    if(ifg_include >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if((retVal = rtl8306e_qos_portRate_set(port, rate, RTL8306_PORT_TX , TRUE)) != RT_ERR_OK)
        return retVal;

    rtl8306e_regbit_set(port, 17, 15, 2, ifg_include ? 0 :1);
    
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_rate_igrBandwidthCtrlRate_get
 * Description:
 *      Get port ingress bandwidth control
 * Input:
 *      port             -  Port id
 * Output:
 *      pRate           -  the pointer of rate of bandwidth control
 *      pIfg_include   -  Rate's calculation including IFG, ENABLED:include DISABLED:exclude
 * Return:
 *      RT_ERR_OK                     -  Success
 *      RT_ERR_PORT_ID             -  Invalid port number
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_NULL_POINTER      -  null pointer
 * Note:
 *      The rate unit is 64Kbps and the range is from 64Kbps to 100Mbps. The granularity of rate is 64Kbps. 
 *      The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble. 
 */

rtk_api_ret_t rtk_rate_egrBandwidthCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pIfg_include)
{
    rtk_api_ret_t retVal;
    uint32 enabled;
    uint32 disabled;

    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;    

    if((NULL == pRate) || (NULL == pIfg_include))
        return RT_ERR_NULL_POINTER;
    
    if((retVal = rtl8306e_qos_portRate_get(port, pRate, RTL8306_PORT_TX, &enabled)) != SUCCESS)
        return retVal;

    if(!enabled)
        *pRate = RTL8306_QOS_RATE_INPUT_MAX;

    rtl8306e_regbit_get(port, 17, 15, 2, &disabled);
    *pIfg_include = disabled ? DISABLED : ENABLED;

    return RT_ERR_OK;
}
#endif
/* Function Name:
 *      rtk_qos_init
 * Description:
 *      Configure Qos default settings with queue number assigment to each port
 * Input:
 *      queueNum     -  Queue number of each port
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                     -  Success
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_QUEUE_NUM         -  Error queue number
 * Note:
 *    This API will initialize related Qos setting with queue number assigment.
 *    The queue number is from 1 to 4.
 */

rtk_api_ret_t rtk_qos_init(rtk_queue_num_t queueNum)
{
    uint32 queue, port;
    
    if (rtl8306e_qos_queueNum_set(queueNum) == FAILED)
        return FAILED;
    
    switch(queueNum)
    {
        case 1:
            /*priority to qid mapping*/
            rtl8306e_qos_priToQueMap_set(0, 0);
            rtl8306e_qos_priToQueMap_set(1, 0);       
            rtl8306e_qos_priToQueMap_set(2, 0);                   
            rtl8306e_qos_priToQueMap_set(3, 0);                               
            break;
            
        case 2:
            /*priority to qid mapping*/
            rtl8306e_qos_priToQueMap_set(0, 0);
            rtl8306e_qos_priToQueMap_set(1, 0);       
            rtl8306e_qos_priToQueMap_set(2, 1);                   
            rtl8306e_qos_priToQueMap_set(3, 1);                                           
            break;
            
        case 3:
            /*priority to qid mapping*/
            rtl8306e_qos_priToQueMap_set(0, 0);
            rtl8306e_qos_priToQueMap_set(1, 1);       
            rtl8306e_qos_priToQueMap_set(2, 1);                   
            rtl8306e_qos_priToQueMap_set(3, 2);                                                       
            break;            
            
        case 4:
            /*priority to qid mapping*/
            rtl8306e_qos_priToQueMap_set(0, 0);
            rtl8306e_qos_priToQueMap_set(1, 1);       
            rtl8306e_qos_priToQueMap_set(2, 2);                   
            rtl8306e_qos_priToQueMap_set(3, 3);

            for(queue = 0; queue < 4; queue++)
            {
                /*queue threshold*/
                rtl8306e_qos_queFlcThr_set(queue, RTL8306_FCO_QLEN, RTL8306_FCON, RTL8306_FCO_SET0, 9);
                rtl8306e_qos_queFlcThr_set(queue, RTL8306_FCO_QLEN, RTL8306_FCOFF, RTL8306_FCO_SET0, 5);        
                rtl8306e_qos_queFlcThr_set(queue, RTL8306_FCO_DSC, RTL8306_FCON, RTL8306_FCO_SET0, 40);
                rtl8306e_qos_queFlcThr_set(queue, RTL8306_FCO_DSC, RTL8306_FCOFF, RTL8306_FCO_SET0, 28);                                     

            }
            for (port = 0; port < 6; port ++)
            {
                rtl8306e_qos_portFlcThr_set(port, 130, 100,  RTL8306_PORT_TX); 
                for(queue = 0; queue < 4; queue++)
                {
                    /*queue threshold*/
                    rtl8306e_qos_queFlcEnable_set(port, queue, TRUE);           
                }
            }
                                      
            break;
            
        default:
            return RT_ERR_QUEUE_NUM;
            
    }

    rtl8306e_qos_softReset_set();    

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_qos_priSel_set
 * Description:
 *      Configure the priority order among different priority mechanisms.
 * Input:
 *      pPriDec - priority level for port, dscp, 802.1Q, ACL and VID based priority decision.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - success
 *      RT_ERR_NULL_POINTER  -   Input parameter is null pointer
 *      RT_ERR_FAILED          - failure 
 *      RT_ERR_QOS_SEL_PRI_SOURCE - Invalid priority decision source parameter.
 * Note: 
 *      (1)For 8306E, there are 5 types of priority mechanisms that could be set arbitration level, which are 
 *      ACL-based  priority, DSCP-based priority, 1Q-based priority, Port-based priority, VID-based priority.
 *      Each one could be set level from 1 to 5.
 *      (2)ASIC will follow user's arbitration level setting of priority mechanisms to select internal priority for receiving frame. 
 *      If two priority mechanisms are the same level, the ASIC will chose the priority mechanism with highest level to 
 *      assign internal priority to receiving frame.
 */
rtk_api_ret_t rtk_qos_priSel_set(rtk_priority_select_t *pPriDec)
{ 
    rtk_api_ret_t retVal;
    rtl8306e_qosPriArbitPara_t priArbit;

    if (NULL == pPriDec)
        return RT_ERR_NULL_POINTER;
    if (pPriDec->port_pri > 5 || pPriDec->dot1q_pri > 5 || pPriDec->acl_pri > 5 || pPriDec->dscp_pri > 5 
        || pPriDec->vid_pri > 5)
        return RT_ERR_QOS_SEL_PRI_SOURCE;

    priArbit.port_pri_lev = pPriDec->port_pri;
    priArbit.acl_pri_lev = pPriDec->acl_pri;
    priArbit.dscp_pri_lev = pPriDec->dscp_pri;
    priArbit.dot1q_pri_lev = pPriDec->dot1q_pri;
    priArbit.vid_pri_lev = pPriDec->vid_pri;

    if ((retVal = rtl8306e_qos_priSrcArbit_set(priArbit)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_priSel_get
 * Description:
 *      Get the priority order configuration among different priority mechanism.
 * Input:
 *      None
 * Output:
 *      pPriDec - priority level for port, dscp, 802.1Q, ACL and VID based priority decision.
 * Return:
 *      RT_ERR_OK              - success
 *      RT_ERR_NULL_POINTER  -   Input parameter is null pointer
 *      RT_ERR_FAILED          - failure 
 * Note:
 *      (1)For 8306E, there are 5 types of priority mechanisms that could be set arbitration level, which are 
 *      ACL-based  priority, DSCP-based priority, 1Q-based priority,Port-based priority, VID-based priority.
 *      Each one could be set level from 1 to 5.
 *      (2)ASIC will follow user's arbitration level setting of priority mechanisms to select internal priority for receiving frame. 
 *      If two priority mechanisms are the same level, the ASIC will chose the priority mechanism with the highest level to 
 *      assign internal priority to receiving frame.
 */

rtk_api_ret_t rtk_qos_priSel_get(rtk_priority_select_t *pPriDec)
{
    rtk_api_ret_t retVal;
    rtl8306e_qosPriArbitPara_t priArbit;   

    if (NULL == pPriDec)
        return RT_ERR_NULL_POINTER;
    
    if ((retVal = rtl8306e_qos_priSrcArbit_get(&priArbit)) != RT_ERR_OK)
        return retVal;
    pPriDec->port_pri = priArbit.port_pri_lev;
    pPriDec->dot1q_pri = priArbit.dot1q_pri_lev;
    pPriDec->dmac_pri = priArbit.dscp_pri_lev;
    pPriDec->acl_pri = priArbit.acl_pri_lev;
    pPriDec->vid_pri = priArbit.vid_pri_lev;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_1pPriRemap_set
 * Description:
 *      Configure 1Q priorities mapping to internal absolute priority
 * Input:
 *      dot1p_pri   -  802.1p priority value
 *      int_pri       -  internal priority value
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                         -  Success
 *      RT_ERR_FAILED                    -  Failure
 *      RT_ERR_VLAN_PRIORITY        -  Invalid 1p priority
 *      RT_ERR_QOS_INT_PRIORITY   -  Invalid priority
 * Note:
 *      Priority of 802.1Q assignment for internal asic priority, and it is used for queue usage 
 *      and packet scheduling.
 */
rtk_api_ret_t rtk_qos_1pPriRemap_set(rtk_pri_t dot1p_pri, rtk_pri_t int_pri)
{
    rtk_api_ret_t retVal;

    if (int_pri > RTL8306_PRIO3)
        return RT_ERR_QOS_INT_PRIORITY;
    
    if (dot1p_pri > RTL8306_1QTAG_PRIO7)
        return  RT_ERR_VLAN_PRIORITY;
    
    if ((retVal = rtl8306e_qos_1pPriRemap_set(dot1p_pri, int_pri)) != SUCCESS)
        return retVal;
    
    return RT_ERR_OK;    
}

/* Function Name:
 *      rtk_qos_1pPriRemap_get
 * Description:
 *      Get 1Q priorities mapping to internal absolute priority
 * Input:
 *      dot1p_pri    -  802.1p priority value
 * Output:
 *      pInt_pri      -  the pointer of internal priority value
 * Return:
 *      RT_ERR_OK                         -  Success
 *      RT_ERR_FAILED                    -  Failure
 *      RT_ERR_VLAN_PRIORITY        -  Invalid 1p priority
 *      RT_ERR_NULL_POINTER         -   null pointer
 * Note:
 *      Priority of 802.1Q assignment for internal asic priority, and it is used for queue usage 
 *      and packet scheduling.
 */

rtk_api_ret_t rtk_qos_1pPriRemap_get(rtk_pri_t dot1p_pri, rtk_pri_t *pInt_pri)
{
    rtk_api_ret_t retVal;
    
    if(NULL == pInt_pri)
        return RT_ERR_NULL_POINTER;

    if (dot1p_pri > RTL8306_1QTAG_PRIO7)
        return  RT_ERR_VLAN_PRIORITY;
    
    if((retVal = rtl8306e_qos_1pPriRemap_get(dot1p_pri, pInt_pri)) != SUCCESS)
        return retVal;
    
    return RT_ERR_OK;    
}


/* Function Name:
 *      rtk_qos_dscpPriRemap_set
 * Description:
 *      Set DSCP-based priority
 * Input:
 *      code      -  dscp code
 *      int_pri    -  internal priority value
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                         -  Success
 *      RT_ERR_FAILED                    -  Failure
 *      RT_ERR_QOS_INT_PRIORITY   -  invalid internal priority
 *      RT_ERR_QOS_DSCP_VALUE    -   invalid DSCP value  
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
 *     The Differentiated Service Code Point is a selector for router's per-hop behaviors. As a selector, there is no implication that a numerically 
 *     greater DSCP implies a better network service. As can be seen, the DSCP totally overlaps the old precedence field of TOS. So if values of 
 *     DSCP are carefully chosen then backward compatibility can be achieved.         
 */ 
rtk_api_ret_t rtk_qos_dscpPriRemap_set(rtk_dscp_t dscp, rtk_pri_t int_pri)
{
    rtk_api_ret_t retVal;

    if (int_pri > RTL8306_PRIO3 )
        return RT_ERR_QOS_INT_PRIORITY; 

    if (dscp > RTL8306_DSCP_BF)
        return RT_ERR_QOS_DSCP_VALUE; 

    if((retVal = rtl8306e_qos_dscpPriRemap_set(dscp, int_pri)) != SUCCESS)
        return RT_ERR_FAILED;
    
    return RT_ERR_OK;    
}


/* Function Name:
 *      rtk_qos_dscpPriRemap_get
 * Description:
 *      Get DSCP-based priority
 * Input:
 *      dscp      -  dscp code
 * Output:
 *      pInt_pri  -  the pointer of internal priority value
 * Return:
 *      RT_ERR_OK                           -  Success
 *      RT_ERR_FAILED                     -  Failure
 *      RT_ERR_QOS_DSCP_VALUE      -  Invalid DSCP value
 *      RT_ERR_NULL_POINTER           -  Input parameter is null pointer
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
 *     The Differentiated Service Code Point is a selector for router's per-hop behaviors. As a selector, there is no implication that a numerically 
 *     greater DSCP implies a better network service. As can be seen, the DSCP totally overlaps the old precedence field of TOS. So if values of 
 *     DSCP are carefully chosen then backward compatibility can be achieved.         
 */ 
rtk_api_ret_t rtk_qos_dscpPriRemap_get(rtk_dscp_t dscp, rtk_pri_t *pInt_pri)
{
    rtk_api_ret_t retVal;
    
    if (dscp > RTL8306_DSCP_BF)
        return RT_ERR_QOS_DSCP_VALUE; 
    
    if(NULL == pInt_pri)
        return RT_ERR_NULL_POINTER;

    if((retVal = rtl8306e_qos_dscpPriRemap_get(dscp, pInt_pri)) != SUCCESS)
        return RT_ERR_FAILED;
    

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_qos_portPri_set
 * Description:
 *      Configure priority usage to each port
 * Input:
 *      port                - Port id.                
 *      int_pri             -  internal priority value
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                        -  Success
 *      RT_ERR_FAILED                  -   Failure
 *      RT_ERR_PORT_ID                -   Error port id
 *      RT_ERR_QOS_INT_PRIORITY  -   Error internal priority value
 * Note:
 *     The API can set priority of port assignments for queue usage and packet scheduling.
 */
rtk_api_ret_t rtk_qos_portPri_set(rtk_port_t port, rtk_pri_t int_pri)
{
    rtk_api_ret_t retVal;
    
    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;    

    if (int_pri > RTL8306_PRIO3 )
        return RT_ERR_QOS_INT_PRIORITY; 

    if((retVal = rtl8306e_qos_portPri_set(port, int_pri)) != SUCCESS)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_portPri_get
 * Description:
 *      Get priority usage to each port
 * Input:
 *      port                  - Port id.                
 * Output:
 *      pInt_pri             -  the pointer of internal priority value
 * Return:
 *      RT_ERR_OK                        -  Success
 *      RT_ERR_FAILED                  -   Failure
 *      RT_ERR_PORT_ID                -   Error port id
 *      RT_ERR_NULL_POINTER        -   null pointer
 * Note:
 *      The API can get priority of port assignments for queue usage and packet scheduling.
 */

rtk_api_ret_t rtk_qos_portPri_get(rtk_port_t port, rtk_pri_t *pInt_pri)
{
    rtk_api_ret_t retVal;
    
    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;        
    
    if(NULL == pInt_pri)
        return RT_ERR_NULL_POINTER;
    
    if((retVal = rtl8306e_qos_portPri_get(port, pInt_pri)) != SUCCESS)
        return retVal;

    return RT_ERR_OK;

}

/* Function Name:
 *      rtk_qos_priMap_set
 * Description:
 *      Set internal priority mapping to queue ID for different queue number
 * Input:
 *      queue_num       - Queue number usage
 *      pPri2qid            - Priority mapping to queue ID               
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                        -  Success
 *      RT_ERR_FAILED                  -   Failure
 *      RT_ERR_QUEUE_ID              -   Error queue id
 *      RT_ERR_NULL_POINTER        -   null pointer
 * Note:
 *      ASIC supports priority mapping to queue with different queue number from 1 to 4.
 *      For different queue numbers usage, ASIC supports different internal available queue IDs.
 */

rtk_api_ret_t rtk_qos_priMap_set(rtk_queue_num_t queue_num, rtk_qos_pri2queue_t *pPri2qid)
{
    rtk_api_ret_t retVal;
    uint32 pri;

    if((queue_num > 4) || (queue_num == 0))
        return RT_ERR_QUEUE_NUM;           
    
    if(NULL == pPri2qid)
        return RT_ERR_NULL_POINTER;

    if((retVal = rtl8306e_qos_queueNum_set(queue_num)) != SUCCESS)
        return RT_ERR_FAILED;

    for(pri = RTL8306_PRIO0; pri <= RTL8306_PRIO3; pri++)
    {
        if(pPri2qid->pri2queue[pri] > (queue_num -1)) 
            return RT_ERR_QUEUE_ID;
        if((retVal = rtl8306e_qos_priToQueMap_set(pri, pPri2qid->pri2queue[pri])) != SUCCESS)
            return RT_ERR_FAILED;
    }
            
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_qos_priMap_get
 * Description:
 *      Get priority to queue ID mapping table parameters
 * Input:
 *      queue_num       - Queue number usage
 *      pPri2qid            - Priority mapping to queue ID               
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                        -  Success
 *      RT_ERR_FAILED                  -   Failure
 *      RT_ERR_QUEUE_ID              -   Error queue id
 *      RT_ERR_NULL_POINTER        -   null pointer
 * Note:
 *      ASIC supports priority mapping to queue with different queue number from 1 to 4.
 *      For different queue numbers usage, ASIC supports different internal available queue IDs.
 */
 
rtk_api_ret_t rtk_qos_priMap_get(rtk_queue_num_t queue_num, rtk_qos_pri2queue_t *pPri2qid)
{
    rtk_api_ret_t retVal;
    uint32 pri;        

    if((queue_num > 4) || (queue_num == 0))
        return RT_ERR_QUEUE_NUM;           
    
    if(NULL == pPri2qid)
        return RT_ERR_NULL_POINTER;

    for(pri = RTL8306_PRIO0; pri <= RTL8306_PRIO3; pri++)
    {
        if((retVal = rtl8306e_qos_priToQueMap_get(pri, &(pPri2qid->pri2queue[pri]))) != SUCCESS)
            return RT_ERR_FAILED;
    }
        
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_qos_1pRemarkEnable_set
 * Description:
 *      Set 802.1P remarking ability
 * Input:
 *      port       -  port number (0~5)
 *      enabled  -  TRUE or FALSE
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK              -  Success
 *      RT_ERR_FAILED        -   Failure
 *      RT_ERR_PORT_ID      -   Invalid port id
 * Note:
 *      
 */
rtk_api_ret_t rtk_qos_1pRemarkEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    
    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;        

    if((retVal = rtl8306e_qos_1pRemarkEnable_set(port, enable)) != SUCCESS)
        return RT_ERR_FAILED;

    return RT_ERR_OK;
}
#ifndef CONFIG_ETHWAN
/* Function Name:
 *      rtk_qos_1pRemarkEnable_get
 * Description:
 *      Get 802.1P remarking ability
 * Input:
 *      port        -  port number (0~5)
 * Output:
 *      pEnabled  -  pointer of the ability status
 * Return:
 *      RT_ERR_OK                     -  Success
 *      RT_ERR_FAILED               -   Failure
 *      RT_ERR_PORT_ID             -   Invalid port id
 *      RT_ERR_NULL_POINTER     -   Input parameter is null pointer
 * Note:
 *      
 */
rtk_api_ret_t rtk_qos_1pRemarkEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    
    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;        
    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;
    if((retVal = rtl8306e_qos_1pRemarkEnable_get(port, (uint32 *)pEnable)) != SUCCESS)
        return RT_ERR_FAILED;    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_1pRemark_set
 * Description:
 *      Set 802.1P remarking priority
 * Input:
 *      int_pri        -  Packet priority(0~4)
 *      dot1p_pri    -  802.1P priority(0~7)
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                           - Success
 *      RT_ERR_FAILED                      - Failure
 *      RT_ERR_VLAN_PRIORITY          - Invalid 1p priority
 *      RT_ERR_QOS_INT_PRIORITY     - Invalid internal priority 
 * Note:
 *      switch determines packet priority, the priority souce could
 *      be port-based, 1Q-based, dscp-based, vid-based, ip address,
 *      cpu tag.
 */
rtk_api_ret_t rtk_qos_1pRemark_set(rtk_pri_t int_pri, rtk_pri_t dot1p_pri)
{
    rtk_api_ret_t retVal;

    if (int_pri > RTL8306_PRIO3 )
        return RT_ERR_QOS_INT_PRIORITY; 

    if (dot1p_pri > RTL8306_1QTAG_PRIO7)
        return RT_ERR_VLAN_PRIORITY; 

    if((retVal = rtl8306e_qos_1pRemark_set(int_pri, dot1p_pri)) != SUCCESS)
        return RT_ERR_FAILED;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_1pRemark_get
 * Description:
 *      Get 802.1P remarking priority
 * Input:
 *      int_pri        -  Packet priority(0~4)
 * Output:
 *      pDot1p_pri  -  the pointer of 802.1P priority(0~7)
 * Return:
 *      RT_ERR_OK                           -  Success
 *      RT_ERR_FAILED                      -  Failure
 *      RT_ERR_NULL_POINTER            -  Input parameter is null pointer
 *      RT_ERR_QOS_INT_PRIORITY     -  Invalid internal priority 
 * Note:
 *      switch determines packet priority, the priority souce could
 *      be port-based, 1Q-based, dscp-based, vid-based, ip address,
 *      cpu tag.
 */
rtk_api_ret_t rtk_qos_1pRemark_get(rtk_pri_t int_pri, rtk_pri_t *pDot1p_pri)
{
    rtk_api_ret_t retVal;
    
    if (int_pri > RTL8306_PRIO3)
        return RT_ERR_QOS_INT_PRIORITY; 
    if(NULL == pDot1p_pri)
        return RT_ERR_NULL_POINTER;
    
    if((retVal = rtl8306e_qos_1pRemark_get(int_pri, pDot1p_pri)) != SUCCESS)
        return RT_ERR_FAILED;
    

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_unknownMcastPktAction_set
 * Description:
 *      Set behavior of unknown multicast
 * Input:
 *      port                -   port id
 *      type               -   unknown multicast packet type
 *      mcast_action    -  unknown multicast action
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_PORT_ID              -  Invalid port id    
 *      RT_ERR_INPUT                 -  Invalid input parameter 
 * Note:
 *      When receives an unknown multicast packet, switch may trap, drop this packet
 *      The unknown multicast packet type is as following:
 *               - MCAST_IPV4
 *               - MCAST_IPV6
 *      The unknown multicast action is as following:
 *               - MCAST_ACTION_FORWARD
 *               - MCAST_ACTION_DROP
 */
rtk_api_ret_t rtk_trap_unknownMcastPktAction_set(rtk_port_t port, rtk_mcast_type_t type, rtk_trap_mcast_action_t mcast_action)
{
    rtk_api_ret_t retVal;

    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;    

    if(mcast_action > MCAST_ACTION_DROP)
        return RT_ERR_INPUT;
    
    if((MCAST_L2 == type) || (type > MCAST_IPV6 ))
        return RT_ERR_INPUT;

    if(MCAST_IPV4 == type)
    {
        retVal = rtl8306e_trap_unknownIPMcastPktAction_set(RTL8306_IPV4_MULTICAST, 
                   (MCAST_ACTION_FORWARD == mcast_action) ? RTL8306_ACT_PERMIT: RTL8306_ACT_DROP);
        if(retVal != SUCCESS)
            return RT_ERR_FAILED;
    }
    else if(MCAST_IPV6 == type)
    {
        retVal = rtl8306e_trap_unknownIPMcastPktAction_set(RTL8306_IPV6_MULTICAST, 
                   (MCAST_ACTION_FORWARD == mcast_action) ? RTL8306_ACT_PERMIT: RTL8306_ACT_DROP);
        if(retVal != SUCCESS)
            return RT_ERR_FAILED;        
    }
           
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_trap_unknownMcastPktAction_get
 * Description:
 *      Get behavior of unknown multicast
 * Input:
 *      port                  -   port id
 *      type                 -   unknown multicast packet type
 * Output:
 *      pMcast_action    -   the pointer of unknown multicast action
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_PORT_ID              -  Invalid port id    
 *      RT_ERR_NULL_POINTER      -  Input parameter is null pointer
 * Note:
 *      When receives an unknown multicast packet, switch may trap, drop this packet.
 *      The unknown multicast packet type is as following:
 *               - MCAST_IPV4
 *               - MCAST_IPV6
 *      The unknown multicast action is as following:
 *               - MCAST_ACTION_FORWARD
 *               - MCAST_ACTION_DROP
 */
rtk_api_ret_t rtk_trap_unknownMcastPktAction_get(rtk_port_t port, rtk_mcast_type_t type, rtk_trap_mcast_action_t *pMcast_action)
{
    rtk_api_ret_t retVal;
    uint32 action;

    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;    

    if((MCAST_L2 == type) || (type > MCAST_IPV6 ))
        return RT_ERR_INPUT;
    
    if(NULL == pMcast_action)
        return RT_ERR_NULL_POINTER;

    if(MCAST_IPV4 == type)
    {
        retVal = rtl8306e_trap_unknownIPMcastPktAction_get(RTL8306_IPV4_MULTICAST, &action);
        if(retVal != SUCCESS)
            return RT_ERR_FAILED;
        *pMcast_action = (action == RTL8306_ACT_PERMIT) ? MCAST_ACTION_FORWARD: MCAST_ACTION_DROP;                        
    }
    else if(MCAST_IPV6 == type)
    {
        retVal = rtl8306e_trap_unknownIPMcastPktAction_get(RTL8306_IPV6_MULTICAST, &action);
        if(retVal != SUCCESS)
            return RT_ERR_FAILED;
        *pMcast_action = (action == RTL8306_ACT_PERMIT) ? MCAST_ACTION_FORWARD: MCAST_ACTION_DROP;                        
    }
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_igmpCtrlPktAction_set
 * Description:
 *      Set IGMP/MLD trap function
 * Input:
 *      type                -   IGMP/MLD packet type
 *      igmp_action      -   IGMP/MLD action
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_INPUT                 -  Invalid input parameter
 *      RT_ERR_NOT_ALLOWED     -  Actions not allowed by the function
 * Note:
 *      This API can set both IPv4 IGMP/IPv6 MLD with/without PPPoE header trapping function.
 *      All 4 kinds of IGMP/MLD function can be set separately.
 *      The IGMP/MLD packet type is as following:
 *          - IGMP_IPV4
 *          - IGMP_MLD
 *          - IGMP_PPPOE_IPV4
 *          - IGMP_PPPOE_MLD
 *      The IGMP/MLD action is as following:
 *          - IGMP_ACTION_FORWARD
 *          - IGMP_ACTION_TRAP2CPU
 */ 
rtk_api_ret_t rtk_trap_igmpCtrlPktAction_set(rtk_igmp_type_t type, rtk_trap_igmp_action_t igmp_action)
{
    rtk_api_ret_t retVal;
    uint32 igmp_type, action;

    if(type > IGMP_PPPOE_MLD)
        return RT_ERR_INPUT;
    if(igmp_action > IGMP_ACTION_TRAP2CPU)
        return RT_ERR_NOT_ALLOWED;
    
    igmp_type = RTL8306_IGMP;
    if(IGMP_IPV4 == type)
    {
        igmp_type = RTL8306_IGMP;
    }
    else if(IGMP_MLD == type)
    {
        igmp_type = RTL8306_MLD;
    }
    else if((IGMP_PPPOE_IPV4 == type) || (IGMP_PPPOE_MLD == type))
    {
        igmp_type = RTL8306_PPPOE;
    }
    action = (igmp_action == IGMP_ACTION_FORWARD) ? RTL8306_ACT_PERMIT: RTL8306_ACT_TRAPCPU;        
    retVal = rtl8306e_trap_igmpCtrlPktAction_set(igmp_type, action);    
    if(retVal != SUCCESS)
        return RT_ERR_FAILED;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_trap_igmpCtrlPktAction_get
 * Description:
 *      Get IGMP/MLD trap function
 * Input:
 *      type                -   IGMP/MLD packet type
 * Output:
 *      pIgmp_action    -   the pointer of IGMP/MLD action
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_INPUT                 -  Invalid input parameter
 *      RT_ERR_NULL_POINTER      -  Input parameter is null pointer
 * Note:
 *      This API can get both IPv4 IGMP/IPv6 MLD with/without PPPoE header trapping function.
 *      All 4 kinds of IGMP/MLD function can be set seperately.
 *      The IGMP/MLD packet type is as following:
 *          - IGMP_IPV4
 *          - IGMP_MLD
 *          - IGMP_PPPOE_IPV4
 *          - IGMP_PPPOE_MLD
 *      The IGMP/MLD action is as following:
 *          - IGMP_ACTION_FORWARD
 *          - IGMP_ACTION_TRAP2CPU
 */
rtk_api_ret_t rtk_trap_igmpCtrlPktAction_get(rtk_igmp_type_t type, rtk_trap_igmp_action_t *pIgmp_action)
{
    rtk_api_ret_t retVal;
    uint32 igmp_type, action;

    if(type > IGMP_PPPOE_MLD)
        return RT_ERR_INPUT;
    
    if(NULL == pIgmp_action)
        return RT_ERR_NULL_POINTER;
    
    igmp_type = RTL8306_IGMP;
    if(IGMP_IPV4 == type)
    {
        igmp_type = RTL8306_IGMP;    
    }
    else if(IGMP_MLD == type)
    {
        igmp_type = RTL8306_MLD;
    }
    else if((IGMP_PPPOE_IPV4 == type) || (IGMP_PPPOE_MLD == type))
    {
        igmp_type = RTL8306_PPPOE;
    }
    retVal = rtl8306e_trap_igmpCtrlPktAction_get(igmp_type, &action);
    if(retVal != SUCCESS)
        return RT_ERR_FAILED;
    *pIgmp_action = (action == RTL8306_ACT_PERMIT) ? IGMP_ACTION_FORWARD: IGMP_ACTION_TRAP2CPU;
    
    return RT_ERR_OK;
}
#endif

/* Function Name:
 *      rtk_vlan_init
 * Description:
 *      Initialize VLAN
 * Input:
 *      none
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                     -  Success
 *      RT_ERR_FAILED                -  Failure
 * Note:
 *      VLAN is disabled by default. User has to call this API to enable VLAN before
 *      using it. And It will set a default VLAN(vid 1) including all ports and set 
 *      all ports PVID to the default VLAN.
 */

rtk_api_ret_t rtk_vlan_init(void)
{
    uint32 i;
    uint32 port;


    /*clear vlan table*/
    for(i = 0; i < 16; i++)
        rtl8306e_vlan_entry_set(i, 0, 0, 0);

    /*set switch default configuration:
      *enable tag aware, disable ingress filter,
      *admit all packet*/
    rtl8306e_vlan_tagAware_set(TRUE);     
    rtl8306e_vlan_IgrFilterEnable_set(FALSE);  
    for (port = 0; port < 6; port++)
    {
        rtl8306e_vlan_portAcceptFrameType_set(port, RTL8306E_ACCEPT_ALL);
    }

    /*add a default vlan 1 which contains all ports*/
    rtl8306e_vlan_entry_set(0, 1, 0x3F, 0);
    
    /*set all ports' vid to vlan 1*/
    for(port = 0; port < 6; port++)
        rtl8306e_vlan_portPvidIndex_set(port, 0);

    /*set vlan enabled*/
    rtl8306e_regbit_set(0, 18, 8, 0, 0);
#ifdef RTL8306_TBLBAK
    rtl8306_TblBak.vlanConfig.enVlan = TRUE; 
#endif
    /*disable trunk*/
    rtl8306e_regbit_set(0, 19, 11, 0, 1);
    
    return RT_ERR_OK;
   
}

/* Function Name:
 *      rtk_vlan_set
 * Description:
 *      Set a VLAN entry
 * Input:
 *      vid              - VLAN ID to configure, should be 1~4094
 *      mbrmsk        - VLAN member set portmask
 *      untagmsk     - VLAN untag set portmask
 *      fid              -  filtering database id, could be any value for RTL8306E
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                     -  Success
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_VLAN_VID            -  Invalid vid
 *      RT_ERR_INPUT                 -  Invalid input parameter 
 *      RT_ERR_TBL_FULL            -  Input table full 
 * Note:
 *     There are 16 VLAN entry supported. User could configure the member set and untag set
 *     for specified vid through this API. The portmask's bit N means port N.
 *     For example, mbrmask 23=0x17=010111 means port 0,1,2,4 in the member set.
 *     FID is for SVL/IVL usage, and the range is 0~4095, rtl8306E only supports SVL, 
 *     so fid is no useage.
 */
rtk_api_ret_t rtk_vlan_set(rtk_vlan_t vid, rtk_portmask_t mbrmsk, rtk_portmask_t untagmsk, rtk_fid_t fid)
{
    uint32 vid_val, mbrmsk_val, untagmsk_val;
    uint32 index, hit_index, hit_flag;
    uint32 fullflag;
    int32 i;

    /* vid must be 1~4094 */
    if ((vid == 0) || (vid > (RTL8306_VIDMAX - 1)))
        return RT_ERR_VLAN_VID;

    if (mbrmsk.bits[0] > RTL8306_MAX_PORTMASK)
        return RT_ERR_INPUT;

    if (untagmsk.bits[0] > RTL8306_MAX_PORTMASK)
        return RT_ERR_INPUT;


    /*check if vid exists and check if vlan is full*/
    fullflag = TRUE;
    hit_flag = FALSE;
    index = 16;
    hit_index= 16;
    for(i = 15; i >= 0; i--)
    {
        rtl8306e_vlan_entry_get((uint32)i, &vid_val, &mbrmsk_val, &untagmsk_val);
        if (0 == vid_val) 
        {
            index = (uint32)i; 
            fullflag = FALSE;
            continue;
        }
        
        if (vid_val == vid)
        {
            hit_flag = TRUE;
            hit_index = (uint32)i;
            fullflag = FALSE;
            break;
        }            
    }        
    
    if (fullflag)       
        return RT_ERR_TBL_FULL;
    else 
    {
        vid_val           = (uint32)vid;
        mbrmsk_val     = mbrmsk.bits[0];
        untagmsk_val  = untagmsk.bits[0];

        /*both mbrmsk_val and untagmsk_val are zero will clear the vlan*/
        if( (0 == mbrmsk_val) && (0 == untagmsk_val))
            vid_val = 0;
            
        if (hit_flag)
            rtl8306e_vlan_entry_set(hit_index, vid_val, mbrmsk_val, untagmsk_val);            
        else
            rtl8306e_vlan_entry_set(index, vid_val, mbrmsk_val, untagmsk_val);            
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_get
 * Description:
 *      Get a VLAN entry
 * Input:
 *      vid             - VLAN ID to configure
 * Output:
 *      pMbrmsk     - VLAN member set portmask
 *      pUntagmsk  - VLAN untag set portmask
 *      pFid           -  filtering database id
 * Return:
 *      RT_ERR_OK                                   -  Success
 *      RT_ERR_FAILED                              -  Failure
 *      RT_ERR_VLAN_VID                          -  Invalid vid
 *      RT_ERR_NULL_POINTER                    -  Input parameter is null pointer
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND    -   Specified vlan entry not found
 * Note:
 *     There are 16 VLAN entry supported. User could configure the member set and untag set
 *     for specified vid through this API. The portmask's bit N means port N.
 *     For example, mbrmask 23=0x17=010111 means port 0,1,2,4 in the member set.
 *     FID is for SVL/IVL usage, and the range is 0~4095, rtl8306E only supports SVL, 
 *     so fid is no useage.
 */
 
rtk_api_ret_t rtk_vlan_get(rtk_vlan_t vid, rtk_portmask_t *pMbrmsk, rtk_portmask_t *pUntagmsk, rtk_fid_t *pFid)
{
    uint32 vid_val, mbrmsk_val, untagmsk_val;
    uint32 hit_flag;
    int32 i;

    /* vid must be 1~4094 */
    if ((vid == 0) || (vid > (RTL8306_VIDMAX - 1)))
        return RT_ERR_VLAN_VID;

    if ((NULL == pMbrmsk) || (NULL == pUntagmsk))
        return RT_ERR_NULL_POINTER;

    /*seach the vlan table*/
    hit_flag = FALSE;
    for (i = 15; i >= 0; i--)
    {
        rtl8306e_vlan_entry_get((rtk_vlan_t)i, &vid_val, &mbrmsk_val, &untagmsk_val);
        if(vid_val == vid)
        {
            hit_flag = TRUE;
            pMbrmsk->bits[0]    = mbrmsk_val;
            pUntagmsk->bits[0] = untagmsk_val;         
            *pFid = 0;
            return RT_ERR_OK;
        }
    }

    if(!hit_flag)
        return RT_ERR_VLAN_ENTRY_NOT_FOUND;
        
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_vlan_destroy
 * Description:
 *      delete vid from vlan table
 * Input:
 *      vid             - VLAN ID to configure
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                   -  Success
 *      RT_ERR_FAILED                              -  Failure
 *      RT_ERR_VLAN_VID                          -  Invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND    -  Specified vlan entry not found
 * Note:
 * 
 */
rtk_api_ret_t rtk_vlan_destroy(rtk_vlan_t vid)
{
    uint32 vid_val, mbrmsk_val, untagmsk_val;
    uint32 hit_flag;
    int32 i;
    
    /* vid must be 1~4094 */
    if ((vid == 0) || (vid > (RTL8306_VIDMAX - 1)))
        return RT_ERR_VLAN_VID;

    hit_flag = FALSE;
    for (i = 15; i >= 0; i--)
    {
        rtl8306e_vlan_entry_get((uint32)i, &vid_val, &mbrmsk_val, &untagmsk_val);
        if(vid_val == vid)
        {
            hit_flag = TRUE;
            vid_val = 0;
            mbrmsk_val = 0;
            untagmsk_val = 0;
            rtl8306e_vlan_entry_set((uint32)i, vid_val, mbrmsk_val, untagmsk_val);
            return RT_ERR_OK;
        }        
    }

    if(!hit_flag)
        return RT_ERR_VLAN_ENTRY_NOT_FOUND;    
    else
        return RT_ERR_OK;        

}
    

/* Function Name:
 *      rtk_vlan_portPvid_set
 * Description:
 *      Set port to specified VLAN ID(PVID)
 * Input:
 *      port             - Port id
 *      pvid             - Specified VLAN ID
 *      priority         - 802.1p priority for the PVID, 0~3 for RTL8306E 
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                   -  Success
 *      RT_ERR_FAILED                              -  Failure
 *      RT_ERR_VLAN_VID                          -  Invalid vid
 *      RT_ERR_VLAN_PRIORITY                  -  Invalid 1p priority 
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND    -  Specified vlan entry not found
 * Note:
 *      The API is used for Port-based VLAN. The untagged frame received from the
 *      port will be classified to the specified VLAN and assigned to the specified priority.
 */
rtk_api_ret_t rtk_vlan_portPvid_set(rtk_port_t port, rtk_vlan_t pvid, rtk_pri_t priority)
{
    uint32 vid_val, mbrmsk_val, untagmsk_val;
    uint32 hit_flag;
    uint32 regVal;
    int32 i;

    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;    

    if ((pvid == 0) || (pvid > (RTL8306_VIDMAX - 1)))
        return RT_ERR_VLAN_VID;

    if (priority > RTL8306_PRIO3)
        return RT_ERR_VLAN_PRIORITY;

    /*seach the vlan table*/
    hit_flag = FALSE;
    for (i = 15; i >= 0; i--)
    {
        rtl8306e_vlan_entry_get((uint32)i, &vid_val, &mbrmsk_val, &untagmsk_val);
        if(vid_val == pvid)
        {
            hit_flag = TRUE;
            rtl8306e_vlan_portPvidIndex_set(port, (uint32)i);
            
            /*802.1Q default priority for untag pkt*/
            if (RTL8306_PORT5 == port)  
                port ++ ;  
            rtl8306e_reg_get(port, 17, 2, &regVal);
            regVal = (regVal & 0x9FFF) | (priority << 13);
            rtl8306e_reg_set(port, 17, 2, regVal);            
            return RT_ERR_OK;
        }
    }

    if (!hit_flag)
        return RT_ERR_VLAN_ENTRY_NOT_FOUND;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_portPvid_get
 * Description:
 *      Get VLAN ID(PVID) on specified port
 * Input:
 *      port             - Port id
 * Output:
 *      pPvid            - Specified VLAN ID
 *      pPriority        - 802.1p priority for the PVID
 * Return:
 *      RT_ERR_OK                                   - Success
 *      RT_ERR_FAILED                             -  Failure
 *      RT_ERR_PORT_ID                           -  Invalid port id
 *      RT_ERR_NULL_POINTER                   -  Input parameter is null pointer
 * Note:
 *    The API is used for Port-based VLAN. The untagged frame received from the
 *    port will be classified to the specified VLAN and assigned to the specified priority.
 */

rtk_api_ret_t rtk_vlan_portPvid_get(rtk_port_t port, rtk_vlan_t *pPvid, rtk_pri_t *pPriority)
{
    rtk_api_ret_t retVal;    
    uint32 vid_val, mbrmsk_val, untagmsk_val;
    uint32 index;
    uint32 regVal;

    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;    

    if((NULL == pPvid) || (NULL == pPriority))
        return RT_ERR_NULL_POINTER;

    retVal = rtl8306e_vlan_portPvidIndex_get(port, &index);
    if (retVal != SUCCESS)
        return RT_ERR_FAILED;
    
    retVal = rtl8306e_vlan_entry_get(index, &vid_val, &mbrmsk_val, &untagmsk_val);    
    if (retVal != SUCCESS)
        return RT_ERR_FAILED;
    *pPvid = vid_val;

    /*get 802.1Q default priority for untag pkt*/
    if (RTL8306_PORT5 == port)  
        port ++ ;  
    rtl8306e_reg_get(port, 17, 2, &regVal);
    *pPriority = (regVal & 0x6000) >> 13;    

    return RT_ERR_OK;
}

#ifndef CONFIG_ETHWAN
/* Function Name:
 *      rtk_vlan_portIgrFilterEnable_set
 * Description:
 *      Set VLAN ingress for each port
 * Input:
 *      port             - Port id
 *      igr_filter        - VLAN ingress function enable status
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                        - Success
 *      RT_ERR_FAILED                  -  Failure
 * Note:
 *      RTL8306E use one ingress filter for whole system, not for each port, so 
 *      any port you set will affect all ports ingress filter setting.
 *      While VLAN function is enabled, ASIC will decide VLAN ID for each received frame 
 *      and get belonged member ports from VLAN table. If received port is not belonged 
 *      to VLAN member ports, ASIC will drop received frame if VLAN ingress function is enabled.
 */
rtk_api_ret_t rtk_vlan_portIgrFilterEnable_set(rtk_port_t port, rtk_enable_t igr_filter)
{
    rtk_api_ret_t retVal;    
    
    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;    

    retVal = rtl8306e_vlan_IgrFilterEnable_set(igr_filter);
    if (retVal != SUCCESS)
        return RT_ERR_FAILED;
            
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_portIgrFilterEnable_get
 * Description:
 *      get VLAN ingress for each port
 * Input:
 *      port             - Port id
 * Output:
 *      pIgr_filter     - the pointer of VLAN ingress function enable status
 * Return:
 *      RT_ERR_OK                 - Success
 *      RT_ERR_FAILED           -  Failure
 * Note:
 *      RTL8306E use one ingress filter for whole system, not for each port, so 
 *      any port you set will affect all ports ingress filter setting.
 *      While VLAN function is enabled, ASIC will decide VLAN ID for each received frame 
 *      and get belonged member ports from VLAN table. If received port is not belonged 
 *      to VLAN member ports, ASIC will drop received frame if VLAN ingress function is enabled.
 */
rtk_api_ret_t rtk_vlan_portIgrFilterEnable_get(rtk_port_t port, rtk_enable_t *pIgr_filter)
{
    rtk_api_ret_t retVal;    
    uint32 enabled;
    
    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;    
    
    if (NULL == pIgr_filter)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8306e_vlan_IgrFilterEnable_get(&enabled);
    if (retVal != SUCCESS)
        return RT_ERR_FAILED;
    *pIgr_filter = enabled ? ENABLED : DISABLED;
        
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_set
 * Description:
 *      Set VLAN support frame type
 * Input:
 *      port                                 - Port id
 *      accept_frame_type             - accept frame type
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                  - Success
 *      RT_ERR_FAILED            -  Failure
 *      RT_ERR_PORT_ID          -  Invalid port id
 * Note:
 *      The API is used for checking 802.1Q tagged frames.
 *      The accept frame type as following:
 *          - ACCEPT_FRAME_TYPE_ALL
 *          - ACCEPT_FRAME_TYPE_TAG_ONLY
 *          - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
rtk_api_ret_t rtk_vlan_portAcceptFrameType_set(rtk_port_t port, rtk_vlan_acceptFrameType_t accept_frame_type)
{
    rtk_api_ret_t retVal;
    rtl8306e_acceptFrameType_t accfrm_type;    

    if ( port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    switch (accept_frame_type)
    {
        case ACCEPT_FRAME_TYPE_ALL:
            accfrm_type = RTL8306E_ACCEPT_ALL;
            break;
        case ACCEPT_FRAME_TYPE_TAG_ONLY:
            accfrm_type = RTL8306E_ACCEPT_TAG_ONLY;
            break;
        case ACCEPT_FRAME_TYPE_UNTAG_ONLY:
            accfrm_type = RTL8306E_ACCEPT_UNTAG_ONLY;
            break;
        default:
            return RT_ERR_VLAN_ACCEPT_FRAME_TYPE;
    }

    retVal = rtl8306e_vlan_portAcceptFrameType_set(port, accfrm_type);
    
    return retVal;
}

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_get
 * Description:
 *      Get VLAN support frame type
 * Input:
 *      port                                 - Port id
 *      accept_frame_type             - accept frame type
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                   - Success
 *      RT_ERR_FAILED                             -  Failure
 *      RT_ERR_PORT_ID                           -  Invalid port id
 *      RT_ERR_VLAN_ACCEPT_FRAME_TYPE -  Invalid accept frame type 
 * Note:
 *    The API is used for checking 802.1Q tagged frames.
 *    The accept frame type as following:
 *    - ACCEPT_FRAME_TYPE_ALL
 *    - ACCEPT_FRAME_TYPE_TAG_ONLY
 *    - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
 
rtk_api_ret_t rtk_vlan_portAcceptFrameType_get(rtk_port_t port, rtk_vlan_acceptFrameType_t *pAccept_frame_type)
{
    rtk_api_ret_t retVal;
    rtl8306e_acceptFrameType_t accfrm_type;    

    if ( port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if (NULL == pAccept_frame_type)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8306e_vlan_portAcceptFrameType_get(port, &accfrm_type);
    switch(accfrm_type)
    {
        case RTL8306E_ACCEPT_ALL:
            *pAccept_frame_type = ACCEPT_FRAME_TYPE_ALL;
            break;
        case RTL8306E_ACCEPT_TAG_ONLY:
            *pAccept_frame_type = ACCEPT_FRAME_TYPE_TAG_ONLY;
            break;
        case RTL8306E_ACCEPT_UNTAG_ONLY:
            *pAccept_frame_type = ACCEPT_FRAME_TYPE_UNTAG_ONLY;
            break;
        default:
            return RT_ERR_VLAN_ACCEPT_FRAME_TYPE;
    }

    return retVal;
}    


/* Function Name:
 *      rtk_vlan_vlanBasedPriority_set
 * Description:
 *      Set VLAN priority for each CVLAN
 * Input:
 *      vid                -Specified VLAN ID
 *      priority           -priority for the VID
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                -  Success
 *      RT_ERR_FAILED                          -   Failure
 *      RT_ERR_VLAN_VID                       -  Invalid vid 
 *      RT_ERR_VLAN_PRIORITY               -  Invalid 1p priority
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND -  Specified vlan entry not found
 * Note:
 *      This API is used to set priority per VLAN.
 */

rtk_api_ret_t rtk_vlan_vlanBasedPriority_set(rtk_vlan_t vid, rtk_pri_t priority)
{
    uint32 vid_val, mbrmsk_val, untagmsk_val;
    uint32 hit_flag;
    int32 i;

    /* vid must be 1~4094 */
    if ((vid == 0) || (vid > (RTL8306_VIDMAX - 1)))
        return RT_ERR_VLAN_VID;

    if (priority > RTL8306_PRIO3)
        return RT_ERR_VLAN_PRIORITY;    

    /*seach the vlan table*/
    hit_flag = FALSE;
    for (i = 15; i >= 0; i--)
    {
        rtl8306e_vlan_entry_get((uint32)i, &vid_val, &mbrmsk_val, &untagmsk_val);
        if(vid_val == vid)
        {
            hit_flag = TRUE;
            rtl8306e_vlan_vlanBasedPriority_set((uint32)i, priority);            
            return RT_ERR_OK;
        }
    }

    if (!hit_flag)
        return RT_ERR_VLAN_ENTRY_NOT_FOUND;
        
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_vlan_vlanBasedPriority_get
 * Description:
 *      Get VLAN priority for each CVLAN
 * Input:
 *      vid                -Specified VLAN ID
 * Output:
 *      pPriority         -the pointer of priority for the VID
 * Return:
 *      RT_ERR_OK                                -  Success
 *      RT_ERR_FAILED                          -   Failure
 *      RT_ERR_VLAN_VID                       -  Invalid vid 
 *      RT_ERR_NULL_POINTER                -   Input parameter is null pointer
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND -   Specified vlan entry not found 
 * Note:
 *      This API is used to set priority per VLAN.
 */

rtk_api_ret_t rtk_vlan_vlanBasedPriority_get(rtk_vlan_t vid, rtk_pri_t *pPriority)
{
    uint32 vid_val, mbrmsk_val, untagmsk_val;
    uint32 hit_flag;
    int32 i;

    /* vid must be 1~4094 */
    if ((vid == 0) || (vid > (RTL8306_VIDMAX - 1)))
        return RT_ERR_VLAN_VID;

    if (NULL == pPriority)
        return RT_ERR_NULL_POINTER;

    /*seach the vlan table*/
    hit_flag = FALSE;
    for (i = 15; i >= 0; i--)
    {
        rtl8306e_vlan_entry_get((uint32)i, &vid_val, &mbrmsk_val, &untagmsk_val);
        if(vid_val == vid)
        {
            hit_flag = TRUE;
            rtl8306e_vlan_vlanBasedPriority_get((uint32)i, pPriority);            
            return RT_ERR_OK;
        }
    }

    if (!hit_flag)
        return RT_ERR_VLAN_ENTRY_NOT_FOUND;
    
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_vlan_vidTranslation_set
 * Description:
 *      Set vid translated to new vid
 * Input:
 *      vid       -  old vid
 *      nvid     -   new vid
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                -  Success
 *      RT_ERR_FAILED                          -   Failure
 *      RT_ERR_VLAN_VID                       -  Invalid vid 
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND -   Specified vlan entry not found 
 * Note:
 *      This API is used to translate a vid to a new vid, the new vid could be 
 *      used by Q-in-Q or vlan translation function.
 */
rtk_api_ret_t rtk_vlan_vidTranslation_set(rtk_vlan_t vid, rtk_vlan_t nvid)
{
    uint32 vid_val, mbrmsk_val, untagmsk_val;
    uint32 hit_flag;
    int32 i;
    
    if ((vid > RTL8306_VIDMAX ) || (nvid > RTL8306_VIDMAX ))
        return RT_ERR_VLAN_VID;

    hit_flag = FALSE;
    for (i = 15; i >= 0; i--)
    {
        rtl8306e_vlan_entry_get((uint32)i, &vid_val, &mbrmsk_val, &untagmsk_val);
        if(vid_val == vid)
        {
            hit_flag = TRUE;
            rtl8306e_vlan_transVid_set((uint32)i, nvid);
            return RT_ERR_OK;
        }        
    }

    if(!hit_flag)
        return RT_ERR_VLAN_ENTRY_NOT_FOUND;            

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_vidTranslation_get
 * Description:
 *      Get vid translation new vid
 * Input:
 *      vid        -  old vid
 * Output:
 *      pNvid     -  the pointer of new vid
 * Return:
 *      RT_ERR_OK                                -  Success
 *      RT_ERR_FAILED                          -   Failure
 *      RT_ERR_VLAN_VID                       -  Invalid vid 
 *      RT_ERR_NULL_POINTER                -   NULL pointer
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND -   Specified vlan entry not found 
 * Note:
 *      This API is used to translate a vid to a new vid, the new vid could be 
 *      used by Q-in-Q or vlan translation function.
 */
rtk_api_ret_t rtk_vlan_vidTranslation_get(rtk_vlan_t vid, rtk_vlan_t *pNvid)
{

    uint32 vid_val, mbrmsk_val, untagmsk_val;
    uint32 hit_flag;
    int32 i;

    if (vid > RTL8306_VIDMAX )
        return RT_ERR_VLAN_VID;
    
    if (NULL == pNvid)
        return RT_ERR_NULL_POINTER;

    hit_flag = FALSE;
    for (i = 15; i >= 0; i--)
    {
        rtl8306e_vlan_entry_get((uint32)i, &vid_val, &mbrmsk_val, &untagmsk_val);
        if(vid_val == vid)
        {
            hit_flag = TRUE;
            rtl8306e_vlan_transVid_get((uint32)i, pNvid);
            return RT_ERR_OK;
        }        
    }

    if(!hit_flag)
        return RT_ERR_VLAN_ENTRY_NOT_FOUND;            
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_vidTranslationEnable_set
 * Description:
 *      Set vlan translation function enabled or disabled 
 * Input:
 *      enable        -  enable or disable
 *      nniMask      -   NNI port mask
 * Output:
 *      pNvid     -  the pointer of new vid
 * Return:
 *      RT_ERR_OK                                -  Success
 *      RT_ERR_FAILED                          -   Failure
 *      RT_ERR_PORT_MASK                   -   Error port mask
 * Note:
 *      VLAN translation only happens between UNI and NNI port, 
 *      in nniMask, 1 means the port is NNI port, 0 means the port
 *      is UNI port
 */
rtk_api_ret_t rtk_vlan_vidTranslationEnable_set(rtk_enable_t enable, rtk_portmask_t nniMask)
{
    rtk_api_ret_t retVal;
    uint32 port;
    
    if (nniMask.bits[0] > RTL8306_MAX_PORTMASK)
        return RT_ERR_PORT_MASK; 

    /*set default vid and priority src*/
    if(enable)
    {
        for (port = 0; port < 6; port ++)
            rtl8306e_svlan_otagSrc_set(port, RTL8306E_VIDSRC_NVID, RTL8306E_PRISRC_1PRMK);
    }

    retVal = rtl8306e_vlan_transEnable_set((uint32) enable, nniMask.bits[0]);
    
    return retVal;

}


/* Function Name:
 *      rtk_vlan_vidTranslationEnable_get
 * Description:
 *      Get vlan translation function enabled or disabled 
 * Input:
 *      none
 * Output:
 *      pEnable      -   the pointer of enable or disable
 *      pNniMask    -   the pointer of NNI port mask
 * Return:
 *      RT_ERR_OK                                -  Success
 *      RT_ERR_FAILED                          -   Failure
 *      RT_ERR_NULL_POINTER                -   NULL pointer
 * Note:
 *      VLAN translation only happens between UNI and NNI port, 
 *      in nniMask, 1 means the port is NNI port, 0 means the port
 *      is UNI port
 */

rtk_api_ret_t rtk_vlan_vidTranslationEnable_get(rtk_enable_t *pEnable, rtk_portmask_t *pNniMask)
{
    rtk_api_ret_t retVal;
    uint32 enable, portmask;
    
    if((NULL == pEnable) || (NULL == pNniMask))
        return RT_ERR_NULL_POINTER;

    retVal = rtl8306e_vlan_transEnable_get(&enable, &portmask);
    *pEnable = enable ? ENABLED : DISABLED;
    pNniMask->bits[0] = portmask;

    return retVal;

}

/* Function Name:
 *      rtk_vlan_tagSrc_set
 * Description:
 *      Set tag vid and priority source for Q-in-Q and VLAN translation
 * Input:
 *      port          -    port id
 *      vidSrc       -    vid source
 *      priSrc        -    priority source
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                 -  Success
 *      RT_ERR_FAILED            -   Failure
 *      RT_ERR_PORT_ID          -   error port id
 * Note:
 *      Q-in-Q(SVLAN) and VLAN translation will modify tag, 
 *      the API could set outer tag or translated VLAN tag
 *      vid/priority source.
 *      - vidSrc: 
 *                - RTL8306E_VIDSRC_POVID - port-based otag vid,     
 *                - RTL8306E_VIDSRC_NVID   - new vid(translated vid)
 *       - priSrc:
 *                - RTL8306E_PRISRC_PPRI    - port-based otag priority, 
 *                - RTL8306E_PRISRC_1PRMK - 1p remarking priority
 */
rtk_api_ret_t rtk_vlan_tagSrc_set(rtk_port_t port, rtk_vidSrc_t vidSrc, rtk_priSrc_t priSrc)    
{
    rtk_api_ret_t retVal;

    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;    

    retVal = rtl8306e_svlan_otagSrc_set(port, vidSrc, priSrc);
    
    return retVal;
}


/* Function Name:
 *      rtk_vlan_tagSrc_get
 * Description:
 *      Get tag vid and priority source for Q-in-Q and VLAN translation
 * Input:
 *      port          -    port id
 * Output:
 *      pVidSrc     -    the pointer of vid source
 *      pPriSrc      -    the pointer of priority source
 * Return:
 *      RT_ERR_OK                 -  Success
 *      RT_ERR_FAILED            -   Failure
 *      RT_ERR_PORT_ID          -   error port id
 * Note:
 *      Q-in-Q(SVLAN) and VLAN translation will modify tag, 
 *      the API could set outer tag or tranlated VLAN tag
 *      vid/priority source.
 *      vidSrc: 
 *                RTL8306E_VIDSRC_POVID - port-based otag vid,     
 *                RTL8306E_VIDSRC_NVID   - new vid(translated vid)
 *       priSrc:
 *                RTL8306E_PRISRC_PPRI    - port-based otag priority, 
 *                RTL8306E_PRISRC_1PRMK - 1p remarking priority
 */
rtk_api_ret_t rtk_vlan_tagSrc_get(rtk_port_t port, rtk_vidSrc_t *pVidSrc, rtk_priSrc_t *pPriSrc)    
{
    rtk_api_ret_t retVal;
    uint32 vidsrc, prisrc;

    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;    
    if((NULL == pVidSrc) || (NULL == pPriSrc))
        return RT_ERR_NULL_POINTER;
    
    retVal = rtl8306e_svlan_otagSrc_get(port, &vidsrc, &prisrc);
    *pVidSrc = (rtk_vidSrc_t)vidsrc;
    *pPriSrc  = (rtk_priSrc_t)prisrc;
    
    return retVal;
}


/* Function Name:
 *      rtk_stp_mstpState_set
 * Description:
 *      Configure spanning tree state per port
 * Input:
 *      msti              - Multiple spanning tree instance, no use for RTL8306E
 *      port              - Port id
 *      stp_state       - Spanning tree state
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_PORT_ID              -  Invalid port id
 *      RT_ERR_MSTP_STATE        -  Invalid spanning tree status
 * Note:
 *      Because RTL8306E does not support multiple spanning tree, so msti is no use. 
 *      There are four states supported by ASIC.
 *          - STP_STATE_DISABLED
 *          - STP_STATE_BLOCKING
 *          - STP_STATE_LEARNING
 *          - STP_STATE_FORWARDING
 */
rtk_api_ret_t rtk_stp_mstpState_set(rtk_stp_msti_id_t msti, rtk_port_t port, rtk_stp_state_t stp_state)
{
    rtk_api_ret_t retVal;
    
    if(port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;
    
    if(stp_state >= STP_STATE_END)
        return RT_ERR_MSTP_STATE;

    retVal = rtl8306e_stp_set(port, stp_state);
    return retVal;

}    

/* Function Name:
 *      rtk_stp_mstpState_get
 * Description:
 *      Get Configuration of spanning tree state per port
 * Input:
 *      msti              - Multiple spanning tree instance, no use for RTL8306E
 *      port              - Port id
 * Output:
 *      pStp_state     - the pointer of Spanning tree state
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_PORT_ID              -  Invalid port id
 *      RT_ERR_NULL_POINTER      -  Input parameter is null pointer
 * Note:
 *      Because RTL8306E does not support multiple spanning tree, so msti is no use. 
 *      There are four states supported by ASIC.
 *          - STP_STATE_DISABLED
 *          - STP_STATE_BLOCKING
 *          - STP_STATE_LEARNING
 *          - STP_STATE_FORWARDING
 */
rtk_api_ret_t rtk_stp_mstpState_get(rtk_stp_msti_id_t msti, rtk_port_t port, rtk_stp_state_t *pStp_state)
{
    rtk_api_ret_t retVal;
    uint32 state;
    
    if(port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;
    
    if(NULL == pStp_state)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8306e_stp_get(port, &state);
    *pStp_state = (rtk_stp_state_t)state;

    return retVal;
}    

/* Function Name:
 *      rtk_l2_addr_add
 * Description:
 *      Set LUT unicast entry
 * Input:
 *      pMac               -   6 bytes unicast(I/G bit is 0) mac address to be written into LUT
 *      pL2_data          -   the mac address attributes
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                       -  Success
 *      RT_ERR_FAILED                 -   Failure
 *      RT_ERR_INPUT                  -   Invalid input parameter
 *      RT_ERR_MAC                    -   Wrong mac address, must be unicast mac        
 *      RT_ERR_NULL_POINTER       -  Input parameter is null pointer    
 *      RT_ERR_L2_INDEXTBL_FULL -  The L2 index table is full
 * Note:
 *      If the unicast mac address already existed in LUT, it will udpate the status of the entry. 
 *      Otherwise, it will find an empty or asic auto learned entry to write. If all the entries 
 *      with the same hash value can't be replaced, ASIC will return a RT_ERR_L2_INDEXTBL_FULL error.
 *      for RTL8306E, pL2_data member fid and sa_block is no use, so it can be chosen any value.
 */
 
rtk_api_ret_t rtk_l2_addr_add(rtk_mac_t *pMac, rtk_l2_ucastAddr_t *pL2_data)
{
    rtk_api_ret_t retVal;
    uint32 entryAddr;
        
    /* must be unicast address */
    if((NULL == pMac) || (pMac->octet[0] & 0x1))
        return RT_ERR_MAC;
    
    if(NULL == pL2_data)
        return RT_ERR_NULL_POINTER;

    if(pL2_data->port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;    

    if(pL2_data->is_static >= RTK_ENABLE_END)
        return RT_ERR_INPUT; 

    if(pL2_data->auth >= RTK_ENABLE_END)
        return RT_ERR_INPUT; 
        
    retVal = rtl8306e_l2_unicastMac_add(pMac->octet, RTL8306_LUT_AGE300, pL2_data->is_static, pL2_data->auth, pL2_data->port, &entryAddr);
    if(RTL8306_LUT_FULL == retVal)
        return RT_ERR_L2_INDEXTBL_FULL;
    else if(retVal != SUCCESS)
        return RT_ERR_FAILED;
    
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_l2_addr_get
 * Description:
 *      Get LUT unicast entry
 * Input:
 *      pMac               -   6 bytes unicast(I/G bit is 0) mac address to be gotten
 *      fid                   -   filtering database id, could be any value for RTL8306E switch
 * Output:
 *      pL2_data          -   the mac address attributes
 * Return:
 *      RT_ERR_OK                               -  Success
 *      RT_ERR_FAILED                         -   Failure
 *      RT_ERR_INPUT                          -   Invalid input parameter
 *      RT_ERR_MAC                            -   Wrong mac address, must be unicast mac        
 *      RT_ERR_NULL_POINTER              -   Input parameter is null pointer    
 *      RT_ERR_L2_ENTRY_NOTFOUND    -   Specified entry not found
 * Note:
 *      If the unicast mac address existed in LUT, it will return the port where
 *      the mac is learned, 802.1x authorized status and dynamic/static entry,
 *      Otherwise, it will return a RT_ERR_L2_ENTRY_NOTFOUND error.
 */

rtk_api_ret_t rtk_l2_addr_get(rtk_mac_t *pMac, rtk_fid_t fid, rtk_l2_ucastAddr_t *pL2_data)
{
    rtk_api_ret_t retVal;
    uint32 isStatic, isAuth, port, entryaddr;    

    /* must be unicast address */
    if((NULL == pMac) || (pMac->octet[0] & 0x1))
        return RT_ERR_MAC;
    
    if(NULL == pL2_data)
        return RT_ERR_NULL_POINTER;

    pL2_data->auth = FALSE;    
    pL2_data->fid = 1;
    pL2_data->is_static = FALSE;
    pL2_data->port = 0;
    pL2_data->sa_block = FALSE;

    retVal = rtl8306e_l2_mac_get(pMac->octet, &isStatic, &isAuth, &port, &entryaddr);
    if(RTL8306_LUT_NOTEXIST == retVal)
    {        
        return RT_ERR_L2_ENTRY_NOTFOUND;        
    }
    else if(retVal != SUCCESS)
    {
        return RT_ERR_FAILED;
    }

    pL2_data->auth = isAuth;
    pL2_data->port = port;
    pL2_data->is_static = isStatic;
    pL2_data->mac.octet[0] = pMac->octet[0];
    pL2_data->mac.octet[1] = pMac->octet[1];
    pL2_data->mac.octet[2] = pMac->octet[2];
    pL2_data->mac.octet[3] = pMac->octet[3];
    pL2_data->mac.octet[4] = pMac->octet[4];
    pL2_data->mac.octet[5] = pMac->octet[5];    
    
    return RT_ERR_OK;
    
}


/* Function Name:
 *      rtk_l2_addr_del
 * Description:
 *      Delete LUT unicast entry
 * Input:
 *      pMac               -   6 bytes unicast mac address to be deleted
 *      fid                   -   filtering database id, could be any value for RTL8306E switch
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                               -  Success
 *      RT_ERR_FAILED                         -   Failure
 *      RT_ERR_INPUT                          -   Invalid input parameter
 *      RT_ERR_MAC                            -   Wrong mac address, must be unicast mac        
 *      RT_ERR_NULL_POINTER              -   Input parameter is null pointer    
 *      RT_ERR_L2_ENTRY_NOTFOUND    -   Specified entry not found 
 * Note:
 *      If the mac has existed in the LUT, it will be deleted.
 *      Otherwise, it will return RT_ERR_L2_ENTRY_NOTFOUND.
 */
rtk_api_ret_t rtk_l2_addr_del(rtk_mac_t *pMac, rtk_fid_t fid)
{
    rtk_api_ret_t retVal;
    uint32 entryAddr;

    /* must be unicast address */
    if((NULL == pMac) || (pMac->octet[0] & 0x1))
        return RT_ERR_MAC;  

    retVal = rtl8306e_l2_mac_del(pMac->octet, &entryAddr);
    if(RTL8306_LUT_NOTEXIST == retVal)
    {
        return RT_ERR_L2_ENTRY_NOTFOUND;
    }
    else if(retVal != SUCCESS)
    {
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_l2_mcastAddr_add
 * Description:
 *      Add LUT multicast entry
 * Input:
 *      pMac               -   6 bytes unicast mac address to be deleted
 *      fid                   -   filtering database id, could be any value for RTL8306E switch
 *      portmask          -   Port mask to be forwarded to 
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                               -  Success
 *      RT_ERR_FAILED                         -   Failure
 *      RT_ERR_INPUT                          -   Invalid input parameter
 *      RT_ERR_PORT_MASK                  -   Invalid port mask
 *      RT_ERR_MAC                            -   Wrong mac address, must be unicast mac        
 *      RT_ERR_L2_INDEXTBL_FULL         -   Hashed index is full of entries
 * Note:
 *      If the multicast mac address already existed in the LUT, it will udpate the
 *      port mask of the entry. Otherwise, it will find an empty or asic auto learned
 *      entry to write. If all the entries with the same hash value can't be replaced, 
 *      ASIC will return a RT_ERR_L2_INDEXTBL_FULL error.
 */
rtk_api_ret_t rtk_l2_mcastAddr_add(rtk_mac_t *pMac, rtk_fid_t fid, rtk_portmask_t portmask)
{
    rtk_api_ret_t retVal;
    uint32 entryAddr;

    /* must be L2 multicast address */
    if((NULL == pMac) || (!(pMac->octet[0] & 0x1)))
        return RT_ERR_MAC;

    if(portmask.bits[0] > RTK_MAX_PORT_MASK)
        return RT_ERR_PORT_MASK;    

    retVal = rtl8306e_l2_multicastMac_add(pMac->octet, TRUE, portmask.bits[0], &entryAddr);
    if(RTL8306_LUT_FULL == retVal)
    {
        return RT_ERR_L2_INDEXTBL_FULL;
    }
    else if(retVal != SUCCESS)
    {
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_mcastAddr_get
 * Description:
 *      Get LUT multicast entry
 * Input:
 *      pMac               -   6 bytes multicast(I/G bit is 0) mac address to be gotten
 *      fid                   -   filtering database id, could be any value for RTL8306E switch
 * Output:
 *      pPortmask         -   the pointer of port mask      
 * Return:
 *      RT_ERR_OK                               -  Success
 *      RT_ERR_FAILED                         -   Failure
 *      RT_ERR_INPUT                          -   Invalid input parameter
 *      RT_ERR_NULL_POINTER              -   Input parameter is null pointer    
 *      RT_ERR_MAC                            -   Wrong mac address, must be unicast mac        
 *      RT_ERR_L2_INDEXTBL_FULL         -   Hashed index is full of entries 
 * Note:
 *      If the multicast mac address existed in LUT, it will return the port mask where
 *      the packet should be forwarded to, Otherwise, it will return a 
 *      RT_ERR_L2_ENTRY_NOTFOUND error.
 */

rtk_api_ret_t rtk_l2_mcastAddr_get(rtk_mac_t *pMac, rtk_fid_t fid, rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;
    uint32 isStatic, isAuth, portmask, entryaddr;        

    /* must be multicast address */
    if((NULL == pMac) || !(pMac->octet[0] & 0x1))
        return RT_ERR_MAC;  

    
    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8306e_l2_mac_get(pMac->octet, &isStatic, &isAuth, &portmask, &entryaddr);
    if(RTL8306_LUT_NOTEXIST == retVal)
    {        
        return RT_ERR_L2_ENTRY_NOTFOUND;        
    }
    else if(retVal != SUCCESS)
    {
        return RT_ERR_FAILED;
    }

    pPortmask->bits[0] = portmask;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_mcastAddr_del
 * Description:
 *      Delete LUT multicast entry
 * Input:
 *      pMac               -   6 bytes multicast(I/G bit is 1) mac address to be gotten
 *      fid                   -   filtering database id, could be any value for RTL8306E switch
 * Output:
*       none
 * Return:
 *      RT_ERR_OK                               -  Success
 *      RT_ERR_FAILED                         -   Failure
 *      RT_ERR_INPUT                          -   Invalid input parameter
 *      RT_ERR_MAC                            -   Wrong mac address, must be unicast mac        
 *      RT_ERR_L2_ENTRY_NOTFOUND     -   No such LUT entry
 * Note:
 *      If the mac has existed in the LUT, it will be deleted.
 *      Otherwise, it will return RT_ERR_L2_ENTRY_NOTFOUND.
 */

rtk_api_ret_t rtk_l2_mcastAddr_del(rtk_mac_t *pMac, rtk_fid_t fid)
{
    rtk_api_ret_t retVal;
    uint32 entryAddr;

    /* must be multicast address */
    if((NULL == pMac) || !(pMac->octet[0] & 0x1))
        return RT_ERR_MAC;  

    retVal = rtl8306e_l2_mac_del(pMac->octet, &entryAddr);
    if(RTL8306_LUT_NOTEXIST == retVal)
    {
        return RT_ERR_L2_ENTRY_NOTFOUND;
    }
    else if(retVal != SUCCESS)
    {
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_limitLearningCnt_set
 * Description:
 *      Set per-Port auto learning limit number
 * Input:
 *      port - Port id.
 *      mac_cnt - Auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - set shared meter successfully
 *      RT_ERR_FAILED          - FAILED to iset shared meter
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_LIMITED_L2ENTRY_NUM - Invalid auto learning limit number
 * Note:
 *      (1)The API can set per-port ASIC auto learning limit number from 0(disable learning) 
 *      to 0x1F(31). 
 *      (2)If mac_cnt is set from 0 to 0x1F, per-port ASIC auto learning limit will be enabled;
 *      if mac_cnt is set 0xFF, per-port ASIC auto learning limit will be disabled.
 */
rtk_api_ret_t rtk_l2_limitLearningCnt_set(rtk_port_t port, rtk_mac_cnt_t mac_cnt)
{
    rtk_api_ret_t retVal;
    uint32 enabled;
    
    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((mac_cnt > RTK_MAX_NUM_OF_LEARN_LIMIT) && (0xFF != mac_cnt))
        return RT_ERR_LIMITED_L2ENTRY_NUM;

    enabled = (0xFF == mac_cnt ?  FALSE : TRUE);
    if ((retVal = rtl8306e_l2_portMacLimit_set(port, enabled, mac_cnt)) != RT_ERR_OK)
        return retVal; 
    
    return RT_ERR_OK;
}    

/* Function Name:
 *      rtk_l2_limitLearningCnt_get
 * Description:
 *      Get per-Port auto learning limit number
 * Input:
 *      port - Port id.
 * Output:
 *      pMac_cnt - Auto learning entries limit number
 * Return:
 *      RT_ERR_OK              - Success
 *      RT_ERR_FAILED          - Failure 
 *      RT_ERR_PORT_ID - Invalid port number. 
 * Note:
 *      The API can get per-port ASIC auto learning limit number.
 */
rtk_api_ret_t rtk_l2_limitLearningCnt_get(rtk_port_t port, rtk_mac_cnt_t *pMac_cnt)
{
    rtk_api_ret_t retVal;
    uint32 enabled;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8306e_l2_portMacLimit_get(port, &enabled, pMac_cnt)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_limitLearningCntAction_set
 * Description:
 *      Configure auto learn over limit number action.
 * Input:
 *      port - Port id (must be RTK_WHOLE_SYSTEM)
 *      action - Auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - Success
 *      RT_ERR_FAILED          - Failure 
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_NOT_ALLOWED - Invalid learn over action
 * Note:
 *      (1)The API can set SA unknown packet action while auto learn limit number is over. 
 *      The action symbol as following:
 *      - LIMIT_LEARN_CNT_ACTION_DROP,
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU,
 *      (2)The action is global, so the port must be set as RTK_WHOLE_SYSTEM
 */
rtk_api_ret_t rtk_l2_limitLearningCntAction_set(rtk_port_t port, rtk_l2_limitLearnCntAction_t action)
{
    rtk_api_ret_t retVal;
    uint32 data;
    
    if (port != RTK_WHOLE_SYSTEM)
        return RT_ERR_PORT_ID;    
    
    if ( LIMIT_LEARN_CNT_ACTION_DROP == action )
        data = RTL8306_ACT_DROP;
    else if ( LIMIT_LEARN_CNT_ACTION_TO_CPU == action )
        data = RTL8306_ACT_TRAPCPU;
    else
        return RT_ERR_NOT_ALLOWED;    

    
    if ((retVal = rtl8306e_l2_macLimitAction_set(data)) != RT_ERR_OK)
        return retVal; 

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_limitLearningCntAction_get
 * Description:
 *      Get auto learn over limit number action.
 * Input:
 *      port - Port id (must be RTK_WHOLE_SYSTEM)
 * Output:
 *      pAction - Learn over action
 * Return:
 *      RT_ERR_OK              - Success
 *      RT_ERR_FAILED          - Failure 
 *      RT_ERR_PORT_ID - Invalid port number. 
 * Note:
 *      (1)The API can get SA unknown packet action while auto learn limit number is over. 
 *      The action symbol as following:
 *      - LIMIT_LEARN_CNT_ACTION_DROP,
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU,
 *      (2)The action is global, so the port must be set as RTK_WHOLE_SYSTEM
 */
rtk_api_ret_t rtk_l2_limitLearningCntAction_get(rtk_port_t port, rtk_l2_limitLearnCntAction_t *pAction)
{
    rtk_api_ret_t retVal;
    uint32 action;
    
    if (port != RTK_WHOLE_SYSTEM)
        return RT_ERR_PORT_ID;    
    
    if ((retVal = rtl8306e_l2_macLimitAction_get(&action)) != RT_ERR_OK)
        return retVal; 

    if (RTL8306_ACT_DROP == action)
        *pAction = LIMIT_LEARN_CNT_ACTION_DROP;
    else if (RTL8306_ACT_TRAPCPU == action)
        *pAction = LIMIT_LEARN_CNT_ACTION_TO_CPU;
    else
        return RT_ERR_FAILED; 

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_learningCnt_get
 * Description:
 *      Get per-Port current auto learning number
 * Input:
 *      port - Port id.
 * Output:
 *      pMac_cnt - ASIC auto learning entries number
 * Return:
 *      RT_ERR_OK              - Success
 *      RT_ERR_PORT_ID - Invalid port number. 
 *      RT_ERR_NULL_POINTER   -   Input parameter is null pointer  
 * Note:
 *      The API can get per-port ASIC auto learning number
 */
rtk_api_ret_t rtk_l2_learningCnt_get(rtk_port_t port, rtk_mac_cnt_t *pMac_cnt)
{
    uint32 regVal;
    
    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;
    
    if (NULL == pMac_cnt)
        return RT_ERR_NULL_POINTER;
    
    if (5 == port)
        port ++;

    if (0 == port)
    {
        rtl8306e_reg_get(0, 27, 3, &regVal);
        *pMac_cnt = (regVal & 0x7FF) ;
    }
    else
    {
        rtl8306e_reg_get(port, 31, 1, &regVal);
        *pMac_cnt = (regVal & 0x7FF) ;
    }
 
    return RT_ERR_OK;
}



/* Function Name:
 *      rtk_cpu_enable_set
 * Description:
 *      Set cpu port function enable or disable
 * Input:
 *      enable          - enable or disable
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                        -  Success
 *      RT_ERR_FAILED                  -   Failure
 * Note:
 *      The API can set CPU port function enable/disable
 *      default port 4 is cpu port.
 */
rtk_api_ret_t rtk_cpu_enable_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (ENABLED == enable)
        retVal = rtl8306e_cpu_set(RTL8306_PORT4, FALSE);
    else
        retVal = rtl8306e_cpu_set(RTL8306_NOCPUPORT, FALSE);
    
    return retVal;    
}


/* Function Name:
 *      rtk_cpu_enable_get
 * Description:
 *      Get cpu port function enable or disable
 * Input:
 *      none
 * Output:
 *      pEnable          -  the pointer of enable or disable cpu port function
 * Return:
 *      RT_ERR_OK                   -  Success
 *      RT_ERR_FAILED             -   Failure
 * Note:
 *      The API can set CPU port function enable/disable
 */
rtk_api_ret_t rtk_cpu_enable_get(rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    uint32 port, entag;
    
    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;
    
    retVal = rtl8306e_cpu_get(&port, &entag);
    if (retVal != SUCCESS)
        return RT_ERR_FAILED;

    if(RTL8306_NOCPUPORT == port)
        *pEnable = DISABLED;
    else
        *pEnable = ENABLED;
        
    return RT_ERR_OK;    
}
#endif
/* Function Name:
 *      rtk_cpu_tagPort_set
 * Description:
 *      Set CPU port and CPU tag insert mode
 * Input:
 *      port          -  Port id
 *      mode        -  CPU tag insert for packets egress from CPU port
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                   -  Success
 *      RT_ERR_FAILED             -   Failure
 *      RT_ERR_PORT_ID           -   Invalid port id
 * Note:
 *      The API can set CPU port and inserting proprietary CPU tag mode (Length/Type 0x8899)
 *      to the frame that transmitting to CPU port.
 *      The inset cpu tag mode is as following:
 *          - CPU_INSERT_TO_ALL
 *          - CPU_INSERT_TO_TRAPPING
 *          - CPU_INSERT_TO_NONE   
 */
 
rtk_api_ret_t rtk_cpu_tagPort_set(rtk_port_t port, rtk_cpu_insert_t mode)
{
    rtk_api_ret_t retVal;
    uint32 regval;

    if(port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    switch(mode)
    {
        case CPU_INSERT_TO_ALL:
            retVal = rtl8306e_cpu_set(port, TRUE);             
            if(retVal != SUCCESS)
                return RT_ERR_FAILED;
            break;
        case CPU_INSERT_TO_TRAPPING:
            retVal = rtl8306e_cpu_set(port, FALSE);                        
            if(retVal != SUCCESS)
                return RT_ERR_FAILED;
            /*enable insert cpu tag for trap pkt */
            rtl8306e_reg_get(6, 30, 1, &regval);
            regval |= (0x7F << 8);
            rtl8306e_reg_set(6, 30, 1, regval);            
            break;
        case CPU_INSERT_TO_NONE:
            retVal = rtl8306e_cpu_set(port, FALSE);                        
            if(retVal != SUCCESS)
                return RT_ERR_FAILED;
            /*disable insert cpu tag for trap pkt */
            rtl8306e_reg_get(6, 30, 1, &regval);
            regval &= ~(0x7F << 8);
            rtl8306e_reg_set(6, 30, 1, regval);                        
            break;
        default:
            return RT_ERR_INPUT;
    }
                    
    return RT_ERR_OK;        
}
#ifndef CONFIG_ETHWAN

/* Function Name:
 *      rtk_cpu_tagPort_get
 * Description:
 *      Get CPU port and CPU tag insert mode
 * Input:
 *      port          -  Port id
 * Output:
 *      pMode       -  the pointer of CPU tag insert for packets egress from CPU port
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -   Failure
 *      RT_ERR_NULL_POINTER      -   Input parameter is null pointer
 * Note:
 *      The API can set CPU port and inserting proprietary CPU tag mode (Length/Type 0x8899)
 *      to the frame that transmitting to CPU port.
 *      The inset cpu tag mode is as following:
 *          CPU_INSERT_TO_ALL
 *          CPU_INSERT_TO_TRAPPING
 *          CPU_INSERT_TO_NONE   
 */

rtk_api_ret_t rtk_cpu_tagPort_get(rtk_port_t *pPort, rtk_cpu_insert_t *pMode)
{
    rtk_api_ret_t retVal;
    uint32 enCputag;
    uint32 regval;

    if ((NULL == pPort) || (NULL == pMode))
        return RT_ERR_NULL_POINTER;
    
    retVal = rtl8306e_cpu_get(pPort, &enCputag);
    if(retVal != SUCCESS)
        return RT_ERR_FAILED;
    
    if(enCputag)
    {
        *pMode = CPU_INSERT_TO_ALL;
    }
    else
    {
        /*enable insert cpu tag for trap pkt */
        rtl8306e_reg_get(6, 30, 1, &regval);
        if((0x7F << 8) == (regval & (0x7F << 8)))
            *pMode = CPU_INSERT_TO_TRAPPING;
        else
            *pMode = CPU_INSERT_TO_NONE;
    }
    
    return RT_ERR_OK;                    
}


#endif
/* Function Name:
 *      rtk_mirror_portBased_set
 * Description:
 *      Set port mirror function
 * Input:
 *      mirroring_port                  - Monitor port, 7 means no monitor port
 *      pMirrored_rx_portmask      - the pointer of Rx mirror port mask
 *      pMirrored_tx_portmask      - the pointer of Tx mirror port mask
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -   Failure
 *      RT_ERR_PORT_MASK         -   Invalid port mask
 * Note:
 *      The API is to set mirror function of source port and mirror port. 
 */

rtk_api_ret_t rtk_mirror_portBased_set(rtk_port_t mirroring_port, rtk_portmask_t *pMirrored_rx_portmask, rtk_portmask_t *pMirrored_tx_portmask)
{
    rtk_api_ret_t retVal;
        
    if ((NULL == pMirrored_rx_portmask) || (NULL == pMirrored_tx_portmask))
        return RT_ERR_NULL_POINTER;
    
    if (pMirrored_rx_portmask->bits[0] > RTL8306_MAX_PORTMASK)
        return RT_ERR_PORT_MASK; 

    if (pMirrored_tx_portmask->bits[0] > RTL8306_MAX_PORTMASK)
        return RT_ERR_PORT_MASK;

    retVal = rtl8306e_mirror_portBased_set(mirroring_port, 
                pMirrored_rx_portmask->bits[0],
                pMirrored_tx_portmask->bits[0]);
    return retVal;
}

#ifndef CONFIG_ETHWAN
/* Function Name:
 *      rtk_mirror_portBased_get
 * Description:
 *      Get port mirror function
 * Input:
 *      none 
 * Output:
 *      pMirroring_port             - the pointer Monitor port, 7 means no monitor port
 *      pMirrored_rx_portmask   - the pointer of Rx mirror port mask
 *      pMirrored_tx_portmask   - the pointer of Tx mirror port mask 
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -   Failure
 *      RT_ERR_PORT_MASK         -   Invalid port mask
 *      RT_ERR_NULL_POINTER      -   Input parameter is null pointer 
 * Note:
 *      The API is to set mirror function of source port and mirror port. 
 */
rtk_api_ret_t rtk_mirror_portBased_get(rtk_port_t *pMirroring_port, rtk_portmask_t *pMirrored_rx_portmask, rtk_portmask_t *pMirrored_tx_portmask)
{
    rtk_api_ret_t retVal;
    
    if ((NULL == pMirrored_rx_portmask) || (NULL == pMirrored_tx_portmask) ||
        (NULL == pMirroring_port))
        return RT_ERR_NULL_POINTER;
    
    retVal = rtl8306e_mirror_portBased_get(pMirroring_port, 
                &(pMirrored_rx_portmask->bits[0]),
                &(pMirrored_tx_portmask->bits[0]));
    
    return retVal;
}


/* Function Name:
 *      rtk_dot1x_unauthPacketOper_set
 * Description:
 *      Set 802.1x unauth action configuration
 * Input:
 *      port                 -   Port id, no use for RTL8306E switch
 *      unauth_action   -   802.1X unauth action    
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -   Failure
 *      RT_ERR_DOT1X_PROC        -   Unauthorized behavior error
 * Note:
 *      This API can set 802.1x unauth action configuration, 
 *      for RTL8306E switch, the action is by whole system,
 *      so port could be any value of 0~6.
 *      The unauth action is as following:
 *          - DOT1X_ACTION_DROP
 *          - DOT1X_ACTION_TRAP2CPU
 */
rtk_api_ret_t rtk_dot1x_unauthPacketOper_set(rtk_port_t port, rtk_dot1x_unauth_action_t unauth_action)
{
    rtk_api_ret_t retVal;

    if(unauth_action > DOT1X_ACTION_TRAP2CPU)
        return RT_ERR_DOT1X_PROC;

    if(DOT1X_ACTION_DROP == unauth_action)
    {
        retVal = rtl8306e_trap_abnormalPktAction_set(RTL8306_DOT1XUNAUTH, RTL8306_ACT_DROP);
    }
    else if(DOT1X_ACTION_TRAP2CPU == unauth_action)
    {
        retVal = rtl8306e_trap_abnormalPktAction_set(RTL8306_DOT1XUNAUTH, RTL8306_ACT_TRAPCPU);
    }
    else
        retVal = RT_ERR_DOT1X_PROC;

    return retVal;
}

/* Function Name:
 *      rtk_dot1x_unauthPacketOper_get
 * Description:
 *      Get 802.1x unauth action configuration
 * Input:
 *      port                  -   Port id, no use for RTL8306E switch
 * Output:
 *      pUnauth_action   -  the pointer of 802.1X unauth action    
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -   Failure
 *      RT_ERR_NULL_POINTER      -   Input parameter is null pointer
 * Note:
 *      This API can set 802.1x unauth action configuration, 
 *      for RTL8306E switch, the action is by whole system,
 *      so port could be any value of 0~6.
 *      The unauth action is as following:
 *          - DOT1X_ACTION_DROP
 *          - DOT1X_ACTION_TRAP2CPU
 */
rtk_api_ret_t rtk_dot1x_unauthPacketOper_get(rtk_port_t port, rtk_dot1x_unauth_action_t *pUnauth_action)
{
    rtk_api_ret_t retVal;
    uint32 action;
    
    if(NULL == pUnauth_action)
        return RT_ERR_NULL_POINTER;
    
    retVal = rtl8306e_trap_abnormalPktAction_get(RTL8306_DOT1XUNAUTH, &action);

    if (retVal != SUCCESS)
        return RT_ERR_FAILED;
    
    if(RTL8306_ACT_DROP == action)
    {
        *pUnauth_action = DOT1X_ACTION_DROP;
    }
    else 
    {
        *pUnauth_action = DOT1X_ACTION_TRAP2CPU;
    }
    
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_dot1x_portBasedEnable_set
 * Description:
 *      Set 802.1x port-based enable configuration
 * Input:
 *      port                  -   Port id
 *      enable               -   enable or disable
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -   Failure
 *      RT_ERR_PORT_ID              -   Invalid port id
 * Note:
 *      The API can update the port-based port enable register content. If a port is 802.1x 
 *      port based network access control "enabled", it should be authenticated so packets 
 *      from that port won't be dropped or trapped to CPU. 
 *      The status of 802.1x port-based network access control is as following:
 *          - DISABLED
 *          - ENABLED
 */    
 
rtk_api_ret_t rtk_dot1x_portBasedEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    uint32 isAuth, direction, enDot1x;

    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;    

    retVal = rtl8306e_dot1x_portBased_get(port, &enDot1x, &isAuth, &direction);
    if(retVal != SUCCESS)
        return RT_ERR_FAILED;
    
    retVal = rtl8306e_dot1x_portBased_set(port, (uint32)enable, isAuth, direction);
    if(retVal != SUCCESS)
        return RT_ERR_FAILED;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_portBasedEnable_get
 * Description:
 *      Get 802.1x port-based enable configuration
 * Input:
 *      port                  -   Port id
 * Output:
 *      pEnable             -   the pointer of enable or disable
 * Return:
 *      RT_ERR_OK                -  Success
 *      RT_ERR_FAILED          -   Failure
 *      RT_ERR_PORT_ID        -   Invalid port id
 * Note:
 *      The API can update the port-based port enable register content. If a port is 802.1x 
 *      port based network access control "enabled", it should be authenticated so packets 
 *      from that port won't be dropped or trapped to CPU. 
 *      The status of 802.1x port-based network access control is as following:
 *          - DISABLED
 *          - ENABLED
 */    
 
rtk_api_ret_t rtk_dot1x_portBasedEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    uint32 isAuth, direction, enDot1x;

    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;    

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8306e_dot1x_portBased_get(port, &enDot1x, &isAuth, &direction);
    if(retVal != SUCCESS)
        return RT_ERR_FAILED;
    
    *pEnable = enDot1x ? ENABLED : DISABLED;
            
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_portBasedAuthStatus_set
 * Description:
 *      Set 802.1x port-based enable configuration
 * Input:
 *      port                  -   Port id
 *      port_auth          -  The status of 802.1x port
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                    -  Success
 *      RT_ERR_FAILED                              -   Failure
 *      RT_ERR_PORT_ID                            -   Invalid port id
 *      RT_ERR_DOT1X_PORTBASEDAUTH      -   Port-based auth port error
 * Note:
 *      The authenticated status of 802.1x port-based network access control is as following:
 *          - UNAUTH
 *          - AUTH
 */    
rtk_api_ret_t rtk_dot1x_portBasedAuthStatus_set(rtk_port_t port, rtk_dot1x_auth_status_t port_auth)
{
    rtk_api_ret_t retVal;
    uint32 isAuth, direction, enDot1x;

    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;    

    if(port_auth > AUTH_STATUS_END)
        return RT_ERR_DOT1X_PORTBASEDAUTH;

    retVal = rtl8306e_dot1x_portBased_get(port, &enDot1x, &isAuth, &direction);
    if(retVal != SUCCESS)
        return RT_ERR_FAILED;
    
    retVal = rtl8306e_dot1x_portBased_set(port, enDot1x, (uint32)port_auth, direction);
    if(retVal != SUCCESS)
        return RT_ERR_FAILED;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_portBasedAuthStatus_get
 * Description:
 *      Get 802.1x port-based enable configuration
 * Input:
 *      port                  -   Port id
 * Output:
 *      pPort_auth         -   the pointer of the status of 802.1x port
 * Return:
 *      RT_ERR_OK                          -  Success
 *      RT_ERR_FAILED                     -  Failure
 *      RT_ERR_PORT_ID                   -  Invalid port id
 *      RT_ERR_NULL_POINTER           -  Input parameter is null pointer
 * Note:
 *      The authenticated status of 802.1x port-based network access control is as following:
 *          - UNAUTH
 *          - AUTH
 */    
rtk_api_ret_t rtk_dot1x_portBasedAuthStatus_get(rtk_port_t port, rtk_dot1x_auth_status_t *pPort_auth)
{
    rtk_api_ret_t retVal;
    uint32 isAuth, direction, enDot1x;

    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;    
    if (NULL == pPort_auth)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8306e_dot1x_portBased_get(port, &enDot1x, &isAuth, &direction);
    if(retVal != SUCCESS)
        return RT_ERR_FAILED;
    *pPort_auth = isAuth ? AUTH: UNAUTH;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_portBasedDirection_set
 * Description:
 *      Set 802.1x port-based operational direction configuration
 * Input:
 *      port                  -   Port id
 *      port_direction     -   Operation direction
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                 -  Success
 *      RT_ERR_FAILED                           -  Failure
 *      RT_ERR_PORT_ID                         -  Invalid port id
 *      RT_ERR_DOT1X_PORTBASEDOPDIR  -  Port-based opdir error
 * Note:
 *      The operate controlled direction of 802.1x port-based network access control is as following:
 *          - BOTH
 *          - IN
 */    

rtk_api_ret_t rtk_dot1x_portBasedDirection_set(rtk_port_t port, rtk_dot1x_direction_t port_direction)
{
    rtk_api_ret_t retVal;
    uint32 isAuth, direction, enDot1x;

    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;    
    
    if(port_direction >= DIRECTION_END)
        return RT_ERR_DOT1X_PORTBASEDOPDIR;

    retVal = rtl8306e_dot1x_portBased_get(port, &enDot1x, &isAuth, &direction);
    if(retVal != SUCCESS)
        return RT_ERR_FAILED;
    
    retVal = rtl8306e_dot1x_portBased_set(port, enDot1x, isAuth, (uint32)port_direction);
    if(retVal != SUCCESS)
        return RT_ERR_FAILED;
    
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_dot1x_portBasedDirection_get
 * Description:
 *      Get 802.1x port-based operational direction configuration
 * Input:
 *      port                  -   Port id
 * Output:
 *      pPort_direction    -   the pointer of Operation direction
 * Return:
 *      RT_ERR_OK                         -  Success
 *      RT_ERR_FAILED                    -  Failure
 *      RT_ERR_PORT_ID                  -  Invalid port id
 *      RT_ERR_NULL_POINTER          -  Input parameter is null pointer
 * Note:
 *      The operate controlled direction of 802.1x port-based network access control is as following:
 *          - BOTH
 *          - IN
 */    
rtk_api_ret_t rtk_dot1x_portBasedDirection_get(rtk_port_t port, rtk_dot1x_direction_t *pPort_direction)
{
    rtk_api_ret_t retVal;
    uint32 isAuth, direction, enDot1x;

    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;    

    if(NULL == pPort_direction)
        return RT_ERR_NULL_POINTER;
    
    retVal = rtl8306e_dot1x_portBased_get(port, &enDot1x, &isAuth, &direction);
    if(retVal != SUCCESS)
        return RT_ERR_FAILED;

    *pPort_direction = (rtk_dot1x_direction_t)direction;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_macBasedEnable_set
 * Description:
 *      Set 802.1x mac-based port enable configuration
 * Input:
 *      port                  -   Port id
 *      enable               -   The status of 802.1x mac-base funtion
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                         -  Success
 *      RT_ERR_FAILED                    -  Failure
 *      RT_ERR_PORT_ID                  -  Invalid port id
 * Note:
 *     If a port is 802.1x MAC based network access control "enabled", the incoming packets should 
 *     be authenticated so packets from that port won't be dropped or trapped to CPU.
 *     The status of 802.1x MAC-based network access control is as following:
 *         - DISABLED
 *         - ENABLED
 */    
rtk_api_ret_t rtk_dot1x_macBasedEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    uint32 direction, enMacBase;
    
    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;    
    
    retVal = rtl8306e_dot1x_macBased_get(port, &enMacBase, &direction); 
    if (retVal != SUCCESS)
        return RT_ERR_FAILED;

    retVal = rtl8306e_dot1x_macBased_set(port, (uint32)enable, direction);
    if (retVal != SUCCESS)
        return RT_ERR_FAILED;
       
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_macBasedEnable_get
 * Description:
 *      Get 802.1x mac-based port enable configuration
 * Input:
 *      port                  -   Port id
 * Output:
 *      pEnable             -   the pointer of the status of 802.1x mac-base funtion
 * Return:
 *      RT_ERR_OK               -  Success
 *      RT_ERR_FAILED         -   Failure
 * Note:
 *     If a port is 802.1x MAC based network access control "enabled", the incoming packets should 
 *     be authenticated so packets from that port won't be dropped or trapped to CPU.
 *     The status of 802.1x MAC-based network access control is as following:
 *         - DISABLED
 *         - ENABLED
 */    
rtk_api_ret_t rtk_dot1x_macBasedEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    uint32 direction, enMacBase;
    
    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;    

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;
    
    retVal = rtl8306e_dot1x_macBased_get(port, &enMacBase, &direction); 
    if (retVal != SUCCESS)
        return RT_ERR_FAILED;

    *pEnable = enMacBase ? ENABLED: DISABLED;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_macBasedDirection_set
 * Description:
 *      Set 802.1x mac-based operational direction configuration
 * Input:
 *      mac_direction    -   Operation direction
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                    -  Success
 *      RT_ERR_FAILED                              -  Failure
 *      RT_ERR_DOT1X_MACBASEDOPDIR      -  MAC-based opdir error
 * Note:
 *      The operate controlled direction of 802.1x mac-based network access control is as following:
 *          - BOTH
 *          - IN
 */    

rtk_api_ret_t rtk_dot1x_macBasedDirection_set(rtk_dot1x_direction_t mac_direction)
{
    rtk_api_ret_t retVal;
        
    if(mac_direction > DIRECTION_END) 
        return RT_ERR_DOT1X_MACBASEDOPDIR;    

    retVal = rtl8306e_regbit_set(2, 22, 7, 3, mac_direction == BOTH ? 0:1);

    return retVal;
}
    
/* Function Name:
 *      rtk_dot1x_macBasedDirection_get
 * Description:
 *      Get 802.1x mac-based operational direction configuration
 * Input:
 *      none
 * Output:
 *      pMac_direction    -   the pointer of Operation direction
 * Return:
 *      RT_ERR_OK                                    -  Success
 *      RT_ERR_FAILED                              -  Failure
 *      RT_ERR_NULL_POINTER                    -  Input parameter is null pointer
 * Note:
 *      The operate controlled direction of 802.1x mac-based network access control is as following:
 *          - BOTH
 *          - IN
 */
rtk_api_ret_t rtk_dot1x_macBasedDirection_get(rtk_dot1x_direction_t *pMac_direction)
{
    rtk_api_ret_t retVal;
    uint32          dir;

    if(NULL == pMac_direction)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8306e_regbit_get(2, 22, 7, 3, &dir);
    if(retVal != SUCCESS)
        return RT_ERR_FAILED;
    
    *pMac_direction = dir ? IN: BOTH;
    
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_dot1x_macBasedAuthMac_add
 * Description:
 *      Add an authenticated MAC to ASIC
 * Input:
 *      port            -  Port id
 *      pAuth_mac   - The authenticated MAC
 *      fid              -  no use for RTL8306E   
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                       - Success
 *      RT_ERR_FAILED                                 -  Failure
 *      RT_ERR_L2_ENTRY_NOTFOUND             -  Specified entry not found
 *      RT_ERR_DOT1X_MAC_PORT_MISMATCH  - Auth MAC and port mismatch eror 
 * Note:
 *     The API can add a 802.1x authenticated MAC address to port. If the MAC does not exist in LUT, 
 *     user can't add this MAC to auth status.
 */    

rtk_api_ret_t rtk_dot1x_macBasedAuthMac_add(rtk_port_t port, rtk_mac_t *pAuth_mac, rtk_fid_t fid)
{
    int32 i;
    uint8 macAddr[6];
    uint32 index,entryaddr;
    uint32 isStatic,isAuth,age, srcPort;
    uint32 isHit;


    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;        

    if(NULL == pAuth_mac)
        return RT_ERR_NULL_POINTER;

    if(pAuth_mac->octet[0] & 0x1)
        return RT_ERR_DOT1X_MAC_PORT_MISMATCH;

    rtl8306e_l2_MacToIdx_get(pAuth_mac->octet, &index);   
    
    isHit = FALSE;
    for (i = 3; i >= 0; i--) 
    {
        entryaddr = (index << 2) | (uint32)i;
        if (rtl8306e_l2_unicastEntry_get(macAddr, entryaddr, &age, &isStatic, &isAuth, &srcPort) != SUCCESS) 
        {
            return RT_ERR_FAILED;
        }
        else if ((pAuth_mac->octet[0] == macAddr[0]) && (pAuth_mac->octet[1] == macAddr[1]) && 
                   (pAuth_mac->octet[2] == macAddr[2]) && (pAuth_mac->octet[3] == macAddr[3]) &&
                   (pAuth_mac->octet[4] == macAddr[4]) && (pAuth_mac->octet[5] == macAddr[5])) 
        {
            if(srcPort != port)
                return RT_ERR_DOT1X_MAC_PORT_MISMATCH;
            if (rtl8306e_l2_unicastEntry_set(pAuth_mac->octet, (uint32)i , age, isStatic, TRUE, port) != SUCCESS)
                return RT_ERR_FAILED;
            isHit = TRUE;
            return RT_ERR_OK;
        }
    }

    if(!isHit)
        return RT_ERR_L2_ENTRY_NOTFOUND;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_macBasedAuthMac_del
 * Description:
 *      Delete an authenticated MAC to ASIC
 * Input:
 *      port            -  Port id
 *      pAuth_mac   - The authenticated MAC
 *      fid              -  no use for RTL8306E   
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                       - Success
 *      RT_ERR_FAILED                                 -  Failure
 *      RT_ERR_L2_ENTRY_NOTFOUND             -  Specified entry not found
 *      RT_ERR_DOT1X_MAC_PORT_MISMATCH  - Auth MAC and port mismatch eror 
 * Note:
 *     The API can delete a 802.1x authenticated MAC address to port. It only change the auth status of
 *     the MAC and won't delete it from LUT.
 */
rtk_api_ret_t rtk_dot1x_macBasedAuthMac_del(rtk_port_t port, rtk_mac_t *pAuth_mac, rtk_fid_t fid)
{
    int32 i;
    uint8 macAddr[6];
    uint32 index,entryaddr;
    uint32 isStatic,isAuth,age, srcPort;
    uint32 isHit;

    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;        

    if(NULL == pAuth_mac)
        return RT_ERR_NULL_POINTER;

    if(pAuth_mac->octet[0] & 0x1)
        return RT_ERR_DOT1X_MAC_PORT_MISMATCH;

    rtl8306e_l2_MacToIdx_get(pAuth_mac->octet, &index);   
    
    isHit = FALSE;
    for (i = 3; i >= 0; i--) 
    {
        entryaddr = (index << 2) | (uint32)i;
        if (rtl8306e_l2_unicastEntry_get(macAddr, entryaddr, &age, &isStatic, &isAuth, &srcPort) != SUCCESS) 
        {
            return RT_ERR_FAILED;
        }
        else if ((pAuth_mac->octet[0] == macAddr[0]) && (pAuth_mac->octet[1] == macAddr[1]) && 
                   (pAuth_mac->octet[2] == macAddr[2]) && (pAuth_mac->octet[3] == macAddr[3]) &&
                   (pAuth_mac->octet[4] == macAddr[4]) && (pAuth_mac->octet[5] == macAddr[5])) 
        {
            if(srcPort != port)
                return RT_ERR_DOT1X_MAC_PORT_MISMATCH;
            if (rtl8306e_l2_unicastEntry_set(pAuth_mac->octet, (uint32)i , age, isStatic, FALSE, port) != SUCCESS)
                return RT_ERR_FAILED;
            isHit = TRUE;
            return RT_ERR_OK;
        }
    }

    if(!isHit)
        return RT_ERR_L2_ENTRY_NOTFOUND;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_init
 * Description:
 *      Initialize SVLAN Configuration
 * Input:
 *      none
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_INPUT
 * Note:
 *    Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 
 *    0x9100 and 0x9200 for Q-in-Q SLAN design. User can set mathced ether 
 *    type as service provider supported protocol. After call this API, all ports are 
 *    set as CVLAN port. you can use rtk_svlan_servicePort_add to add SVLAN port. 
 */
rtk_api_ret_t rtk_svlan_init(void)
{

    uint32 regval;
    uint32 port;
    
    /*enable Q_in_Q*/
    rtl8306e_regbit_set(6, 30, 6, 3, 1);
    rtl8306e_regbit_set(6, 30, 7, 3, 1);    

   /*clear NNI port list*/
   rtl8306e_reg_get(6, 30, 3, &regval);
   regval &= (~0x3f);
   rtl8306e_reg_set(6, 30, 3, regval);
    
   /*set default TPID*/
    rtl8306e_reg_set(6, 28, 3, 0x88a8);

   /*default use port-base otag PVID and priority as Otag source*/
   for(port = 0; port < 6; port ++)
        rtl8306e_svlan_otagSrc_set(port, 0, 0);

   return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_servicePort_add
 * Description:
 *      Enable one service port in the specified device
 * Input:
 *      port     -  Port id
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 * Note:
 *    This API is setting which port is connected to provider switch. All frames receiving from this port
 *    will recognize Service Vlan Tag.
 *    
 */
rtk_api_ret_t rtk_svlan_servicePort_add(rtk_port_t port)
{
    uint32 regval;

    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;        

    /*add the port to NNI port list*/
    rtl8306e_reg_get(6, 30, 3, &regval);    
    regval |= (1 << port);
    rtl8306e_reg_set(6, 30, 3, regval);
    
    return RT_ERR_OK;    
}


/* Function Name:
 *      rtk_svlan_servicePort_del
 * Description:
 *      Disable one service port in the specified device
 * Input:
 *      none
 * Output:
 *      pSvlan_portmask  - svlan ports mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 * Note:
 */
rtk_api_ret_t rtk_svlan_servicePort_del(rtk_port_t port)
{
    uint32 regval;

    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;        
    
    /*delete the port from NNI port list*/
    rtl8306e_reg_get(6, 30, 3, &regval);    
    regval &= (~(1 << port));
    rtl8306e_reg_set(6, 30, 3, regval);
        
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_svlan_servicePort_get
 * Description:
 *      Disable one service port in the specified device
 * Input:
 *      none
 * Output:
 *      pSvlan_portmask  - svlan ports mask
 * Return:
 *      RT_ERR_OK                 - success
 *      RT_ERR_FAILED            -  fail
 *      RT_ERR_NULL_POINTER  -  null pointer
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 
 *      and 0x9200 for Q-in-Q SLAN design. User can set mathced ether type as service 
 *      provider supported protocol. 
 */
rtk_api_ret_t rtk_svlan_servicePort_get(rtk_portmask_t *pSvlan_portmask)
{
    uint32 regval;

    if(NULL == pSvlan_portmask)
        return RT_ERR_NULL_POINTER;

    rtl8306e_reg_get(6, 30, 3, &regval);
    pSvlan_portmask->bits[0] = regval & 0x3F;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_svlan_tpidEntry_set
 * Description:
 *      Configure accepted S-VLAN ether type. The default ether type of S-VLAN is 0x88a8
 * Input:
 *      svlan_tag_id  - Ether type of S-tag frame parsing in uplink ports
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 
 *      and 0x9200 for Q-in-Q SLAN design. User can set mathced ether type as service 
 *      provider supported protocol. 
 */
rtk_api_ret_t rtk_svlan_tpidEntry_set(rtk_svlan_tpid_t svlan_tag_id)
{

    rtl8306e_reg_set(6, 28, 3, (uint32)svlan_tag_id);

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_tpidEntry_get
 * Description:
 *      Get accepted S-VLAN ether type. The default ether type of S-VLAN is 0x88a8
 * Input:
 *      pSvlan_tag_id       - Ether type of S-tag frame parsing in uplink ports
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 
 *      and 0x9200 for Q-in-Q SLAN design. User can set mathced ether type as service 
 *      provider supported protocol. 
 */
rtk_api_ret_t rtk_svlan_tpidEntry_get(rtk_svlan_tpid_t *pSvlan_tag_id)
{
    uint32 regval;
    
    if(NULL == pSvlan_tag_id)
        return RT_ERR_NULL_POINTER;
    
    rtl8306e_reg_get(6, 28, 3, &regval);
    *pSvlan_tag_id = regval;    

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_portPvid_set
 * Description:
 *      Set port to specified VLAN ID(PVID) for Service Provider Port
 * Input:
 *      port             - Port id
 *      pvid             - Specified Service VLAN ID
 *      priority         - 802.1p priority for the PVID
 *      dei               - Service VLAN tag DEI bit
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_VLAN_VID
 *      RT_ERR_SMI 
 *      RT_ERR_VLAN_PRIORITY 
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND
 * Note:
 *    The API is used for Port-based VLAN. The untagged frame received from the
 *    port will be classified to the specified VLAN and assigned to the specified priority.
 */
rtk_api_ret_t rtk_svlan_portPvid_set(rtk_port_t port, rtk_vlan_t pvid, rtk_pri_t priority, rtk_dei_t dei)
{
    uint32 regval;
    
    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;        
    
    /* vid must be 0~4095 */
    if(pvid > 0x4095)
        return RT_ERR_VLAN_VID;

    /* priority must be 0~7 */
    if(priority > 0x7)
        return RT_ERR_VLAN_PRIORITY;

    rtl8306e_reg_get(6, 26 + port, 0, &regval);
    regval &= (1 << 12);
    regval |= pvid | (priority << 13);
    rtl8306e_reg_set(6, 26 + port, 0, regval);

    return RT_ERR_OK;
}



/* Function Name:
 *      rtk_svlan_portPvid_get
 * Description:
 *      Get Service VLAN ID(PVID) on specified port
 * Input:
 *      port             - Port id
 *      pPvid            - Specified VLAN ID
 *      pPriority        - 802.1p priority for the PVID
 *      pDei             - DEI bit
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_PORT_ID
 *      RT_ERR_NULL_POINTER 
 * Note:
 *    The API is used for Port-based VLAN. The untagged frame received from the
 *    port will be classified to the specified VLAN and assigned to the specified priority.
 */
rtk_api_ret_t rtk_svlan_portPvid_get(rtk_port_t port, rtk_vlan_t *pPvid, rtk_pri_t* pPriority, rtk_dei_t *pDei)
{
    uint32 regval;
    
    if(port > RTK_PORT_ID_MAX) 
        return RT_ERR_PORT_ID;        
    
    if((NULL == pPvid) || (NULL == pPriority) || (NULL == pDei))
        return RT_ERR_NULL_POINTER;
    
    rtl8306e_reg_get(6, 26 + port, 0, &regval);
    *pPvid = regval & 0xfff;
    *pPriority = (regval & (0x7 << 13)) >> 13;
    *pDei = 0;

    return RT_ERR_OK;
}
#endif
/* Function Name:
 *      rtk_filter_igrAcl_init
 * Description:
 *       Initialize ACL 
 * Input:
 *      none
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      The API init ACL module.
 */
rtk_api_ret_t rtk_filter_igrAcl_init(void)
{
    uint32 entaddr;

    /*empty the acl table*/
    for (entaddr = 0; entaddr < RTL8306_ACL_ENTRYNUM; entaddr++ ) 
    {
        if (rtl8306e_acl_entry_set(entaddr, RTL8306_ACL_INVALID, RTL8306_ACT_DROP, RTL8306_ACL_ETHER, 0, 0) == FAILED)
            return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_filter_igrAcl_rule_add
 * Description:
 *      Add an acl rule into acl table
 * Input:
 *      pRule    -  the pointer of rule structure
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_TBL_FULL
 *      RT_ERR_NULL_POINTER
 * Note:
 *      The API add an  ACL rule. <nl>
 *      phyport could be  0~5: port number,  RTL8306_ACL_ANYPORT: any port;<nl>
 *      protocol could be RTL8306_ACL_ETHER(ether type), RTL8306_ACL_TCP(TCP), RTL8306_ACL_UDP(UDP), RTL8306_ACL_TCPUDP(TCP or UDP);<nl>
 *      prority could be RTL8306_PRIO0~RTL8306_PRIO3;<nl>
 *      action could be RTL8306_ACT_DROP/RTL8306_ACT_PERMIT/RTL8306_ACT_TRAPCPU/RTL8306_ACT_MIRROR;<nl>
 */

rtk_api_ret_t rtk_filter_igrAcl_rule_add(rtk_filter_rule_t *pRule)
{
    uint32 entaddr;
    uint32 port, pro, val, pri, act;  
    uint32 isFull ;

    if(NULL == pRule)
        return RT_ERR_NULL_POINTER;
    
    isFull = TRUE;
    
    /*if  exist an  acl entry of the same rule according by phyport, protocol,data,
     *only update priority and action
     */
    for (entaddr = 0; entaddr < RTL8306_ACL_ENTRYNUM; entaddr ++ ) 
    {
        if (rtl8306e_acl_entry_get(entaddr, &port, &act, &pro, &val, &pri) != SUCCESS)
            return RT_ERR_FAILED;

        if ((pRule->phyport == port) && (pRule->protocol == pro) && (pRule->data == val)) 
        {
            if (rtl8306e_acl_entry_set(entaddr, pRule->phyport, pRule->action, pRule->protocol, pRule->data, pRule->priority) != SUCCESS )
                 return RT_ERR_FAILED;
            else
                 return RT_ERR_OK;
        }            
    }    

    /*if not exist the rule, find an invalid entry to write it , else return table full */
    for (entaddr = 0; entaddr < RTL8306_ACL_ENTRYNUM; entaddr ++ )
    {
        if (rtl8306e_acl_entry_get(entaddr, &port, &act, &pro, &val, &pri) != SUCCESS)
            return RT_ERR_FAILED;
        
        if (port == RTL8306_ACL_INVALID) 
        {
            if (rtl8306e_acl_entry_set(entaddr, pRule->phyport, pRule->action, pRule->protocol, pRule->data, pRule->priority) != SUCCESS)
                 return RT_ERR_FAILED;
            else 
            {
                isFull = FALSE;
                break;
             }                        
        }            
    }    

    if (isFull)
        return RT_ERR_TBL_FULL;
    else
        return SUCCESS;


   
}

#ifndef CONFIG_ETHWAN

/* Function Name:
 *      rtk_filter_igrAcl_rule_get
 * Description:
 *      Get ACL rule priority and action 
 * Input:
 *      pRule    -  the pointer of rule structure
 * Output:
 *      pRule    -  the pointer of rule structure
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      The API add an  ACL rule. <nl>
 *      phyport could be  0~5: port number,  RTL8306_ACL_ANYPORT: any port;<nl>
 *      protocol could be RTL8306_ACL_ETHER(ether type), RTL8306_ACL_TCP(TCP), RTL8306_ACL_UDP(UDP), RTL8306_ACL_TCPUDP(TCP or UDP);<nl>
 *      prority could be RTL8306_PRIO0~RTL8306_PRIO3;<nl>
 *      action could be RTL8306_ACT_DROP/RTL8306_ACT_PERMIT/RTL8306_ACT_TRAPCPU/RTL8306_ACT_MIRROR;<nl>
 */
rtk_api_ret_t rtk_filter_igrAcl_rule_get(rtk_filter_rule_t *pRule)
{
    uint32 entaddr;
    uint32 port, pro, val, pri, act;  


    if(NULL == pRule)
        return RT_ERR_NULL_POINTER;

    /*if  exist an  acl entry of the same rule according by phyport, protocol,data,
     *get the priority and action
     */
    for (entaddr = 0; entaddr < RTL8306_ACL_ENTRYNUM; entaddr ++ ) 
    {
        if (rtl8306e_acl_entry_get(entaddr, &port, &act, &pro, &val, &pri) != SUCCESS)
            return RT_ERR_FAILED;

        if ((pRule->phyport == port) && (pRule->protocol == pro) && (pRule->data == val)) 
        {
                 pRule->priority = pri;
                 pRule->action = act;
                 return RT_ERR_OK;
        }            
    }    

    return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_filter_igrAcl_rule_del
 * Description:
 *      Delete an acl rule into acl table
 * Input:
 *      pRule    -  the pointer of rule structure
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      The API delete an  ACL rule. <nl>
 *      only phyport/protocol/data field in pRule needs to be specified.
 */
rtk_api_ret_t rtk_filter_igrAcl_rule_del(rtk_filter_rule_t *pRule)
{
    uint32 entaddr;
    uint32 port, pro, val, pri, act;  
    uint32 isHit;

    if(NULL == pRule)
        return RT_ERR_NULL_POINTER;
    
    isHit = FALSE;
    
    if (pRule->phyport == RTL8306_ACL_INVALID)
        return RT_ERR_INPUT;
    
    for (entaddr = 0; entaddr < RTL8306_ACL_ENTRYNUM; entaddr ++ ) 
    {
        if (rtl8306e_acl_entry_get(entaddr, &port, &act, &pro, &val, &pri) != SUCCESS)
            return RT_ERR_FAILED;
        
        if ((port == pRule->phyport) && (pro == pRule->protocol) && (val == pRule->data))
        {
            if (rtl8306e_acl_entry_set(entaddr, RTL8306_ACL_INVALID, RTL8306_ACT_DROP, RTL8306_ACL_ETHER, 0, 0) != SUCCESS)
                return RT_ERR_FAILED;
            isHit = TRUE;
            break;
        }                
    }

    if (isHit)
        return RT_ERR_OK;
    else
        return RT_ERR_INPUT;
    


}
#endif

int32 rtl8306M_ACL_Qos_Init(void)
{
    /*for xdsl Qos example setting */
    rtk_priority_select_t priDec;
    rtl8306e_qosSchPara_t schPara;
    rtk_port_phy_ability_t  ability;
    uint32 port;

        /* initialize Chip */ 
     //   rtk_switch_init(); 
   	printk("calling DSCP Qos Init \n");
        
        /* initialize QOS */ 
        rtk_qos_init(4);
       
       /*Force PHY 0 100FULL, and enable Symmetric PAUSE flow control capabilities*/
        memset(&ability, 0, sizeof(ability));
        ability.Full_100 = 1;
        ability.FC = 1;
        rtk_port_phyForceModeAbility_set(0, &ability);
        
        /*Set port0 as CPU port and not insert CPU tag to all the packets transmitted to CPU */
        rtk_cpu_tagPort_set (0, CPU_INSERT_TO_NONE);

        /*Enable port 0 Tx rate limitation, the rate is 32Mbps(500x64Kbps)*/
        // rtk_rate_egrBandwidthCtrlRate_set(0, 500, ENABLED);    //here you can modify it .

        /*initialize VLAN */ 
//        rtk_vlan_init(); 
        
        /*Set port-base priority as highest level, the others are 1*/
#if 0
        priDec.port_pri = 1;
        priDec.dot1q_pri= 1; 
        priDec.dscp_pri = 5;
        priDec.acl_pri = 1;
        priDec.vid_pri = 1;
-----------------------------------
        priDec.port_pri = 3;
        priDec.dot1q_pri= 1; 
        priDec.dscp_pri = 1;
        priDec.acl_pri = 1;
        priDec.vid_pri = 1;

#endif
	 priDec.port_pri = 4;
        priDec.dot1q_pri= 1; 
        priDec.dscp_pri = 1;
        priDec.acl_pri = 5;
        priDec.vid_pri = 1;
		
        rtk_qos_priSel_set(&priDec);

	 rtk_filter_rule_t rule;
        rtk_filter_igrAcl_init();
	
 
         /*Enable port-based priority, disable DSCP-based, 1Q-based and CPU tag priority*/
        for (port = 1; port < 5; port ++)    //Egress port is port0 ,other lan port init
        {
#if 0
            rtl8306e_qos_priSrcEnable_set(port, RTL8306_PBP_PRIO, FALSE); 
            rtl8306e_qos_priSrcEnable_set(port, RTL8306_DSCP_PRIO, TRUE); 
#else
            rtl8306e_qos_priSrcEnable_set(port, RTL8306_PBP_PRIO, TRUE); 
            rtl8306e_qos_priSrcEnable_set(port, RTL8306_DSCP_PRIO, FALSE); 
#endif
            rtl8306e_qos_priSrcEnable_set(port, RTL8306_1QBP_PRIO, FALSE);
            rtl8306e_qos_priSrcEnable_set(port, RTL8306_CPUTAG_PRIO, FALSE);  

			
		   /*set ACL rule, let pkts with udp source port = 50000 enter Q3*/
            rule.phyport = port;
            rule.protocol = RTL8306_ACL_UDP;
            rule.data = 9000; /*UDP port number: 50000*/
            rule.priority = 3;  /*ACL assign pri 3*/
            rule.action = RTL8306_ACT_PERMIT;
            rtk_filter_igrAcl_rule_add (&rule); 

		rule.phyport = port;
            rule.protocol = RTL8306_ACL_UDP;
            rule.data = 5060; /*UDP port number: 50000*/
            rule.priority = 3;  /*ACL assign pri 3*/
            rule.action = RTL8306_ACT_PERMIT;
            rtk_filter_igrAcl_rule_add (&rule); 	
            
            /*set port-based priority to 2, so non-Voip pkt enter Q2*/
            rtk_qos_portPri_set(port, 2);
            

            /* disable priority remarking on port 1~4*/
            rtk_qos_1pRemarkEnable_set(port, DISABLED);
            
        } 
  #if 0      
        /*  DSCP 0x38/0x30 Enter Q1 ,  DSCP_REG_PRI Enter Q3  */
      	rtl8306_setAsicQosDSCPUserAssignPriority(RTL8306_DSCP_USERA,0x38,1);
	      rtl8306_setAsicQosDSCPUserAssignPriority(RTL8306_DSCP_USERB,0x30,1);
        	// voice queue (q3)
	    //  rtl8306_setPrioritySourcePriority(RTL8306_PRI_DSCP,RTL8306_DSCP_REG_PRI,3);
	        rtk_qos_dscpPriRemap_set(RTL8306_DSCP_REG_PRI, 3);
#endif
        /*set schedule parameter for cpu port*/
  //      rtl8306e_qos_portSchedulingMode_set(0, 1, 0x0);  //set port0 use set1 schedule
  	  rtl8306e_qos_portSchedulingMode_set(0, 1, 0x4);  //set port0 use set1 schedule
        rtl8306e_qos_schedulingPara_get(1, &schPara);
        schPara.q0_wt = 0x1;
        schPara.q1_wt = 0x2;
        schPara.q2_wt = 0x4;
        schPara.q3_wt = 0x0;  // weight 0 means strict priority

	//  schPara.q2_n64Kbps = 0xa0;  //Q2 queue bandwidth control to 0xa0* 64Kbps = 10M bps

	 schPara.q2_n64Kbps = 0x7ff;    
	
        rtl8306e_qos_schedulingPara_set(1, schPara);
        
        /*set cpu port's flow control mode to set 1*/
        rtl8306e_qos_portFlcMode_set(0, 1);    //set port0 as set1

	/*queue threshold*/
        rtl8306e_qos_queFlcThr_set(3, RTL8306_FCO_QLEN, RTL8306_FCON, RTL8306_FCO_SET1, 9);
        rtl8306e_qos_queFlcThr_set(3, RTL8306_FCO_QLEN, RTL8306_FCOFF, RTL8306_FCO_SET1, 5);        
        rtl8306e_qos_queFlcThr_set(3, RTL8306_FCO_DSC, RTL8306_FCON, RTL8306_FCO_SET1, 40);
        rtl8306e_qos_queFlcThr_set(3, RTL8306_FCO_DSC, RTL8306_FCOFF, RTL8306_FCO_SET1, 28);	
 
}

#define rtlglue_printf printk
void switch8306info(void)
{
	uint32 enabled, port, msk, index, vlanId, priority, vid, memberPortMask, untagMask;
	rtl8306e_acceptFrameType_t accept_frame_type;

		unsigned int value;
		rtlglue_printf("[	CPU-Tag INFO	] \n"); 	
		rtl8306_getAsicPhyReg(4, 21, 0, &value);
		rtlglue_printf("enable check CPU-tag?(bit7: 0:disable, 1:enable) phy4 reg21 page0 value:0x%x\n", value);

		rtl8306_getAsicPhyReg(2, 21, 3, &value);
		rtlglue_printf("enable remove/insert cpu-tag?(bit11/bit12) phy2 reg21 page3 value:0x%x\n", value);


		rtlglue_printf("[	VLAN CONFIGURATION	INFO	] \n");
		rtl8306_getAsicVlanEnable(&enabled);
		rtlglue_printf("VLAN is %s\n", enabled==TRUE? "Enabled":"Disabled");		
		rtl8306e_vlan_tagAware_get(&enabled);
		rtlglue_printf("Tag %s, ", enabled==TRUE? "Awared":" Unawared");		
		rtl8306e_vlan_IgrFilterEnable_get(&enabled);
		rtlglue_printf("Ingressfilter %s\n", enabled == TRUE ? "Enabled":"Disabled");

		rtlglue_printf("\n");
		for(port = 0; port < RTL8306_PORT_NUMBER; port++)
		{
			rtl8306e_vlan_portAcceptFrameType_get(port, &accept_frame_type);
			rtlglue_printf("port%d support accept frame type: %s\n", port, accept_frame_type == RTL8306E_ACCEPT_ALL ? "all" : (accept_frame_type == RTL8306E_ACCEPT_TAG_ONLY ? "Tag only" : "Untag only"));
		}
			rtlglue_printf("\n");
		rtl8306e_vlan_leaky_get(RTL8306_VALN_LEAKY_ARP, &enabled);
		rtlglue_printf("ARP vlan %s, ", enabled == TRUE ? "Enabled":"Disabled");
		rtl8306e_vlan_leaky_get(RTL8306_VALN_LEAKY_MULTICAST, &enabled);
		rtlglue_printf("Multicast Vlan %s, ", enabled == TRUE ? "Enabled":"Disabled");
		rtl8306e_vlan_leaky_get(RTL8306_VALN_LEAKY_MIRROR, &enabled);
		rtlglue_printf("Mirror vlan %s, ", enabled == TRUE ? "Enabled" : "Disabled");
		rtl8306e_trap_abnormalPktAction_get(RTL8306_UNMATCHVID, &enabled);
		rtlglue_printf("VID unmatch Table pkt trapping to CPU %s\n", enabled == TRUE ? "Enabled" :"Disabled");
		rtlglue_printf("[\nVLAN PER PORT INFO]\n");
		rtlglue_printf("%7s%-19s%-19s%-19s\n", "", "NULLVID Replace", "1P remark", 
			"Tag instet mask");
		for ( port= 0 ; port < RTL8306_PORT_NUMBER; port++) 
			 {
			rtl8306e_vlan_nullVidReplace_get(port, &enabled);
			rtlglue_printf("Port%d: %-19s", port, enabled == TRUE ? "Enabled" : "Disabled");
			rtl8306e_qos_1pRemarkEnable_get(port, &enabled);
			rtlglue_printf("%-19s", enabled == TRUE ? "Enabled" : "Disabled");
			rtl8306e_vlan_tagInsert_get(port, &msk);
					rtlglue_printf("0x%-2x\n", msk);
			
		}	
		rtlglue_printf("\n[Port PVID]\n");
		for(port = 0; port < RTL8306_PORT_NUMBER; port++) {
			rtl8306e_vlan_portPvidIndex_get(port, &index);
					rtk_vlan_portPvid_get(port, &vlanId, &priority);
			rtlglue_printf("Port %d   VLAN index %2d, VID 0X%03X,  Priority %d\n", port, index, vlanId, priority);
		}				
		rtlglue_printf("\n[VLAN ENTRY]\n");
		for(index= 0; index < 16; index++) {
			rtl8306e_vlan_entry_get(index, &vid, &memberPortMask,&untagMask);
			rtlglue_printf("[%2d] VID %-4d, Member port ", index, vid);
			for(port = 0; port < RTL8306_PORT_NUMBER; port ++) {
				if(memberPortMask & (1<<port))
					rtlglue_printf(" %d", port);
			}
			rtlglue_printf("   unTag Member port ");
			for(port = 0; port < RTL8306_PORT_NUMBER; port ++) {
				if(untagMask & (1<<port))
					rtlglue_printf(" %d", port);
			}
			rtlglue_printf("\n");
	}
}

void switch8306alive(void)
{	   
	//int8 *nextToken;
	//int32 size;
	uint32 isStatic, isAuth;
	uint32 port;//, portmask, entry;
	uint8 macAddr[6];
	uint32 age, entryaddr;//, index;
	//uint8 macAddr_1[6];
	//uint32 age_1, isStatic_1, isAuth_1, port_1, portmask_1, pattern;
	//int32 funcVal;
	//uint32 i=0,j=0;
	//uint32 repeatTimes=0;
	 //uint32 mac_cnt,action;

		 for (entryaddr = 0 ; entryaddr < 2048; entryaddr++) {
					   if (rtl8306e_l2_unicastEntry_get(macAddr, entryaddr, &age, &isStatic, &isAuth, &port) == FAILED) {
					rtlglue_printf("Failed to read look up table entry %04d!\n", entryaddr);														
			return	FAILED;
					}
			   if((age!=0)&&(!(macAddr[0] & 0x1)))
				{	
					   rtlglue_printf("%04d Unicast %02X:%02X:%02X:%02X:%02X:%02X, Age-%d, %s, %s, Port%d\n",
								entryaddr,macAddr[0]&0xFF, macAddr[1]&0xFF, macAddr[2]&0xFF,
								macAddr[3]&0xFF, macAddr[4]&0xFF, macAddr[5]&0xFF, 
								age, isStatic==TRUE?"STATIC":"Dynamic", isAuth == TRUE ? "AUTH":"UNAUTH", port);
	
				}	 

				if (rtl8306e_l2_multicastEntry_get(macAddr, entryaddr, &isAuth, &port) == FAILED) {
					rtlglue_printf("Failed to read look up table entry %04d!\n", entryaddr);														
			return	;//FAILED;
					}
			  if((isAuth==1)&&((macAddr[0] & 0x1)))
				{	
					   rtlglue_printf("%04d Multicast %02X:%02X:%02X:%02X:%02X:%02X, %s, Port%d\n",
								entryaddr,macAddr[0]&0xFF, macAddr[1]&0xFF, macAddr[2]&0xFF,
								macAddr[3]&0xFF, macAddr[4]&0xFF, macAddr[5]&0xFF, 
								isAuth == TRUE ? "AUTH":"UNAUTH", port);
	
				}								   
				  
		 }						   
}

int add_rtk8306_igrAcl_rule(unsigned int port ,unsigned int udpport)
{
	rtk_filter_rule_t rule;
     

	 rule.phyport = port;
       rule.protocol = RTL8306_ACL_UDP;
       rule.data = udpport; /*UDP port number: 50000*/
       rule.priority = 3;  /*ACL assign pri 3*/
       rule.action = RTL8306_ACT_PERMIT;
       rtk_filter_igrAcl_rule_add (&rule); 	
				
}

int reset_rtk8306_igrAcl_rule_by_user(void)
{
	int port ;

	rtk_filter_igrAcl_init();
	 
}

int add_rtk8306_igrAcl_rule_by_user(int udpport)
{
	int port ;
	 
	for (port = 1; port < 5; port ++) 
	{
		add_rtk8306_igrAcl_rule(port,udpport);	
	}
}



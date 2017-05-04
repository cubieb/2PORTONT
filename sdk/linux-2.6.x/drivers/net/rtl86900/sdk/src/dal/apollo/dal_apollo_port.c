/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 39101 $
 * $Date: 2013-05-03 17:35:27 +0800 (Fri, 03 May 2013) $
 *
 * Purpose : Definition of Port API
 *
 * Feature : The file have include the following module and sub-modules
 *           (1) Parameter settings for the port-based view
 *           (2) RTCT
 */




/*
 * Include Files
 */
#include <common/rt_type.h>
#include <dal/apollo/dal_apollo.h>
#include <rtk/port.h>
#include <dal/apollo/dal_apollo_port.h>
#include <dal/apollo/dal_apollo_vlan.h>
#include <dal/apollo/raw/apollo_raw_vlan.h>
#include <dal/apollo/raw/apollo_raw_port.h>
#include <dal/apollo/raw/apollo_raw_congest.h>
#include <osal/lib.h>
#include <hal/common/miim.h>

/*
 * Symbol Definition
 */

typedef struct dal_apollo_phy_info_s {
    uint8   force_mode_speed[RTK_MAX_NUM_OF_PORTS];
    uint8   force_mode_duplex[RTK_MAX_NUM_OF_PORTS];
    uint8   force_mode_flowControl[RTK_MAX_NUM_OF_PORTS];
    uint8   auto_mode_pause[RTK_MAX_NUM_OF_PORTS];
    uint8   auto_mode_asy_pause[RTK_MAX_NUM_OF_PORTS];
} dal_apollo_phy_info_t;

/*
 * Data Declaration
 */
static uint32    port_init = INIT_NOT_COMPLETED;

static dal_apollo_phy_info_t   phy_info;

static uint32 dal_apollo_isolation_vidx_num[DAL_APOLLO_MAX_NUM_MBR_CFG];

/*
 * Function Declaration
 */

/* Function Name:
 *      dal_apollo_port_init
 * Description:
 *      Initialize port module of the specified device.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Module must be initialized before using all of APIs in this module
 */
int32
dal_apollo_port_init(void)
{
    int32 ret;
    uint32 idx;
    rtk_port_t port;
    rtk_port_phy_ability_t phy_ability;
    uint32   data;
    apollo_raw_port_ability_t mac_ability;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    port_init = INIT_COMPLETED;

    osal_memset(&phy_info, 0x00, sizeof(dal_apollo_phy_info_t));

    /* Port Isolation */
    HAL_SCAN_ALL_PORT(port)
    {
        if((ret = apollo_raw_port_isolation_set(port, DAL_APOLLO_PORT_ISO_VIDX_DEFAULT)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
            port_init = INIT_NOT_COMPLETED;
            return ret;
        }

        if((ret = apollo_raw_port_isolationL34_set(port, DAL_APOLLO_PORT_ISO_VIDX_DEFAULT)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
            port_init = INIT_NOT_COMPLETED;
            return ret;
        }
    }

    HAL_SCAN_ALL_EXT_PORT_EX_CPU(port)
    {
        if((ret = apollo_raw_extport_isolation_set(port - 1, DAL_APOLLO_PORT_ISO_VIDX_DEFAULT)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
            port_init = INIT_NOT_COMPLETED;
            return ret;
        }

        if((ret = apollo_raw_extport_isolationL34_set(port - 1, DAL_APOLLO_PORT_ISO_VIDX_DEFAULT)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
            port_init = INIT_NOT_COMPLETED;
            return ret;
        }
    }

    /* Clear Port Isolaation VLAN mbr CFG, don't clear index 31*/
    for(idx = 0; idx <= HAL_MAX_VLAN_MBRCFG_IDX() - 1; idx++)
    {
        if(dal_apollo_isolation_vidx_num[idx] > 0)
        {
            if((ret = dal_apollo_vlan_checkAndDeleteByIsolation(idx)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
                port_init = INIT_NOT_COMPLETED;
                return ret;
            }
        }
    }

    /* Port isolation to vidx usage database */
    osal_memset(dal_apollo_isolation_vidx_num, 0x00, sizeof(uint32) * DAL_APOLLO_MAX_NUM_MBR_CFG);
    dal_apollo_isolation_vidx_num[DAL_APOLLO_PORT_ISO_VIDX_DEFAULT] = 24;

    /* PHY initialization */
    phy_ability.Half_10     = ENABLED;
    phy_ability.Full_10     = ENABLED;
    phy_ability.Half_100    = ENABLED;
    phy_ability.Full_100    = ENABLED;
    phy_ability.Half_1000   = ENABLED;
    phy_ability.Full_1000   = ENABLED;
    phy_ability.FC          = ENABLED;
    phy_ability.AsyFC       = ENABLED;

    HAL_SCAN_ALL_PORT(port)
    {
        if(HAL_IS_PHY_EXIST(port))
        {
            if((ret = dal_apollo_port_phyAutoNegoAbility_set(port, &phy_ability)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                port_init = INIT_NOT_COMPLETED;
                return ret;
            }

            if((ret = dal_apollo_port_phyAutoNegoEnable_set(port, ENABLED)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                port_init = INIT_NOT_COMPLETED;
                return ret;
            }

            if((ret = dal_apollo_port_greenEnable_set(port, ENABLED)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                port_init = INIT_NOT_COMPLETED;
                return ret;
            }

            if((ret = dal_apollo_port_phyCrossOverMode_set(port, PORT_CROSSOVER_MODE_AUTO)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                port_init = INIT_NOT_COMPLETED;
                return ret;
            }
        }
    }

    HAL_SCAN_ALL_PORT(port)
    {
        if((ret = dal_apollo_port_macRemoteLoopbackEnable_set(port, DISABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            port_init = INIT_NOT_COMPLETED;
            return ret;
        }

        if((ret = dal_apollo_port_macLocalLoopbackEnable_set(port, DISABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            port_init = INIT_NOT_COMPLETED;
            return ret;
        }

        if((ret = dal_apollo_port_adminEnable_set(port, ENABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            port_init = INIT_NOT_COMPLETED;
            return ret;
        }

        if((ret = dal_apollo_port_specialCongest_set(port, 0)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            port_init = INIT_NOT_COMPLETED;
            return ret;
        }

        /* Force CPU port */
        if(port == HAL_GET_CPU_PORT())
        {
            osal_memset(&mac_ability, 0x00, sizeof(apollo_raw_port_ability_t));
            mac_ability.speed           = PORT_SPEED_1000M;
            mac_ability.duplex          = PORT_FULL_DUPLEX;
        	mac_ability.linkFib1g       = DISABLED;
        	mac_ability.linkStatus      = PORT_LINKDOWN;
        	mac_ability.txFc            = DISABLED;
        	mac_ability.rxFc            = DISABLED;
        	mac_ability.nwayAbility     = DISABLED;
        	mac_ability.masterMod       = DISABLED;
        	mac_ability.nwayFault       = DISABLED;
        	mac_ability.lpi_100m        = DISABLED;
        	mac_ability.lpi_giga        = DISABLED;
            if((ret = apollo_raw_port_ForceAbility_set(port, &mac_ability)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                port_init = INIT_NOT_COMPLETED;
                return ret;
            }

            mac_ability.linkStatus      = PORT_LINKUP;
            if((ret = apollo_raw_port_ForceAbility_set(port, &mac_ability)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                port_init = INIT_NOT_COMPLETED;
                return ret;
            }

            if((ret = reg_field_read(ABLTY_FORCE_MODEr, ABLTY_FORCE_MODEf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                port_init = INIT_NOT_COMPLETED;
                return ret;
            }

            data |= (0x01 << port);

            if((ret = reg_field_write(ABLTY_FORCE_MODEr, ABLTY_FORCE_MODEf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                port_init = INIT_NOT_COMPLETED;
                return ret;
            }
        }
        else
        {
            osal_memset(&mac_ability, 0x00, sizeof(apollo_raw_port_ability_t));
        	mac_ability.linkStatus      = PORT_LINKDOWN;
            if((ret = apollo_raw_port_ForceAbility_set(port, &mac_ability)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                port_init = INIT_NOT_COMPLETED;
                return ret;
            }

            if((ret = reg_field_read(ABLTY_FORCE_MODEr, ABLTY_FORCE_MODEf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                port_init = INIT_NOT_COMPLETED;
                return ret;
            }

            data &= ~(0x01 << port);

            if((ret = reg_field_write(ABLTY_FORCE_MODEr, ABLTY_FORCE_MODEf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                port_init = INIT_NOT_COMPLETED;
                return ret;
            }
        }

        /* Disable extension port */
        if(port == HAL_GET_RGMII_PORT())
        {
            if((ret = dal_apollo_port_macExtMode_set(port, EXT_MODE_DISABLE)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                port_init = INIT_NOT_COMPLETED;
                return ret;
            }

            if((ret = dal_apollo_port_macExtRgmiiDelay_set(port, 0, 0)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                port_init = INIT_NOT_COMPLETED;
                return ret;
            }
        }
    }

#if defined(CONFIG_GPON_FEATURE)
    /* Do nothing */
#else
    /* PON MAC select phy 4 enable*/
    /*default MAC3 init to UTP mode*/
    data = 1;
    if ((ret = reg_field_write(CHIP_INF_SELr, PHY4_ENf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return ret;
    }
    data = 0;
    if ((ret = reg_field_write(CHIP_INF_SELr, CKGPHY_SELf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return ret;
    }
#endif

    return RT_ERR_OK;
} /* end of dal_apollo_port_init */

/* Module Name    : Port                                       */
/* Sub-module Name: Parameter settings for the port-based view */

/* Function Name:
 *      dal_apollo_port_link_get
 * Description:
 *      Get the link status of the specific port
 * Input:
 *      port    - port id
 * Output:
 *      pStatus - pointer to the link status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The link status of the port is as following:
 *      - LINKDOWN
 *      - LINKUP
 */
int32
dal_apollo_port_link_get(rtk_port_t port, rtk_port_linkStatus_t *pStatus)
{
    int32   ret;
    uint32  data;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pStatus), RT_ERR_NULL_POINTER);

    if ((ret = reg_array_field_read(P_ABLTYr, (int32)port, REG_ARRAY_INDEX_NONE, P_LINK_STATUSf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return ret;
    }

    *pStatus = ((data == 1) ? PORT_LINKUP : PORT_LINKDOWN);
    return RT_ERR_OK;
} /* end of dal_apollo_port_link_get */

/* Function Name:
 *      dal_apollo_port_speedDuplex_get
 * Description:
 *      Get the negotiated port speed and duplex status of the specific port
 * Input:
 *      port    - port id
 * Output:
 *      pSpeed  - pointer to the port speed
 *      pDuplex - pointer to the port duplex
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 *      RT_ERR_PORT_LINKDOWN - link down port status
 * Note:
 *      (1) The speed type of the port is as following:
 *          - PORT_SPEED_10M
 *          - PORT_SPEED_100M
 *          - PORT_SPEED_1000M
 *
 *      (2) The duplex mode of the port is as following:
 *          - HALF_DUPLEX
 *          - FULL_DUPLEX
 */
int32
dal_apollo_port_speedDuplex_get(
    rtk_port_t        port,
    rtk_port_speed_t  *pSpeed,
    rtk_port_duplex_t *pDuplex)
{
    int32  ret;
    uint32 speed;
    uint32 duplex;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pSpeed), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pDuplex), RT_ERR_NULL_POINTER);

    if ((ret = reg_array_field_read(P_ABLTYr, (int32)port, REG_ARRAY_INDEX_NONE, P_LINK_SPDf, &speed)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return ret;
    }

    if ((ret = reg_array_field_read(P_ABLTYr, (int32)port, REG_ARRAY_INDEX_NONE, P_DUPLEXf, &duplex)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return ret;
    }

    switch(speed)
    {
        case 0:
            *pSpeed = PORT_SPEED_10M;
            break;
        case 1:
            *pSpeed = PORT_SPEED_100M;
            break;
        case 2:
            *pSpeed = PORT_SPEED_1000M;
            break;
        default:
            return RT_ERR_FAILED;
            break;
    }

    *pDuplex = ((1 == duplex) ? PORT_FULL_DUPLEX : PORT_HALF_DUPLEX);
    return RT_ERR_OK;
} /* end of dal_apollo_port_speedDuplex_get */

/* Function Name:
 *      dal_apollo_port_flowctrl_get
 * Description:
 *      Get the negotiated flow control status of the specific port
 * Input:
 *      port      - port id
 * Output:
 *      pTxStatus - pointer to the negotiation result of the Tx flow control
 *      pRxStatus - pointer to the negotiation result of the Rx flow control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 *      RT_ERR_PORT_LINKDOWN - link down port status
 * Note:
 *      None
 */
int32
dal_apollo_port_flowctrl_get(
    rtk_port_t  port,
    uint32      *pTxStatus,
    uint32      *pRxStatus)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pTxStatus), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pRxStatus), RT_ERR_NULL_POINTER);

    if ((ret = reg_array_field_read(P_ABLTYr, (int32)port, REG_ARRAY_INDEX_NONE, P_TX_FCf, pTxStatus)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return ret;
    }

    if ((ret = reg_array_field_read(P_ABLTYr, (int32)port, REG_ARRAY_INDEX_NONE, P_RX_FCf, pRxStatus)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_apollo_port_flowctrl_get */

/* Function Name:
 *      dal_apollo_port_phyAutoNegoEnable_get
 * Description:
 *      Get PHY ability of the specific port
 * Input:
 *      port    - port id
 * Output:
 *      pEnable - pointer to PHY auto negotiation status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_port_phyAutoNegoEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    int32  ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    if ((ret = phy_autoNegoEnable_get(port, pEnable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_apollo_port_phyAutoNegoEnable_get */

/* Function Name:
 *      dal_apollo_port_phyAutoNegoEnable_set
 * Description:
 *      Set PHY ability of the specific port
 * Input:
 *      port   - port id
 *      enable - enable PHY auto negotiation
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - input parameter out of range
 * Note:
 *      - ENABLED : switch to PHY auto negotiation mode
 *      - DISABLED: switch to PHY force mode
 *      - Once the abilities of both auto-nego and force mode are set,
 *        you can freely swtich the mode without calling ability setting API again
 */
int32
dal_apollo_port_phyAutoNegoEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    int32  ret;
    rtk_port_phy_ability_t ability;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    if (ENABLED == enable)
    {
        if ((ret = phy_autoNegoAbility_get(port, &ability)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        ability.FC = phy_info.auto_mode_pause[port];
        ability.AsyFC = phy_info.auto_mode_asy_pause[port];

        if ((ret = phy_autoNegoAbility_set(port, &ability)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    if ((ret = phy_autoNegoEnable_set(port, enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if (DISABLED == enable)
    {
        if ((ret = phy_duplex_set(port, phy_info.force_mode_duplex[port])) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        if ((ret = phy_speed_set(port, phy_info.force_mode_speed[port])) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        if ((ret = phy_autoNegoAbility_get(port, &ability)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        ability.FC = phy_info.force_mode_flowControl[port];
        ability.AsyFC = phy_info.force_mode_flowControl[port];

        if ((ret = phy_autoNegoAbility_set(port, &ability)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of dal_apollo_port_phyAutoNegoEnable_set */

/* Function Name:
 *      dal_apollo_port_phyAutoNegoAbility_get
 * Description:
 *      Get PHY auto negotiation ability of the specific port
 * Input:
 *      port    - port id
 * Output:
 *      pAbility - pointer to the PHY ability
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_port_phyAutoNegoAbility_get(
    rtk_port_t              port,
    rtk_port_phy_ability_t  *pAbility)
{
    int32  ret;
    rtk_enable_t enable;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pAbility), RT_ERR_NULL_POINTER);

    if ((ret = dal_apollo_port_phyAutoNegoEnable_get(port, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL | MOD_PORT), "");
        return ret;
    }

    if ((ret = phy_autoNegoAbility_get(port, pAbility)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return ret;
    }

    if (DISABLED == enable)
    {
        /* Force mode currently, return database status */
        pAbility->FC    = phy_info.auto_mode_pause[port];
        pAbility->AsyFC = phy_info.auto_mode_asy_pause[port];
    }

    return RT_ERR_OK;
} /* end of dal_apollo_port_phyAutoNegoAbility_get */

/* Function Name:
 *      dal_apollo_port_phyAutoNegoAbility_set
 * Description:
 *      Set PHY auto negotiation ability of the specific port
 * Input:
 *      port     - port id
 *      pAbility - pointer to the PHY ability
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      You can set these abilities no matter which mode PHY currently stays on
 */
int32
dal_apollo_port_phyAutoNegoAbility_set(
    rtk_port_t              port,
    rtk_port_phy_ability_t  *pAbility)
{
    int32   ret;
    rtk_enable_t enable;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pAbility), RT_ERR_NULL_POINTER);

    if ((ret = dal_apollo_port_phyAutoNegoEnable_get(port, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL | MOD_PORT), "dal_cypress_port_phyAutoNegoEnable_get(port=%d) failed!!", port);
        return ret;
    }

    if (DISABLED == enable)
    {
        /* PHY is in force mode currently, keep user configuration and set Force ability */
        phy_info.auto_mode_pause[port] = pAbility->FC;
        phy_info.auto_mode_asy_pause[port] = pAbility->AsyFC;
        pAbility->FC = phy_info.force_mode_flowControl[port];
        pAbility->AsyFC = phy_info.force_mode_flowControl[port];
    }

    if ((ret = phy_autoNegoAbility_set(port, pAbility)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PORT | MOD_DAL), "");
        return ret;
    }

    if (ENABLED == enable)
    {
        /* PHY is in auto-nego mode, store user configuration to database */
        phy_info.auto_mode_pause[port] = pAbility->FC;
        phy_info.auto_mode_asy_pause[port] = pAbility->AsyFC;
    }

    return RT_ERR_OK;
} /* end of dal_apollo_port_phyAutoNegoAbility_set */

/* Function Name:
 *      dal_apollo_port_phyForceModeAbility_get
 * Description:
 *      Get PHY ability status of the specific port
 * Input:
 *      port         - port id
 * Output:
 *      pSpeed       - pointer to the port speed
 *      pDuplex      - pointer to the port duplex
 *      pFlowControl - pointer to the flow control enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None.
 */
int32
dal_apollo_port_phyForceModeAbility_get(
    rtk_port_t          port,
    rtk_port_speed_t    *pSpeed,
    rtk_port_duplex_t   *pDuplex,
    rtk_enable_t        *pFlowControl)
{
    int32   ret;
    rtk_enable_t enable;
    rtk_port_phy_ability_t ability;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pSpeed), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pDuplex), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pFlowControl), RT_ERR_NULL_POINTER);

    if ((ret = dal_apollo_port_phyAutoNegoEnable_get(port, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL | MOD_PORT), "");
        return ret;
    }

    if (ENABLED == enable)
    {
        *pSpeed = phy_info.force_mode_speed[port];
        *pDuplex = phy_info.force_mode_duplex[port];
        *pFlowControl = phy_info.force_mode_flowControl[port];
    }
    else
    {
        if ((ret = phy_speed_get(port, pSpeed)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        if ((ret = phy_duplex_get(port, pDuplex)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        if ((ret = phy_autoNegoAbility_get(port, &ability)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        *pFlowControl = ability.FC;
    }

    return RT_ERR_OK;
} /* end of dal_apollo_port_phyForceModeAbility_get */

/* Function Name:
 *      dal_apollo_port_phyForceModeAbility_set
 * Description:
 *      Set the port speed/duplex mode/pause/asy_pause in the PHY force mode
 * Input:
 *      port        - port id
 *      speed       - port speed
 *      duplex      - port duplex mode
 *      flowControl - enable flow control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_PHY_SPEED  - invalid PHY speed setting
 *      RT_ERR_PHY_DUPLEX - invalid PHY duplex setting
 *      RT_ERR_INPUT      - invalid input parameter
 * Note:
 *      (1) You can set these abilities no matter which mode PHY stays currently.
 *          However, these abilities only take effect when the PHY is in Force mode
 *
 *      (2) The speed type of the port is as following:
 *          - PORT_SPEED_10M
 *          - PORT_SPEED_100M
 *          - PORT_SPEED_1000M
 *
 *      (3) The duplex mode of the port is as following:
 *          - HALF_DUPLEX
 *          - FULL_DUPLEX
 */
int32
dal_apollo_port_phyForceModeAbility_set(
    rtk_port_t          port,
    rtk_port_speed_t    speed,
    rtk_port_duplex_t   duplex,
    rtk_enable_t        flowControl)
{

    int32   ret;
    rtk_enable_t enable;
    rtk_port_phy_ability_t ability;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((PORT_SPEED_END <= speed), RT_ERR_INPUT);
    RT_PARAM_CHK((PORT_DUPLEX_END <= duplex), RT_ERR_INPUT);
    RT_PARAM_CHK((RTK_ENABLE_END <= flowControl), RT_ERR_INPUT);

    if ((ret = dal_apollo_port_phyAutoNegoEnable_get(port, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL | MOD_PORT), "");
        return ret;
    }

    phy_info.force_mode_speed[port] = speed;
    phy_info.force_mode_duplex[port] = duplex;
    phy_info.force_mode_flowControl[port] = flowControl;

    if (DISABLED == enable)
    {
        if ((ret = phy_speed_set(port, speed)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        if ((ret = phy_duplex_set(port, duplex)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        if ((ret = phy_autoNegoAbility_get(port, &ability)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        if (ENABLED == flowControl)
        {
            ability.FC = ENABLED;
            ability.AsyFC = ENABLED;
        }
        else
        {
            ability.FC = DISABLED;
            ability.AsyFC = DISABLED;
        }

        if ((ret = phy_autoNegoAbility_set(port, &ability)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of dal_apollo_port_phyForceModeAbility_set */

/* Function Name:
 *      dal_apollo_port_phyReg_get
 * Description:
 *      Get PHY register data of the specific port
 * Input:
 *      port  - port id
 *      page  - page id
 *      reg   - reg id
 * Output:
 *      pData - pointer to the PHY reg data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid page id
 *      RT_ERR_PHY_REG_ID   - invalid reg id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_port_phyReg_get(
    rtk_port_t          port,
    uint32              page,
    rtk_port_phy_reg_t  reg,
    uint32              *pData)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((page > HAL_MIIM_PAGE_ID_MAX()), RT_ERR_INPUT);
    RT_PARAM_CHK(((page < HAL_MIIM_PAGE_ID_MIN()) && page != 0), RT_ERR_INPUT);
    RT_PARAM_CHK((reg > HAL_MIIM_REG_ID_MAX()), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    /* Check Link status */
    ret = hal_miim_read(port, page, reg, pData);
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pData=0x%x", *pData);

    return ret;
} /* end of dal_apollo_port_phyReg_get */

/* Function Name:
 *      dal_apollo_port_phyReg_set
 * Description:
 *      Set PHY register data of the specific port
 * Input:
 *      port - port id
 *      page - page id
 *      reg  - reg id
 *      data - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID     - invalid port id
 *      RT_ERR_PHY_PAGE_ID - invalid page id
 *      RT_ERR_PHY_REG_ID  - invalid reg id
 * Note:
 *      None
 */
int32
dal_apollo_port_phyReg_set(
    rtk_port_t          port,
    uint32              page,
    rtk_port_phy_reg_t  reg,
    uint32              data)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((page > HAL_MIIM_PAGE_ID_MAX()), RT_ERR_INPUT);
    RT_PARAM_CHK(((page < HAL_MIIM_PAGE_ID_MIN()) && page != 0), RT_ERR_INPUT);
    RT_PARAM_CHK((reg > HAL_MIIM_REG_ID_MAX()), RT_ERR_INPUT);
    RT_PARAM_CHK((data > HAL_MIIM_DATA_MAX()), RT_ERR_INPUT);

    ret = hal_miim_write(port, page, reg, data);

    return ret;
} /* end of dal_apollo_port_phyReg_set */

/* Function Name:
 *      dal_apollo_port_phyMasterSlave_get
 * Description:
 *      Get PHY configuration of master/slave mode of the specific port
 * Input:
 *      port                - port id
 * Output:
 *      pMasterSlave        - pointer to the PHY master slave configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_port_phyMasterSlave_get(
    rtk_port_t          port,
    rtk_port_masterSlave_t   *pMasterSlave)
{
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pMasterSlave), RT_ERR_NULL_POINTER);

    *pMasterSlave = PORT_AUTO_MODE;
    return RT_ERR_OK;
}/* end of dal_apollo_port_phyMasterSlave_get */

/* Function Name:
 *      dal_apollo_port_phyMasterSlave_set
 * Description:
 *      Set PHY configuration of master/slave mode of the specific port
 * Input:
 *      port                - port id
 *      masterSlave         - PHY master slave configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - RT_ERR_INPUT
 * Note:
 *      None
 */
int32
dal_apollo_port_phyMasterSlave_set(
    rtk_port_t          port,
    rtk_port_masterSlave_t   masterSlave)
{
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(masterSlave >= PORT_MASTER_SLAVE_END, RT_ERR_INPUT);


    return RT_ERR_OK;
}/* end of dal_apollo_port_phyMasterSlave_set */

/* Function Name:
 *      dal_apollo_port_phyTestMode_get
 * Description:
 *      Get PHY in which test mode.
 * Input:
 *      port - Port id.
 * Output:
 *      pTestMode - PHY test mode 0:normal 1:test mode 1 2:test mode 2 3: test mode 3 4:test mode 4 5~7:reserved
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_PORT_ID 			- Invalid port number.
 *      RT_ERR_INPUT 			- Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      Get test mode of PHY from register setting 9.15 to 9.13.
 */
int32
dal_apollo_port_phyTestMode_get(rtk_port_t port, rtk_port_phy_test_mode_t *pTestMode)
{
    int32 ret;
    uint32 data;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "port=%d",port);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pTestMode), RT_ERR_NULL_POINTER);

    /* function body */
    if((ret = dal_apollo_port_phyReg_get(port, 0xA41, 9, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    *pTestMode = (data & 0xE000) >> 13;

    return RT_ERR_OK;
}   /* end of dal_apollo_port_phyTestMode_get */

/* Function Name:
 *      dal_apollo_port_phyTestMode_set
 * Description:
 *      Set PHY in test mode.
 * Input:
 *      port - port id.
 *      testMode - PHY test mode 0:normal 1:test mode 1 2:test mode 2 3: test mode 3 4:test mode 4 5~7:reserved
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_PORT_ID 			- Invalid port number.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      None.
 */
int32
dal_apollo_port_phyTestMode_set(rtk_port_t port, rtk_port_phy_test_mode_t testMode)
{
    int32 ret;
    uint32 data;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "port=%d,testMode=%d",port, testMode);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((PHY_TEST_MODE_END <= testMode), RT_ERR_INPUT);

    /* Test mode 2 & 3 are not supported */
    if( (PHY_TEST_MODE_2 == testMode) || (PHY_TEST_MODE_3 == testMode) )
    {
        return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    /* Set test mode */
    if((ret = dal_apollo_port_phyReg_get(port, 0xA41, 9, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    data &= 0x1FFF;
    data |= (testMode << 13);

    if((ret = dal_apollo_port_phyReg_set(port, 0xA41, 9, data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    return RT_ERR_OK;
}   /* end of dal_apollo_port_phyTestMode_set */

/* Function Name:
 *      dal_apollo_port_cpuPortId_get
 * Description:
 *      Get CPU port id
 * Input:
 *      None
 * Output:
 *      pPort - pointer to CPU port id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_port_cpuPortId_get(rtk_port_t *pPort)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pPort), RT_ERR_NULL_POINTER);

    *pPort = (rtk_port_t)HAL_GET_CPU_PORT();
    return RT_ERR_OK;
} /* end of dal_apollo_port_cpuPortId_get */

/* Function Name:
 *      dal_apollo_port_isolation_get
 * Description:
 *      Get the portmask of the port isolation
 * Input:
 *      port      - port id
 * Output:
 *      pPortmask       - pointer to the portmask
 *      pExtPortmask    - pointer to extension portmask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      (1) Default value of each port is 1
 *      (2) Enable port isolation in the certain ports if relative portmask bits are set to 1
 */
int32
dal_apollo_port_isolation_get(rtk_port_t port, rtk_portmask_t *pPortmask, rtk_portmask_t *pExtPortmask)
{
    int32   ret;
    uint32  old_vidx;
    apollo_raw_vlanconfig_t vlan_mbrcfg;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pExtPortmask), RT_ERR_NULL_POINTER);

    /* Get original vidx */
    if((ret = apollo_raw_port_isolation_get(port, &old_vidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* Get current port mask */
    osal_memset(&vlan_mbrcfg, 0x00, sizeof(apollo_raw_vlanconfig_t));
    vlan_mbrcfg.index = old_vidx;
    if((ret = apollo_raw_vlan_memberConfig_get(&vlan_mbrcfg)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    RTK_PORTMASK_ASSIGN((*pPortmask), vlan_mbrcfg.mbr);
    RTK_PORTMASK_ASSIGN((*pExtPortmask), vlan_mbrcfg.exMbr);
    return RT_ERR_OK;
} /* end of dal_apollo_port_isolation_get */

/* Function Name:
 *      dal_apollo_port_isolation_set
 * Description:
 *      Set the portmask of the port isolation
 * Input:
 *      port            - port id
 *      pPortmask       - pointer to the portmask
 *      pExtPortmask    - pointer to extension portmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID   - invalid port id
 *      RT_ERR_PORT_MASK - invalid port mask
 * Note:
 *      (1) Default value of each port is 1
 *      (2) Enable port isolation in the certain ports if relative portmask bits are set to 1
 */
int32
dal_apollo_port_isolation_set(rtk_port_t port, rtk_portmask_t *pPortmask, rtk_portmask_t *pExtPortmask)
{
    int32 ret;
    uint32 vidx;
    uint32 old_vidx;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORTMASK_VALID((*pPortmask)), RT_ERR_PORT_MASK);
    RT_PARAM_CHK((NULL == pExtPortmask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_EXTPORTMASK_VALID((*pExtPortmask)), RT_ERR_PORT_MASK);

    /* Try to get a vidx from VLAN module */
    if((ret = dal_apollo_vlan_checkAndCreateByIsolation(pPortmask, pExtPortmask, &vidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* Get original vidx */
    if((ret = apollo_raw_port_isolation_get(port, &old_vidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* if vidx == old_vidx, nothing to do, return. */
    if(vidx == old_vidx)
    {
        return RT_ERR_OK;
    }

    /* decrease usage number in database */
    dal_apollo_isolation_vidx_num[old_vidx]--;

    /* Check old vidx member config usage, if not port use, delete it! */
    if(0 == dal_apollo_isolation_vidx_num[old_vidx])
    {
        if((ret = dal_apollo_vlan_checkAndDeleteByIsolation(old_vidx)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    /* set to new vidx */
    if((ret = apollo_raw_port_isolation_set(port, vidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* increase usage number in database */
    dal_apollo_isolation_vidx_num[vidx]++;

    return RT_ERR_OK;
} /* end of dal_apollo_port_isolation_set */

/* Function Name:
 *      dal_apollo_port_isolationExt_get
 * Description:
 *      Get the portmask of the extension port isolation
 * Input:
 *      extPort      - extension port id
 * Output:
 *      pPortmask       - pointer to the portmask
 *      pExtPortmask    - pointer to extension portmask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      (1) Default value of each port is 1
 *      (2) Enable port isolation in the certain ports if relative portmask bits are set to 1
 */
int32
dal_apollo_port_isolationExt_get(rtk_port_t extPort, rtk_portmask_t *pPortmask, rtk_portmask_t *pExtPortmask)
{
    int32   ret;
    uint32 old_vidx;
    apollo_raw_vlanconfig_t vlan_mbrcfg;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_EXT_PORT(extPort), RT_ERR_PORT_ID);
    RT_PARAM_CHK((HAL_GET_EXT_CPU_PORT() == extPort), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pExtPortmask), RT_ERR_NULL_POINTER);

    /* Get original vidx */
    if((ret = apollo_raw_extport_isolation_get(extPort - 1, &old_vidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* Get current port mask */
    osal_memset(&vlan_mbrcfg, 0x00, sizeof(apollo_raw_vlanconfig_t));
    vlan_mbrcfg.index = old_vidx;
    if((ret = apollo_raw_vlan_memberConfig_get(&vlan_mbrcfg)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    RTK_PORTMASK_ASSIGN((*pPortmask), vlan_mbrcfg.mbr);
    RTK_PORTMASK_ASSIGN((*pExtPortmask), vlan_mbrcfg.exMbr);
    return RT_ERR_OK;
} /* end of dal_apollo_port_isolationExt_get */

/* Function Name:
 *      dal_apollo_port_isolationExt_set
 * Description:
 *      Set the portmask of the extension port isolation
 * Input:
 *      extPort   - extension port id
 *      pPortmask - pointer to the portmask
 *      pExtPortmask    - pointer to extension portmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID   - invalid port id
 *      RT_ERR_PORT_MASK - invalid port mask
 * Note:
 *      (1) Default value of each port is 1
 *      (2) Enable port isolation in the certain ports if relative portmask bits are set to 1
 */
int32
dal_apollo_port_isolationExt_set(rtk_port_t extPort, rtk_portmask_t *pPortmask, rtk_portmask_t *pExtPortmask)
{
    int32   ret;
    uint32 vidx;
    uint32 old_vidx;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_EXT_PORT(extPort), RT_ERR_PORT_ID);
    RT_PARAM_CHK((HAL_GET_EXT_CPU_PORT() == extPort), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORTMASK_VALID((*pPortmask)), RT_ERR_PORT_MASK);
    RT_PARAM_CHK((NULL == pExtPortmask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_EXTPORTMASK_VALID((*pExtPortmask)), RT_ERR_PORT_MASK);

    /* Try to get a vidx from VLAN module */
    if((ret = dal_apollo_vlan_checkAndCreateByIsolation(pPortmask, pExtPortmask, &vidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* Get original vidx */
    if((ret = apollo_raw_extport_isolation_get(extPort - 1, &old_vidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* if vidx == old_vidx, nothing to do, return. */
    if(vidx == old_vidx)
    {
        return RT_ERR_OK;
    }

    /* decrease usage number in database */
    dal_apollo_isolation_vidx_num[old_vidx]--;

    /* Check old vidx member config usage, if not port use, delete it! */
    if(0 == dal_apollo_isolation_vidx_num[old_vidx])
    {
        if((ret = dal_apollo_vlan_checkAndDeleteByIsolation(old_vidx)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    /* set to new vidx */
    if((ret = apollo_raw_extport_isolation_set(extPort - 1, vidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* increase usage number in database */
    dal_apollo_isolation_vidx_num[vidx]++;

    return RT_ERR_OK;
} /* end of dal_apollo_port_isolationExt_set */

/* Function Name:
 *      dal_apollo_port_isolationL34_get
 * Description:
 *      Get the portmask of the port isolation for L34 packet
 * Input:
 *      port      - port id
 * Output:
 *      pPortmask       - pointer to the portmask
 *      pExtPortmask    - pointer to extension portmask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      (1) Default value of each port is 1
 *      (2) Enable port isolation in the certain ports if relative portmask bits are set to 1
 */
int32
dal_apollo_port_isolationL34_get(rtk_port_t port, rtk_portmask_t *pPortmask, rtk_portmask_t *pExtPortmask)
{
    int32   ret;
    uint32  old_vidx;
    apollo_raw_vlanconfig_t vlan_mbrcfg;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pExtPortmask), RT_ERR_NULL_POINTER);

    /* Get original vidx */
    if((ret = apollo_raw_port_isolationL34_get(port, &old_vidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* Get current port mask */
    osal_memset(&vlan_mbrcfg, 0x00, sizeof(apollo_raw_vlanconfig_t));
    vlan_mbrcfg.index = old_vidx;
    if((ret = apollo_raw_vlan_memberConfig_get(&vlan_mbrcfg)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    RTK_PORTMASK_ASSIGN((*pPortmask), vlan_mbrcfg.mbr);
    RTK_PORTMASK_ASSIGN((*pExtPortmask), vlan_mbrcfg.exMbr);
    return RT_ERR_OK;
} /* end of dal_apollo_port_isolationL34_get */

/* Function Name:
 *      dal_apollo_port_isolationL34_set
 * Description:
 *      Set the portmask of the port isolation for L34 packet
 * Input:
 *      port            - port id
 *      pPortmask       - pointer to the portmask
 *      pExtPortmask    - pointer to extension portmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID   - invalid port id
 *      RT_ERR_PORT_MASK - invalid port mask
 * Note:
 *      (1) Default value of each port is 1
 *      (2) Enable port isolation in the certain ports if relative portmask bits are set to 1
 */
int32
dal_apollo_port_isolationL34_set(rtk_port_t port, rtk_portmask_t *pPortmask, rtk_portmask_t *pExtPortmask)
{
    int32 ret;
    uint32 vidx;
    uint32 old_vidx;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORTMASK_VALID((*pPortmask)), RT_ERR_PORT_MASK);
    RT_PARAM_CHK((NULL == pExtPortmask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_EXTPORTMASK_VALID((*pExtPortmask)), RT_ERR_PORT_MASK);

    /* Try to get a vidx from VLAN module */
    if((ret = dal_apollo_vlan_checkAndCreateByIsolation(pPortmask, pExtPortmask, &vidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* Get original vidx */
    if((ret = apollo_raw_port_isolationL34_get(port, &old_vidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* if vidx == old_vidx, nothing to do, return. */
    if(vidx == old_vidx)
    {
        return RT_ERR_OK;
    }

    /* decrease usage number in database */
    dal_apollo_isolation_vidx_num[old_vidx]--;

    /* Check old vidx member config usage, if not port use, delete it! */
    if(0 == dal_apollo_isolation_vidx_num[old_vidx])
    {
        if((ret = dal_apollo_vlan_checkAndDeleteByIsolation(old_vidx)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    /* set to new vidx */
    if((ret = apollo_raw_port_isolationL34_set(port, vidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* increase usage number in database */
    dal_apollo_isolation_vidx_num[vidx]++;

    return RT_ERR_OK;
} /* end of dal_apollo_port_isolationL34_set */

/* Function Name:
 *      dal_apollo_port_isolationExtL34_get
 * Description:
 *      Get the portmask of the extension port isolation for L34 packet
 * Input:
 *      extPort      - extension port id
 * Output:
 *      pPortmask       - pointer to the portmask
 *      pExtPortmask    - pointer to extension portmask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      (1) Default value of each port is 1
 *      (2) Enable port isolation in the certain ports if relative portmask bits are set to 1
 */
int32
dal_apollo_port_isolationExtL34_get(rtk_port_t extPort, rtk_portmask_t *pPortmask, rtk_portmask_t *pExtPortmask)
{
    int32   ret;
    uint32 old_vidx;
    apollo_raw_vlanconfig_t vlan_mbrcfg;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_EXT_PORT(extPort), RT_ERR_PORT_ID);
    RT_PARAM_CHK((HAL_GET_EXT_CPU_PORT() == extPort), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pExtPortmask), RT_ERR_NULL_POINTER);

    /* Get original vidx */
    if((ret = apollo_raw_extport_isolationL34_get(extPort - 1, &old_vidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* Get current port mask */
    osal_memset(&vlan_mbrcfg, 0x00, sizeof(apollo_raw_vlanconfig_t));
    vlan_mbrcfg.index = old_vidx;
    if((ret = apollo_raw_vlan_memberConfig_get(&vlan_mbrcfg)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    RTK_PORTMASK_ASSIGN((*pPortmask), vlan_mbrcfg.mbr);
    RTK_PORTMASK_ASSIGN((*pExtPortmask), vlan_mbrcfg.exMbr);
    return RT_ERR_OK;
} /* end of dal_apollo_port_isolationExtL34_get */

/* Function Name:
 *      dal_apollo_port_isolationExtL34_set
 * Description:
 *      Set the portmask of the extension port isolation for L34 packet
 * Input:
 *      extPort   - extension port id
 *      pPortmask - pointer to the portmask
 *      pExtPortmask    - pointer to extension portmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID   - invalid port id
 *      RT_ERR_PORT_MASK - invalid port mask
 * Note:
 *      (1) Default value of each port is 1
 *      (2) Enable port isolation in the certain ports if relative portmask bits are set to 1
 */
int32
dal_apollo_port_isolationExtL34_set(rtk_port_t extPort, rtk_portmask_t *pPortmask, rtk_portmask_t *pExtPortmask)
{
    int32   ret;
    uint32 vidx;
    uint32 old_vidx;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_EXT_PORT(extPort), RT_ERR_PORT_ID);
    RT_PARAM_CHK((HAL_GET_EXT_CPU_PORT() == extPort), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORTMASK_VALID((*pPortmask)), RT_ERR_PORT_MASK);
    RT_PARAM_CHK((NULL == pExtPortmask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_EXTPORTMASK_VALID((*pExtPortmask)), RT_ERR_PORT_MASK);

    /* Try to get a vidx from VLAN module */
    if((ret = dal_apollo_vlan_checkAndCreateByIsolation(pPortmask, pExtPortmask, &vidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* Get original vidx */
    if((ret = apollo_raw_extport_isolationL34_get(extPort - 1, &old_vidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* if vidx == old_vidx, nothing to do, return. */
    if(vidx == old_vidx)
    {
        return RT_ERR_OK;
    }

    /* decrease usage number in database */
    dal_apollo_isolation_vidx_num[old_vidx]--;

    /* Check old vidx member config usage, if not port use, delete it! */
    if(0 == dal_apollo_isolation_vidx_num[old_vidx])
    {
        if((ret = dal_apollo_vlan_checkAndDeleteByIsolation(old_vidx)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    /* set to new vidx */
    if((ret = apollo_raw_extport_isolationL34_set(extPort - 1, vidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* increase usage number in database */
    dal_apollo_isolation_vidx_num[vidx]++;

    return RT_ERR_OK;
} /* end of dal_apollo_port_isolationExtL34_set */


/* Function Name:
 *      dal_apollo_port_isolationIpmcLeaky_get
 * Description:
 *      Get the ip multicast leaky state of the port isolation
 * Input:
 *      port      - port id
 * Output:
 *      pEnable   - status of port isolation leaky for ip multicast packets
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      none
 */
int32
dal_apollo_port_isolationIpmcLeaky_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  val;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    if ((ret = reg_array_field_read(L2_IPMC_ISO_LEAKYr,port,REG_ARRAY_INDEX_NONE,ENf,&val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PORT|MOD_DAL), "");
        return ret;
    }

    *pEnable = (rtk_enable_t)val;

    return RT_ERR_OK;
} /* end of dal_apollo_port_isolationIpmcLeaky_get */

/* Function Name:
 *      dal_apollo_port_isolationIpmcLeaky_set
 * Description:
 *      Set the ip multicast leaky state of the port isolation
 * Input:
 *      port      - port id
 *      enable    - status of port isolation leaky for ip multicast packets
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      none
 */
int32
dal_apollo_port_isolationIpmcLeaky_set(rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  val;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    val = (uint32)enable;
    if ((ret = reg_array_field_write(L2_IPMC_ISO_LEAKYr,port,REG_ARRAY_INDEX_NONE,ENf,&val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PORT|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_apollo_port_isolationIpmcLeaky_set */


static int32
_dal_apollo_port_rmaPortIsolationLeakyEnable_get(rtk_mac_t *pRmaFrame, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  regAddr;
    uint32  val;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);


    /* parameter check */
    RT_PARAM_CHK((NULL == pRmaFrame), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    if( (0x01 == pRmaFrame->octet[0]) &&
        (0x00 == pRmaFrame->octet[1]) &&
        (0x0C == pRmaFrame->octet[2]) &&
        (0xCC == pRmaFrame->octet[3]) &&
        (0xCC == pRmaFrame->octet[4]) &&
        (0xCC == pRmaFrame->octet[5] || 0xCD == pRmaFrame->octet[5]) )
    {
    	switch(pRmaFrame->octet[5])
    	{
    		case 0xCC:
    			regAddr = RMA_CTRL_CDPr;
    			break;
    		case 0xCD:
    			regAddr = RMA_CTRL_SSTPr;
    			break;
    		default:
                return RT_ERR_CHIP_NOT_SUPPORTED;
    	}
    }
    else if((0x01 == pRmaFrame->octet[0]) &&
            (0x80 == pRmaFrame->octet[1]) &&
            (0xC2 == pRmaFrame->octet[2]) &&
            (0x00 == pRmaFrame->octet[3]) &&
            (0x00 == pRmaFrame->octet[4]) &&
            (0x2F >= pRmaFrame->octet[5]) )
    {

    	switch(pRmaFrame->octet[5])
    	{
    		case 0x00:
    			regAddr = RMA_CTRL00r;
    			break;
    		case 0x01:
    			regAddr = RMA_CTRL01r;
    			break;
    		case 0x02:
    			regAddr = RMA_CTRL02r;
    			break;
    		case 0x03:
    			regAddr = RMA_CTRL03r;
    			break;
    		case 0x04:
    		case 0x05:
    		case 0x06:
    		case 0x07:
    		case 0x09:
    		case 0x0A:
    		case 0x0B:
    		case 0x0C:
    		case 0x0F:
    			regAddr = RMA_CTRL04r;
    			break;
    		case 0x08:
    			regAddr = RMA_CTRL08r;
    			break;
    		case 0x0D:
    			regAddr = RMA_CTRL0Dr;
    			break;
    		case 0x0E:
    			regAddr = RMA_CTRL0Er;
    			break;
    		case 0x10:
    			regAddr = RMA_CTRL10r;
    			break;
    		case 0x11:
    			regAddr = RMA_CTRL11r;
    			break;
    		case 0x12:
    			regAddr = RMA_CTRL12r;
    			break;
    		case 0x13:
    		case 0x14:
    		case 0x15:
    		case 0x16:
    		case 0x17:
    		case 0x19:
    		case 0x1B:
    		case 0x1C:
    		case 0x1D:
    		case 0x1E:
    		case 0x1F:
    			regAddr = RMA_CTRL13r;
    			break;
    		case 0x18:
    			regAddr = RMA_CTRL18r;
    			break;
    		case 0x1A:
    			regAddr = RMA_CTRL1Ar;
    			break;
    		case 0x20:
    			regAddr = RMA_CTRL20r;
    			break;
    		case 0x21:
    			regAddr = RMA_CTRL21r;
    			break;
    		case 0x22:
            case 0x23:
            case 0x24:
            case 0x25:
            case 0x26:
            case 0x27:
            case 0x28:
            case 0x29:
            case 0x2a:
            case 0x2b:
            case 0x2c:
            case 0x2d:
            case 0x2e:
            case 0x2f:
    			regAddr = RMA_CTRL22r;
    			break;
    		default:
                return RT_ERR_CHIP_NOT_SUPPORTED;
    	}
    }

    if ((ret = reg_field_read(regAddr, PORTISO_LEAKYf, &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    *pEnable = (rtk_enable_t)!val;

    return RT_ERR_OK;
} /* end of _dal_apollo_trap_rmaPortIsolationEnable_get */


static int32
_dal_apollo_port_rmaPortIsolationLeakyEnable_set(rtk_mac_t *pRmaFrame, rtk_enable_t enable)
{
    int32   ret;
    uint32  regAddr;
    uint32  val;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);


    /* parameter check */
    RT_PARAM_CHK((NULL == pRmaFrame), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    val = (uint32)!enable;

    if( (0x01 == pRmaFrame->octet[0]) &&
        (0x00 == pRmaFrame->octet[1]) &&
        (0x0C == pRmaFrame->octet[2]) &&
        (0xCC == pRmaFrame->octet[3]) &&
        (0xCC == pRmaFrame->octet[4]) &&
        (0xCC == pRmaFrame->octet[5] || 0xCD == pRmaFrame->octet[5]) )
    {
    	switch(pRmaFrame->octet[5])
    	{
    		case 0xCC:
    			regAddr = RMA_CTRL_CDPr;
    			break;
    		case 0xCD:
    			regAddr = RMA_CTRL_SSTPr;
    			break;
    		default:
                return RT_ERR_CHIP_NOT_SUPPORTED;
    	}
    }
    else if((0x01 == pRmaFrame->octet[0]) &&
            (0x80 == pRmaFrame->octet[1]) &&
            (0xC2 == pRmaFrame->octet[2]) &&
            (0x00 == pRmaFrame->octet[3]) &&
            (0x00 == pRmaFrame->octet[4]) &&
            (0x2F >= pRmaFrame->octet[5]) )
    {

    	switch(pRmaFrame->octet[5])
    	{
    		case 0x00:
    			regAddr = RMA_CTRL00r;
    			break;
    		case 0x01:
    			regAddr = RMA_CTRL01r;
    			break;
    		case 0x02:
    			regAddr = RMA_CTRL02r;
    			break;
    		case 0x03:
    			regAddr = RMA_CTRL03r;
    			break;
    		case 0x04:
    		case 0x05:
    		case 0x06:
    		case 0x07:
    		case 0x09:
    		case 0x0A:
    		case 0x0B:
    		case 0x0C:
    		case 0x0F:
    			regAddr = RMA_CTRL04r;
    			break;
    		case 0x08:
    			regAddr = RMA_CTRL08r;
    			break;
    		case 0x0D:
    			regAddr = RMA_CTRL0Dr;
    			break;
    		case 0x0E:
    			regAddr = RMA_CTRL0Er;
    			break;
    		case 0x10:
    			regAddr = RMA_CTRL10r;
    			break;
    		case 0x11:
    			regAddr = RMA_CTRL11r;
    			break;
    		case 0x12:
    			regAddr = RMA_CTRL12r;
    			break;
    		case 0x13:
    		case 0x14:
    		case 0x15:
    		case 0x16:
    		case 0x17:
    		case 0x19:
    		case 0x1B:
    		case 0x1C:
    		case 0x1D:
    		case 0x1E:
    		case 0x1F:
    			regAddr = RMA_CTRL13r;
    			break;
    		case 0x18:
    			regAddr = RMA_CTRL18r;
    			break;
    		case 0x1A:
    			regAddr = RMA_CTRL1Ar;
    			break;
    		case 0x20:
    			regAddr = RMA_CTRL20r;
    			break;
    		case 0x21:
    			regAddr = RMA_CTRL21r;
    			break;
    		case 0x22:
            case 0x23:
            case 0x24:
            case 0x25:
            case 0x26:
            case 0x27:
            case 0x28:
            case 0x29:
            case 0x2a:
            case 0x2b:
            case 0x2c:
            case 0x2d:
            case 0x2e:
            case 0x2f:
    			regAddr = RMA_CTRL22r;
    			break;
    		default:
                return RT_ERR_CHIP_NOT_SUPPORTED;
    	}
    }

    if ((ret = reg_field_write(regAddr, PORTISO_LEAKYf, &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of _dal_apollo_trap_rmaPortIsolationEnable_set */


/* Function Name:
 *      dal_apollo_port_isolationLeaky_get
 * Description:
 *      Get the per port isolation leaky state for given type
 * Input:
 *      type      - Packet type for isolation leaky.
 * Output:
 *      pEnable   - status of port isolation leaky for given leaky type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      none
 */
int32
dal_apollo_port_isolationLeaky_get(rtk_leaky_type_t type, rtk_enable_t *pEnable)
{
    rtk_mac_t rmaFrame;
    int32   ret;
    uint32  data;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "type=%d",type);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK((LEAKY_END <=type), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    rmaFrame.octet[0] = 0x01;
    rmaFrame.octet[1] = 0x80;
    rmaFrame.octet[2] = 0xC2;
    rmaFrame.octet[3] = 0x00;
    rmaFrame.octet[4] = 0x00;

    /* function body */
    switch(type)
    {
        case LEAKY_BRG_GROUP:
        case LEAKY_FD_PAUSE:
        case LEAKY_SP_MCAST:
        case LEAKY_1X_PAE:
        case LEAKY_UNDEF_BRG_04:
        case LEAKY_UNDEF_BRG_05:
        case LEAKY_UNDEF_BRG_06:
        case LEAKY_UNDEF_BRG_07:
        case LEAKY_PROVIDER_BRIDGE_GROUP_ADDRESS:
        case LEAKY_UNDEF_BRG_09:
        case LEAKY_UNDEF_BRG_0A:
        case LEAKY_UNDEF_BRG_0B:
        case LEAKY_UNDEF_BRG_0C:
        case LEAKY_PROVIDER_BRIDGE_GVRP_ADDRESS:
        case LEAKY_8021AB:
        case LEAKY_UNDEF_BRG_0F:
        case LEAKY_BRG_MNGEMENT:
        case LEAKY_UNDEFINED_11:
        case LEAKY_UNDEFINED_12:
        case LEAKY_UNDEFINED_13:
        case LEAKY_UNDEFINED_14:
        case LEAKY_UNDEFINED_15:
        case LEAKY_UNDEFINED_16:
        case LEAKY_UNDEFINED_17:
        case LEAKY_UNDEFINED_18:
        case LEAKY_UNDEFINED_19:
        case LEAKY_UNDEFINED_1A:
        case LEAKY_UNDEFINED_1B:
        case LEAKY_UNDEFINED_1C:
        case LEAKY_UNDEFINED_1D:
        case LEAKY_UNDEFINED_1E:
        case LEAKY_UNDEFINED_1F:
        case LEAKY_GMRP:
        case LEAKY_GVRP:
        case LEAKY_UNDEF_GARP_22:
        case LEAKY_UNDEF_GARP_23:
        case LEAKY_UNDEF_GARP_24:
        case LEAKY_UNDEF_GARP_25:
        case LEAKY_UNDEF_GARP_26:
        case LEAKY_UNDEF_GARP_27:
        case LEAKY_UNDEF_GARP_28:
        case LEAKY_UNDEF_GARP_29:
        case LEAKY_UNDEF_GARP_2A:
        case LEAKY_UNDEF_GARP_2B:
        case LEAKY_UNDEF_GARP_2C:
        case LEAKY_UNDEF_GARP_2D:
        case LEAKY_UNDEF_GARP_2E:
        case LEAKY_UNDEF_GARP_2F:
            rmaFrame.octet[5] = type;
            if (( ret = _dal_apollo_port_rmaPortIsolationLeakyEnable_get(&rmaFrame, pEnable)) != RT_ERR_OK )
            {
                RT_ERR(ret, (MOD_PORT|MOD_DAL), "");
                return ret;
            }

            break;


        case LEAKY_SSTP:
        case LEAKY_CDP:
            rmaFrame.octet[0] = 0x01;
            rmaFrame.octet[1] = 0x00;
            rmaFrame.octet[2] = 0x0C;
            rmaFrame.octet[3] = 0xCC;
            rmaFrame.octet[4] = 0xCC;

            if(LEAKY_CDP == type)
                rmaFrame.octet[5] = 0xCC;
            else
                rmaFrame.octet[5] = 0xCD;

            if (( ret = _dal_apollo_port_rmaPortIsolationLeakyEnable_get(&rmaFrame, pEnable)) != RT_ERR_OK )
            {
                RT_ERR(ret, (MOD_PORT|MOD_DAL), "");
                return ret;
            }
            break;

        case LEAKY_IGMP:
            if ((ret = reg_field_write(IGMP_GLB_CTRLr, PISO_LEAKYf, (uint32 *)&data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }
            *pEnable = data;

            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;

    }


    return RT_ERR_OK;
}   /* end of dal_apollo_port_isolationLeaky_get */

/* Function Name:
 *      dal_apollo_port_isolationLeaky_set
 * Description:
 *      Set the per port isolation leaky state for given type
 * Input:
 *      type      - Packet type for isolation leaky.
 *      enable    - status of port isolation leaky for given leaky type
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      none
 */
int32
dal_apollo_port_isolationLeaky_set(rtk_leaky_type_t type, rtk_enable_t enable)
{
    rtk_mac_t rmaFrame;
    int32   ret;
    uint32 data;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "type=%d",type);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK((LEAKY_END <=type), RT_ERR_INPUT);
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    rmaFrame.octet[0] = 0x01;
    rmaFrame.octet[1] = 0x80;
    rmaFrame.octet[2] = 0xC2;
    rmaFrame.octet[3] = 0x00;
    rmaFrame.octet[4] = 0x00;

    /* function body */
    switch(type)
    {
        case LEAKY_BRG_GROUP:
        case LEAKY_FD_PAUSE:
        case LEAKY_SP_MCAST:
        case LEAKY_1X_PAE:
        case LEAKY_UNDEF_BRG_04:
        case LEAKY_UNDEF_BRG_05:
        case LEAKY_UNDEF_BRG_06:
        case LEAKY_UNDEF_BRG_07:
        case LEAKY_PROVIDER_BRIDGE_GROUP_ADDRESS:
        case LEAKY_UNDEF_BRG_09:
        case LEAKY_UNDEF_BRG_0A:
        case LEAKY_UNDEF_BRG_0B:
        case LEAKY_UNDEF_BRG_0C:
        case LEAKY_PROVIDER_BRIDGE_GVRP_ADDRESS:
        case LEAKY_8021AB:
        case LEAKY_UNDEF_BRG_0F:
        case LEAKY_BRG_MNGEMENT:
        case LEAKY_UNDEFINED_11:
        case LEAKY_UNDEFINED_12:
        case LEAKY_UNDEFINED_13:
        case LEAKY_UNDEFINED_14:
        case LEAKY_UNDEFINED_15:
        case LEAKY_UNDEFINED_16:
        case LEAKY_UNDEFINED_17:
        case LEAKY_UNDEFINED_18:
        case LEAKY_UNDEFINED_19:
        case LEAKY_UNDEFINED_1A:
        case LEAKY_UNDEFINED_1B:
        case LEAKY_UNDEFINED_1C:
        case LEAKY_UNDEFINED_1D:
        case LEAKY_UNDEFINED_1E:
        case LEAKY_UNDEFINED_1F:
        case LEAKY_GMRP:
        case LEAKY_GVRP:
        case LEAKY_UNDEF_GARP_22:
        case LEAKY_UNDEF_GARP_23:
        case LEAKY_UNDEF_GARP_24:
        case LEAKY_UNDEF_GARP_25:
        case LEAKY_UNDEF_GARP_26:
        case LEAKY_UNDEF_GARP_27:
        case LEAKY_UNDEF_GARP_28:
        case LEAKY_UNDEF_GARP_29:
        case LEAKY_UNDEF_GARP_2A:
        case LEAKY_UNDEF_GARP_2B:
        case LEAKY_UNDEF_GARP_2C:
        case LEAKY_UNDEF_GARP_2D:
        case LEAKY_UNDEF_GARP_2E:
        case LEAKY_UNDEF_GARP_2F:
            rmaFrame.octet[5] = type;
            if (( ret = _dal_apollo_port_rmaPortIsolationLeakyEnable_set(&rmaFrame, enable)) != RT_ERR_OK )
            {
                RT_ERR(ret, (MOD_PORT|MOD_DAL), "");
                return ret;
            }
            break;

        case LEAKY_SSTP:
        case LEAKY_CDP:
            rmaFrame.octet[0] = 0x01;
            rmaFrame.octet[1] = 0x00;
            rmaFrame.octet[2] = 0x0C;
            rmaFrame.octet[3] = 0xCC;
            rmaFrame.octet[4] = 0xCC;

            if(LEAKY_CDP == type)
                rmaFrame.octet[5] = 0xCC;
            else
                rmaFrame.octet[5] = 0xCD;

            if (( ret = _dal_apollo_port_rmaPortIsolationLeakyEnable_set(&rmaFrame, enable)) != RT_ERR_OK )
            {
                RT_ERR(ret, (MOD_PORT|MOD_DAL), "");
                return ret;
            }
            break;
        case LEAKY_IGMP:
            data = enable;
            if ((ret = reg_field_write(IGMP_GLB_CTRLr, PISO_LEAKYf, (uint32 *)&data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }

            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;

    }


    return RT_ERR_OK;
}   /* end of dal_apollo_port_isolationLeaky_set */



/* Function Name:
 *      dal_apollo_port_isolationPortLeaky_get
 * Description:
 *      Get the per port isolation leaky state for given type
 * Input:
 *      port      - port id
 *      type      - Packet type for isolation leaky.
 * Output:
 *      pEnable   - status of port isolation leaky for given leaky type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      none
 */
int32
dal_apollo_port_isolationPortLeaky_get(rtk_port_t port, rtk_leaky_type_t type, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  val;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "port=%d,type=%d",port, type);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((LEAKY_IPMULTICAST !=type), RT_ERR_CHIP_NOT_SUPPORTED);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    /* function body */
    if ((ret = reg_array_field_read(L2_IPMC_ISO_LEAKYr, port, REG_ARRAY_INDEX_NONE, ENf, &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PORT|MOD_DAL), "");
        return ret;
    }

    *pEnable = (rtk_enable_t)val;
    return RT_ERR_OK;
}   /* end of dal_apollo_port_isolationPortLeaky_get */


/* Function Name:
 *      dal_apollo_port_isolationPortLeaky_set
 * Description:
 *      Set the per port isolation leaky state for given type
 * Input:
 *      port      - port id
 *      type      - Packet type for isolation leaky.
 *      enable    - status of port isolation leaky for given leaky type
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      none
 */
int32
dal_apollo_port_isolationPortLeaky_set(rtk_port_t port, rtk_leaky_type_t type, rtk_enable_t enable)
{
    int32   ret;
    uint32  val;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "port=%d,type=%d,enable=%d",port, type, enable);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((LEAKY_IPMULTICAST !=type), RT_ERR_CHIP_NOT_SUPPORTED);
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    /* function body */
    val = (uint32)enable;
    if ((ret = reg_array_field_write(L2_IPMC_ISO_LEAKYr, port, REG_ARRAY_INDEX_NONE, ENf, &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PORT|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}   /* end of dal_apollo_port_isolationPortLeaky_set */



/* Function Name:
 *      dal_apollo_port_macRemoteLoopbackEnable_get
 * Description:
 *      Get the mac remote loopback enable status of the specific port
 * Input:
 *      port    - port id
 * Output:
 *      pEnable - pointer to the enable status of mac remote loopback
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      (1) The mac remote loopback enable status of the port is as following:
 *          - DISABLE
 *          - ENABLE
 *      (2) Remote loopback is used to loopback packet RX to switch core back to the outer interface.
 */
int32
dal_apollo_port_macRemoteLoopbackEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
#if 0
    int32   ret;
#endif
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    return RT_ERR_OK;
} /* end of dal_apollo_port_macRemoteLoopbackEnable_get */

/* Function Name:
 *      dal_apollo_port_macRemoteLoopbackEnable_set
 * Description:
 *      Set the mac remote loopback enable status of the specific port
 * Input:
 *      port   - port id
 *      enable - enable status of mac remote loopback
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      (1) The mac remote loopback enable status of the port is as following:
 *          - DISABLE
 *          - ENABLE
 *      (2) Remote loopback is used to loopback packet RX to switch core back to the outer interface.
 */
int32
dal_apollo_port_macRemoteLoopbackEnable_set(rtk_port_t port, rtk_enable_t enable)
{
#if 0
    int32   ret;
#endif
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    return RT_ERR_OK;
} /* end of dal_apollo_port_macRemoteLoopbackEnable_set */

/* Function Name:
 *      dal_apollo_port_macLocalLoopbackEnable_get
 * Description:
 *      Get the mac local loopback enable status of the specific port
 * Input:
 *      port    - port id
 * Output:
 *      pEnable - pointer to the enable status of mac local loopback
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      (1) The mac local loopback enable status of the port is as following:
 *          - DISABLE
 *          - ENABLE
 *      (2) Local loopback is used to loopback packet TX from switch core back to switch core.
 */
int32
dal_apollo_port_macLocalLoopbackEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  data;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    if((ret = reg_array_field_read(P_MISCr, (int32)port, REG_ARRAY_INDEX_NONE, MAC_LOOPBACKf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    *pEnable = (rtk_enable_t)data ;
    return RT_ERR_OK;
} /* end of dal_apollo_port_macLocalLoopbackEnable_get */

/* Function Name:
 *      dal_apollo_port_macLocalLoopbackEnable_set
 * Description:
 *      Set the mac local loopback enable status of the specific port
 * Input:
 *      port   - port id
 *      enable - enable status of mac local loopback
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      (1) The mac local loopback enable status of the port is as following:
 *          - DISABLE
 *          - ENABLE
 *      (2) Local loopback is used to loopback packet TX from switch core back to switch core.
 */
int32
dal_apollo_port_macLocalLoopbackEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  data;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    data = (uint32)enable;
    if((ret = reg_array_field_write(P_MISCr, (int32)port, REG_ARRAY_INDEX_NONE, MAC_LOOPBACKf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_apollo_port_macLocalLoopbackEnable_set */

/* Function Name:
 *      dal_apollo_port_adminEnable_get
 * Description:
 *      Get port admin status of the specific port
 * Input:
 *      port    - port id
 * Output:
 *      pEnable - pointer to the port admin status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_port_adminEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32 data;
    uint32 link;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    if((ret = reg_field_read(ABLTY_FORCE_MODEr, ABLTY_FORCE_MODEf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if(data & (0x01 << port) )
    {
        if((ret = reg_array_field_read(FORCE_P_ABLTYr, (int32)port, REG_ARRAY_INDEX_NONE, LINK_ABLTYf, &link)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        if(PORT_LINKDOWN == link)
            *pEnable = DISABLED;
        else
            *pEnable = ENABLED;
    }
    else
        *pEnable = ENABLED;

    return RT_ERR_OK;
} /* end of dal_apollo_port_adminEnable_get */

/* Function Name:
 *      dal_apollo_port_adminEnable_set
 * Description:
 *      Set port admin status of the specific port
 * Input:
 *      port    - port id
 *      enable  - port admin status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      None
 */
int32
dal_apollo_port_adminEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32 data;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    if((ret = reg_field_read(ABLTY_FORCE_MODEr, ABLTY_FORCE_MODEf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if(DISABLED == enable)
        data |= (0x01 << port);
    else
        data &= ~(0x01 << port);

    if((ret = reg_field_write(ABLTY_FORCE_MODEr, ABLTY_FORCE_MODEf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if(DISABLED == enable)
    {
        data = 0;
        if((ret = reg_array_field_write(FORCE_P_ABLTYr, (int32)port, REG_ARRAY_INDEX_NONE, LINK_ABLTYf, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of dal_apollo_port_adminEnable_set */

/* Function Name:
 *      dal_apollo_port_specialCongest_get
 * Description:
 *      Set the congest seconds of the specific port
 * Input:
 *      port    - port id
 * Output:
 *      pSecond - congest timer (seconds)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_port_specialCongest_get(rtk_port_t port, uint32 *pSecond)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pSecond), RT_ERR_NULL_POINTER);

    if((ret = reg_array_field_read(SC_P_CTRL_0r, (int32)port, REG_ARRAY_INDEX_NONE, CGST_SUST_TMR_LMTf, pSecond)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_apollo_port_specialCongest_get */

/* Function Name:
 *      dal_apollo_port_specialCongest_set
 * Description:
 *      Set the congest seconds of the specific port
 * Input:
 *      port   - port id
 *      second - congest timer (seconds)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      None
 */
int32
dal_apollo_port_specialCongest_set(rtk_port_t port, uint32 second)
{
    int32  ret;
    uint32 data;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((second > HAL_MAX_SPECIAL_CONGEST_SEC()), RT_ERR_INPUT);

    data = second;
    if((ret = reg_array_field_write(SC_P_CTRL_0r, (int32)port, REG_ARRAY_INDEX_NONE, CGST_SUST_TMR_LMTf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_apollo_port_specialCongest_set */

/* Function Name:
 *      dal_apollo_port_specialCongestStatus_get
 * Description:
 *      Get the congest status of the specific port
 * Input:
 *      port   - port id
 * Output:
 *      pStatus - Congest status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      None
 */
int32
dal_apollo_port_specialCongestStatus_get(rtk_port_t port, uint32 *pStatus)
{
    int32  ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pStatus), RT_ERR_NULL_POINTER);

    if((ret = apollo_raw_sc_cgstInd_get(port, pStatus)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_apollo_port_specialCongestStatus_get
 * Description:
 *      Get the congest status of the specific port
 * Input:
 *      port   - port id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      None
 */
int32
dal_apollo_port_specialCongestStatus_clear(rtk_port_t port)
{
    int32  ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);

    if((ret = apollo_raw_sc_cgstInd_reset(port)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_apollo_port_greenEnable_get
 * Description:
 *      Get the statue of green feature of the specific port
 * Input:
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of green feature
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_port_greenEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
#if 0
    int32   ret;
#endif
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    return RT_ERR_OK;
} /* end of dal_apollo_port_greenEnable_get */

/* Function Name:
 *      dal_apollo_port_greenEnable_set
 * Description:
 *      Set the statue of green feature of the specific port
 * Input:
 *      port   - port id
 *      enable - status of green feature
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
dal_apollo_port_greenEnable_set(rtk_port_t port, rtk_enable_t enable)
{
#if 0
    int32   ret;
#endif
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    return RT_ERR_OK;
} /* end of dal_apollo_port_greenEnable_set */


/* Function Name:
 *      dal_apollo_port_phyCrossOverMode_get
 * Description:
 *      Get cross over mode in the specified port.
 * Input:
 *      port  - port id
 * Output:
 *      pMode - pointer to cross over mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Following value is valid
 *      - PORT_CROSSOVER_MODE_AUTO
 *      - PORT_CROSSOVER_MODE_MDI
 *      - PORT_CROSSOVER_MODE_MDIX
 */
int32
dal_apollo_port_phyCrossOverMode_get(rtk_port_t port, rtk_port_crossOver_mode_t *pMode)
{
#if 0
    int32   ret;
#endif
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);

    return RT_ERR_OK;
} /* end of dal_apollo_port_phyCrossOverMode_get */

/* Function Name:
 *      dal_apollo_port_phyCrossOverMode_set
 * Description:
 *      Set cross over mode in the specified port.
 * Input:
 *      port - port id
 *      mode - cross over mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      Following value is valid
 *      - PORT_CROSSOVER_MODE_AUTO
 *      - PORT_CROSSOVER_MODE_MDI
 *      - PORT_CROSSOVER_MODE_MDIX
 */
int32
dal_apollo_port_phyCrossOverMode_set(rtk_port_t port, rtk_port_crossOver_mode_t mode)
{
#if 0
    int32   ret;
#endif
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((PORT_CROSSOVER_MODE_END <= mode), RT_ERR_INPUT);

    return RT_ERR_OK;
} /* end of dal_apollo_port_phyCrossOverMode_set */

/* Function Name:
 *      dal_apollo_port_enhancedFid_get
 * Description:
 *      Get port EFID
 * Input:
 *      port      - port id
 * Output:
 *      pEfid     - EFID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      none
 */
int32
dal_apollo_port_enhancedFid_get(rtk_port_t port, rtk_efid_t *pEfid)
{
    int32 ret;
    uint32 data;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEfid), RT_ERR_NULL_POINTER);

    if((ret = reg_array_field_read(L2_EFIDr, (int32)port, REG_ARRAY_INDEX_NONE, EFIDf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    *pEfid = (rtk_efid_t)data;
    return RT_ERR_OK;
} /* end of dal_apollo_port_enhancedFid_get */

/* Function Name:
 *      dal_apollo_port_enhancedFid_set
 * Description:
 *      Set port EFID
 * Input:
 *      port      - port id
 *      efid      - EFID
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      none
 */
int32
dal_apollo_port_enhancedFid_set(rtk_port_t port, rtk_efid_t efid)
{
    int32 ret;
    uint32 data;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(efid > HAL_ENHANCED_FID_MAX(), RT_ERR_INPUT);

    data = (uint32)efid;
    if((ret = reg_array_field_write(L2_EFIDr, (int32)port, REG_ARRAY_INDEX_NONE, EFIDf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_apollo_port_enhancedFid_set */


/* Function Name:
 *      dal_apollo_port_rtctResult_get
 * Description:
 *      Get test result of RTCT.
 * Input:
 *      port        - the port for retriving RTCT test result
 * Output:
 *      pRtctResult - RTCT result
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_RTCT_NOT_FINISH   - RTCT not finish. Need to wait a while.
 *      RT_ERR_TIMEOUT      - RTCT test timeout in this port.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      If linkType is PORT_SPEED_1000M, test result will be stored in ge_result.
 *      If linkType is PORT_SPEED_10M or PORT_SPEED_100M, test result will be stored in fe_result.
 */
int32
dal_apollo_port_rtctResult_get(rtk_port_t port, rtk_rtctResult_t *pRtctResult)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pRtctResult), RT_ERR_NULL_POINTER);

    return RT_ERR_OK;
} /*end of dal_apollo_port_rtctResult_get*/

/* Function Name:
 *      dal_apollo_port_rtct_start
 * Description:
 *      Start RTCT for ports.
 *      When enable RTCT, the port won't transmit and receive normal traffic.
 * Input:
 *      pPortmask - the ports for RTCT test
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_port_rtct_start(rtk_portmask_t *pPortmask)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);

    return RT_ERR_OK;
} /* end of dal_apollo_port_rtct_start */


/* Function Name:
 *      dal_apollo_port_macForceAbility_set
 * Description:
 *      Set MAC forece ability
 * Input:
 *      port - the ports for set ability
 *      macAbility - mac ability value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_port_macForceAbility_set(rtk_port_t port,rtk_port_macAbility_t macAbility)
{
	int32 ret;
	apollo_raw_port_ability_t ability;
	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);
	/* check Init status */
	RT_INIT_CHK(port_init);
	ability.duplex 	= macAbility.duplex;
	ability.speed	= macAbility.speed;
	ability.linkFib1g= macAbility.linkFib1g;
	ability.linkStatus= macAbility.linkStatus;
	ability.txFc = macAbility.txFc;
	ability.rxFc = macAbility.rxFc;
	ability.nwayAbility= macAbility.nwayAbility;
	ability.masterMod = macAbility.masterMod;
	ability.nwayFault = macAbility.nwayFault;
	ability.lpi_100m = macAbility.lpi_100m;
	ability.lpi_giga = macAbility.lpi_giga;

	if((ret = apollo_raw_port_ForceAbility_set(port,&ability))!=RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
	}
	return RT_ERR_OK;

}


/* Function Name:
 *      dal_apollo_port_macForceAbility_get
 * Description:
 *      Set MAC forece ability
 * Input:
 *      port - the ports for get ability
 *      pMacAbility - mac ability value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_port_macForceAbility_get(rtk_port_t port,rtk_port_macAbility_t *pMacAbility)
{
	int32 ret;
	apollo_raw_port_ability_t ability;
	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);
	/* check Init status */
	RT_INIT_CHK(port_init);

    RT_PARAM_CHK((NULL == pMacAbility), RT_ERR_NULL_POINTER);

	if((ret = apollo_raw_port_ForceAbility_get(port,&ability))!=RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
	}
	pMacAbility->duplex = ability.duplex;
	pMacAbility->speed = ability.speed;
	pMacAbility->linkFib1g = ability.linkFib1g;
	pMacAbility->linkStatus = ability.linkStatus;
	pMacAbility->txFc = ability.txFc;
	pMacAbility->rxFc = ability.rxFc;
	pMacAbility->nwayAbility = ability.nwayAbility;
	pMacAbility->masterMod = ability.masterMod;
	pMacAbility->nwayFault = ability.nwayFault;
	pMacAbility->lpi_100m = ability.lpi_100m;
	pMacAbility->lpi_giga = ability.lpi_giga;


	return RT_ERR_OK;
}


/* Function Name:
 *      dal_apollo_port_macForceAbility_set
 * Description:
 *      Set MAC forece ability state
 * Input:
 *      port - the ports for set ability
 *      state - mac ability state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_port_macForceAbilityState_set(rtk_port_t port,rtk_enable_t state)
{
	int32 ret;
	uint32 data=0;

	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);
	/* check Init status */
	RT_INIT_CHK(port_init);

	if((ret = reg_field_read(ABLTY_FORCE_MODEr, ABLTY_FORCE_MODEf, &data)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
		return ret;
	}

	if(state == ENABLED){
		data |= (0x1 << port);
	}else
	{
		data &= ~(0x1 << port);
	}
	if((ret = reg_field_write(ABLTY_FORCE_MODEr, ABLTY_FORCE_MODEf, &data)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
		return ret;
	}
	return RT_ERR_OK;

}


/* Function Name:
 *      dal_apollo_port_macForceAbilityState_get
 * Description:
 *      Get MAC forece ability state
 * Input:
 *      port - the ports for get ability
 *      pState - mac ability state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_port_macForceAbilityState_get(rtk_port_t port,rtk_enable_t *pState)
{
	int32 ret;
	uint32 data=0;
	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT),"%s",__FUNCTION__);
	/* check Init status */
	RT_INIT_CHK(port_init);

    RT_PARAM_CHK((NULL == pState), RT_ERR_NULL_POINTER);

	if((ret = reg_field_read(ABLTY_FORCE_MODEr, ABLTY_FORCE_MODEf, &data)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
		return ret;
	}

	*pState = data & (0x01 << port) ? ENABLED : DISABLED ;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_apollo_port_macExtMode_set
 * Description:
 *      Set extension MAC mode
 * Input:
 *      port - the ports number
 *      ext_mode - the mode setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 * Note:
 *      None
 */
int32
dal_apollo_port_macExtMode_set(rtk_port_t port, rtk_port_ext_mode_t ext_mode)
{
    int32 ret;
	uint32 data=0;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "port=%d,ext_mode=%d",port, ext_mode);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_RGMII_PORT(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((EXT_MODE_END <= ext_mode), RT_ERR_INPUT);
    RT_PARAM_CHK((EXT_MODE_RESERVED == ext_mode), RT_ERR_INPUT);

    /* function body */
    data = (uint32)ext_mode;
    if((ret = reg_field_write(MODE_EXTr, MODE_EXTf, &data)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
		return ret;
	}

    return RT_ERR_OK;
}   /* end of dal_apollo_port_macExtMode_set */

/* Function Name:
 *      dal_apollo_port_macExtMode_get
 * Description:
 *      Get extension MAC mode
 * Input:
 *      port - the ports number
 * Output:
 *      pExt_mode - the mode setting
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_port_macExtMode_get(rtk_port_t port, rtk_port_ext_mode_t *pExt_mode)
{
    int32 ret;
	uint32 data=0;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "port=%d",port);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_RGMII_PORT(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pExt_mode), RT_ERR_NULL_POINTER);

    /* function body */
    if((ret = reg_field_read(MODE_EXTr, MODE_EXTf, &data)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
		return ret;
	}

    *pExt_mode = (rtk_port_ext_mode_t)data;
    return RT_ERR_OK;
}   /* end of dal_apollo_port_macExtMode_get */

/* Function Name:
 *      dal_apollo_port_macExtRgmiiDelay_set
 * Description:
 *      Set RGMII TX/RX delay
 * Input:
 *      port - the ports number
 *      txDelay - the TX delay (0 ~ 1)
 *      rxDelay - the RX delay (0 ~ 7)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 * Note:
 *      None
 */
int32
dal_apollo_port_macExtRgmiiDelay_set(rtk_port_t port, uint32 txDelay, uint32 rxDelay)
{
    int32 ret;
	uint32 data=0;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "port=%d,txDelay=%d,rxDelay=%d",port, txDelay, rxDelay);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_RGMII_PORT(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((HAL_MAX_RGMII_TX_DELAY() < txDelay), RT_ERR_INPUT);
    RT_PARAM_CHK((HAL_MAX_RGMII_RX_DELAY() < rxDelay), RT_ERR_INPUT);

    /* function body */
    data = ((txDelay << 3) | rxDelay);
    if((ret = reg_field_write(EXT_RGMXFr, EXT_RGMXFf, &data)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
		return ret;
	}

    return RT_ERR_OK;
}   /* end of dal_apollo_port_macExtRgmiiDelay_set */

/* Function Name:
 *      dal_apollo_port_macExtRgmiiDelay_get
 * Description:
 *      Get RGMII TX/RX delay
 * Input:
 *      port - the ports number
 * Output:
 *      pTxDelay - the TX delay (0 ~ 1)
 *      pRxDelay - the RX delay (0 ~ 7)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_port_macExtRgmiiDelay_get(rtk_port_t port, uint32 *pTxDelay, uint32 *pRxDelay)
{
    int32 ret;
	uint32 data=0;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "port=%d",port);

    /* check Init status */
    RT_INIT_CHK(port_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_RGMII_PORT(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pTxDelay), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pRxDelay), RT_ERR_NULL_POINTER);

    /* function body */
    if((ret = reg_field_read(EXT_RGMXFr, EXT_RGMXFf, &data)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
		return ret;
	}

    *pTxDelay = ((data >> 3) & 0x1);
    *pRxDelay = (data & 0x7);
    return RT_ERR_OK;
}   /* end of dal_apollo_port_macExtRgmiiDelay_get */



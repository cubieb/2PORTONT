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
 * $Revision: 46644 $
 * $Date: 2014-02-26 18:16:35 +0800 (?Ÿæ?ä¸? 26 äºŒæ? 2014) $
 *
 * Purpose : Definition of Switch Global API
 *
 * Feature : The file have include the following module and sub-modules
 *           (1) Switch parameter settings
 *           (2) Management address and vlan configuration.
 *
 */


/*
 * Include Files
 */
#include <rtk/switch.h>

#include <dal/rtl9602bvb/dal_rtl9602bvb.h>
#include <dal/rtl9602bvb/dal_rtl9602bvb_switch.h>
#include <dal/rtl9602bvb/dal_rtl9602bvb_flowctrl.h>
#include <hal/common/miim.h>
#include <osal/time.h>
#include <hal/mac/mac_probe.h>

/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */
static uint32    switch_init = INIT_NOT_COMPLETED;


static uint32  chipId;
static uint32  chipRev;
static uint32  chipSubtype;

/*
 * Function Declaration
 */

int32 dal_rtl9602bvb_switch_phyPatch(void);

int32 _rtl9602bvb_switch_macAddr_set(rtk_mac_t *pMacAddr);
int32 _rtl9602bvb_switch_macAddr_get(rtk_mac_t *pMacAddr);

static int32
_dal_rtl9602bvb_switch_ponAsicPhyPortId_get(rtk_switch_port_name_t portName, int32 *pPortId)
{
    switch(portName)
    {
        case RTK_PORT_UTP0:
            *pPortId = 0;
            break;
        case RTK_PORT_UTP1:
            *pPortId = 1;
            break;
        case RTK_PORT_PON:
        case RTK_PORT_UTP2:
            *pPortId = 2;
            break;
        case RTK_PORT_CPU:
            *pPortId = 3;
            break;

        default:
            return RT_ERR_INPUT;
    }
    return RT_ERR_OK;
}




static int32
_dal_rtl9602bvb_switch_ponAsicLogicalPortName_get(int32 portId, rtk_switch_port_name_t *pPortName)
{
    switch(portId)
    {
        case 0:
            *pPortName = RTK_PORT_UTP0;
            break;
        case 1:
            *pPortName = RTK_PORT_UTP1;
            break;
        case 2:
            *pPortName = RTK_PORT_PON | RTK_PORT_UTP2;
            break;
        case 3:
            *pPortName = RTK_PORT_CPU;
            break;

        default:
            return RT_ERR_INPUT;
    }
    return RT_ERR_OK;
}


static int32
_dal_rtl9602bvb_switch_tickToken_init(void)
{
    int ret;
    uint32 wData;

    /*meter pon-tick-token configuration*/
    wData = 0x6e;
    if ((ret = reg_field_write(RTL9602BVB_PON_TB_CTRLr, RTL9602BVB_TICK_PERIODf, &wData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }
    wData = 0x95;
    if ((ret = reg_field_write(RTL9602BVB_PON_TB_CTRLr, RTL9602BVB_TKNf, &wData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }
    /*meter switch-tick-token configuration*/
    wData = 43;
    if ((ret = reg_field_write(RTL9602BVB_METER_TB_CTRLr, RTL9602BVB_TICK_PERIODf, &wData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }
    wData = 189;
    if ((ret = reg_field_write(RTL9602BVB_METER_TB_CTRLr, RTL9602BVB_TKNf, &wData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}



static int32
_dal_rtl9602bvb_switch_green_enable(void)
{
    return RT_ERR_OK;
}

static int32
_dal_rtl9602bvb_switch_eee_enable(void)
{
    return RT_ERR_OK;
}

static int32
_dal_rtl9602bvb_switch_powerSaving_init(void)
{
    int ret;

    /* === Green enable === */
    if ((ret = _dal_rtl9602bvb_switch_green_enable()) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }

    /* === EEE enable === */
    if ((ret = _dal_rtl9602bvb_switch_eee_enable()) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}



int32
dal_rtl9602bvb_switch_phyPatch(void)
{
    return RT_ERR_OK;
}

static int32 _dal_rtl9602bvb_get_chip_version(void)
{
    int32   ret;    
	uint32 value;
    if (drv_swcore_cid_get(&chipId, &chipRev) != RT_ERR_OK)
    {
 	    RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }
#if 1
    value = 0xb;
    if ((ret = reg_field_write(RTL9602BVB_BOND_INFOr,RTL9602BVB_BOND_INFO_ENf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }

    if ((ret = reg_field_read(RTL9602BVB_BOND_INFOr,RTL9602BVB_BOND_CHIP_MODEf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }

    chipSubtype = value & 0x1f;

    value = 0x0;
    if ((ret = reg_field_write(RTL9602BVB_BOND_INFOr,RTL9602BVB_BOND_INFO_ENf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }
#endif

#if 0
    if ((ret = dal_apollomp_tool_get_chipSubType(&chipSubtype)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }
#endif



    return RT_ERR_OK;
}


/* Module Name    : Switch     */
/* Sub-module Name: Switch parameter settings */

/* Function Name:
 *      dal_rtl9602bvb_switch_init
 * Description:
 *      Initialize switch module of the specified device.
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
dal_rtl9602bvb_switch_init(void)
{
    int32   ret;
    uint32 wData;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH),"%s",__FUNCTION__);

    switch_init = INIT_COMPLETED;

    /*get chip id*/
    if((ret = _dal_rtl9602bvb_get_chip_version())!=RT_ERR_OK)
    {
 	    RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }

    /*flow control threshold and set buffer mode*/
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
    /*set packet buffer size*/
#else
    /*set packet buffer size*/
#endif


    /*flow control threshold and set buffer mode*/
    if((ret = rtl9602bvb_raw_flowctrl_patch(FLOWCTRL_PATCH_DEFAULT)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }

    /*meter tick-token configuration*/
    if((ret = _dal_rtl9602bvb_switch_tickToken_init())!=RT_ERR_OK)
    {
 	    RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }

#if defined(FPGA_DEFINED)

#else
	#if 0 // TBD: Remove register
    /*CFG_PHY_POLL_CMD set to 0x1100*/
    wData = 0x1100;
    if ((ret = reg_write(RTL9602BVB_CFG_PHY_POLL_CMDr,&wData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }
	#endif
#if defined(RTL_CYGWIN_EMULATE)

#else
    /*analog patch*/
    /*phy patch*/
    if((ret=dal_rtl9602bvb_switch_phyPatch())!=RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "phy patch fail\n");
        return ret;
    }

    /* power saving feature */
    if((ret=_dal_rtl9602bvb_switch_powerSaving_init())!=RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }
#endif
    /*set switch ready, phy patch done*/
    wData = 1;
    if ((ret = reg_field_write(RTL9602BVB_WRAP_GPHY_MISCr,RTL9602BVB_PATCH_PHY_DONEf,&wData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }
	#if 0 // TBD: Remove register
    /*CFG_PHY_POLL_CMD set to 0x1110*/
    wData = 0x1110;
    if ((ret = reg_write(RTL9602BVB_CFG_PHY_POLL_CMDr,&wData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }
	#endif
#endif
	/* TBD remove field
    wData = 0;
    if ((ret = reg_field_write(RTL9602BVB_L34_GLB_CFGr, RTL9602BVB_CFG_CPU_ORG_OPf, (uint32 *)&wData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }
	*/
    wData = 1;
    if ((ret = reg_field_write(RTL9602BVB_CFG_UNHIOLr, RTL9602BVB_IPG_COMPENSATIONf, (uint32 *)&wData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_switch_init */

/* Module Name    : Switch     */
/* Sub-module Name: Switch parameter settings */


/* Function Name:
 *      dal_rtl9602bvb_switch_phyPortId_get
 * Description:
 *      Get physical port id from logical port name
 * Input:
 *      portName - logical port name
 * Output:
 *      pPortId  - pointer to the physical port id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Call RTK API the port ID must get from this API
 */
int32
dal_rtl9602bvb_switch_phyPortId_get(rtk_switch_port_name_t portName, int32 *pPortId)
{
    dal_rtl9602bvb_switch_asic_type_t asicType;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH),"%s",__FUNCTION__);

    /* parameter check */
    RT_PARAM_CHK((RTK_PORT_NAME_END <= portName), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pPortId), RT_ERR_NULL_POINTER);

    /*get chip info to check port name mapping*/
    asicType = RTL9602BVB_ASIC_PON;

    switch(asicType)
    {
        case RTL9602BVB_ASIC_PON:
        default:
            return _dal_rtl9602bvb_switch_ponAsicPhyPortId_get(portName,pPortId);
            break;
    }

    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_switch_phyPortId_get */



/* Function Name:
 *      dal_rtl9602bvb_switch_logicalPort_get
 * Description:
 *      Get logical port name from physical port id
 * Input:
 *      portId  - physical port id
 * Output:
 *      pPortName - pointer to logical port name
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 */
int32
dal_rtl9602bvb_switch_logicalPort_get(int32 portId, rtk_switch_port_name_t *pPortName)
{
    dal_rtl9602bvb_switch_asic_type_t asicType;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH),"%s",__FUNCTION__);

    /* parameter check */
    RT_PARAM_CHK((NULL == pPortName), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(portId), RT_ERR_PORT_ID);

    /*get chip info to check port name mapping*/
    asicType = RTL9602BVB_ASIC_PON;

    switch(asicType)
    {
        case RTL9602BVB_ASIC_PON:
        default:
            return _dal_rtl9602bvb_switch_ponAsicLogicalPortName_get(portId,pPortName);
            break;
    }

    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_switch_logicalPort_get */



/* Function Name:
 *      dal_rtl9602bvb_switch_port2PortMask_set
 * Description:
 *      Set port id to the portlist
 * Input:
 *      pPortMask    - port mask
 *      portName     - logical port name
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Call RTK API the port mask must set by this API
 */
int32
dal_rtl9602bvb_switch_port2PortMask_set(rtk_portmask_t *pPortMask, rtk_switch_port_name_t portName)
{
    int32 portId;
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH),"%s",__FUNCTION__);

    /* parameter check */
    RT_PARAM_CHK((NULL == pPortMask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((RTK_PORT_NAME_END <= portName), RT_ERR_INPUT);

    if((ret=dal_rtl9602bvb_switch_phyPortId_get(portName,&portId))!=RT_ERR_OK)
        return ret;

    RTK_PORTMASK_PORT_SET(*pPortMask,portId);

    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_switch_port2PortMask_set */



/* Function Name:
 *      dal_rtl9602bvb_switch_port2PortMask_clear
 * Description:
 *      Set port id to the portlist
 * Input:
 *      pPortMask    - port mask
 *      portName     - logical port name
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Call RTK API the port mask must set by this API
 */
int32
dal_rtl9602bvb_switch_port2PortMask_clear(rtk_portmask_t *pPortMask, rtk_switch_port_name_t portName)
{
    int32   ret;
    int32 portId;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH),"%s",__FUNCTION__);

    /* parameter check */
    RT_PARAM_CHK((NULL == pPortMask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((RTK_PORT_NAME_END <= portName), RT_ERR_INPUT);

    if((ret=dal_rtl9602bvb_switch_phyPortId_get(portName,&portId))!=RT_ERR_OK)
        return ret;

    RTK_PORTMASK_PORT_CLEAR(*pPortMask, portId);

    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_switch_port2PortMask_clear */



/* Function Name:
 *      dal_rtl9602bvb_switch_portIdInMask_check
 * Description:
 *      Check if given port is in port list
 * Input:
 *      pPortMask    - port mask
 *      portName     - logical port name
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 */
int32
dal_rtl9602bvb_switch_portIdInMask_check(rtk_portmask_t *pPortMask, rtk_switch_port_name_t portName)
{
    int32   ret;
    int32 portId;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH),"%s",__FUNCTION__);

    /* parameter check */
    RT_PARAM_CHK((NULL == pPortMask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((RTK_PORT_NAME_END <= portName), RT_ERR_INPUT);

    if((ret=dal_rtl9602bvb_switch_phyPortId_get(portName,&portId))!=RT_ERR_OK)
        return ret;

    if(RTK_PORTMASK_IS_PORT_SET(*pPortMask,portId))
        return RT_ERR_OK;
    else
        return RT_ERR_FAILED;

} /* end of dal_rtl9602bvb_switch_portIdInMask_check */


/* Function Name:
  *      dal_rtl9602bvb_switch_maxPktLenLinkSpeed_get
  * Description:
  *      Get the max packet length setting of the specific speed type
  * Input:
  *      speed - speed type
  * Output:
  *      pLen  - pointer to the max packet length
  * Return:
  *      RT_ERR_OK
  *      RT_ERR_FAILED
  *      RT_ERR_NULL_POINTER - input parameter may be null pointer
  *      RT_ERR_INPUT        - invalid enum speed type
  * Note:
  *      Max packet length setting speed type
  *      - MAXPKTLEN_LINK_SPEED_FE
  *      - MAXPKTLEN_LINK_SPEED_GE
  */
int32
dal_rtl9602bvb_switch_maxPktLenLinkSpeed_get(rtk_switch_maxPktLen_linkSpeed_t speed, uint32 *pLen)
{
    int32   ret;
	uint32 regAddr;
    uint32 fieldIdx;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(switch_init);

    /* parameter check */
    RT_PARAM_CHK((MAXPKTLEN_LINK_SPEED_END <= speed), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pLen), RT_ERR_NULL_POINTER);

    if(speed == MAXPKTLEN_LINK_SPEED_FE)
    {
	    regAddr =  RTL9602BVB_MAX_LENGTH_CFG0r;
        fieldIdx = RTL9602BVB_ACCEPT_MAX_LENTH_CFG0f;
    }
    else
    {
	    regAddr =  RTL9602BVB_MAX_LENGTH_CFG1r;
        fieldIdx = RTL9602BVB_ACCEPT_MAX_LENTH_CFG1f;
    }

    if ((ret = reg_field_read(regAddr, fieldIdx, pLen)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }


    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_switch_maxPktLenLinkSpeed_get */

/* Function Name:
  *      dal_rtl9602bvb_switch_maxPktLenLinkSpeed_set
  * Description:
  *      Set the max packet length of the specific speed type
  * Input:
  *      speed - speed type
  *      len   - max packet length
  * Output:
  *      None
  * Return:
  *      RT_ERR_OK
  *      RT_ERR_FAILED
  *      RT_ERR_INPUT   - invalid enum speed type
  * Note:
  *      Max packet length setting speed type
  *      - MAXPKTLEN_LINK_SPEED_FE
  *      - MAXPKTLEN_LINK_SPEED_GE
  */
int32
dal_rtl9602bvb_switch_maxPktLenLinkSpeed_set(rtk_switch_maxPktLen_linkSpeed_t speed, uint32 len)
{
    rtk_port_t  port, max_port;
    int32   ret;
	uint32 regAddr;
    uint32 fieldIdx;
    uint32 portField;
    uint32 index;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(switch_init);

    /* parameter check */
    RT_PARAM_CHK((MAXPKTLEN_LINK_SPEED_END <= speed), RT_ERR_INPUT);
    RT_PARAM_CHK((RTL9602BVB_PACEKT_LENGTH_MAX < len), RT_ERR_INPUT);

    if(speed == MAXPKTLEN_LINK_SPEED_FE)
    {
	    regAddr =  RTL9602BVB_MAX_LENGTH_CFG0r;
        fieldIdx = RTL9602BVB_ACCEPT_MAX_LENTH_CFG0f;
        portField = RTL9602BVB_MAX_LENGTH_10_100f;
        index = 0;
    }
    else
    {
	    regAddr   = RTL9602BVB_MAX_LENGTH_CFG1r;
        fieldIdx  = RTL9602BVB_ACCEPT_MAX_LENTH_CFG1f;
        portField = RTL9602BVB_MAX_LENGTH_GIGAf;
        index = 1;
    }

    if ((ret = reg_field_write(regAddr, fieldIdx, &len)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }

    /*set all port index to currect length index*/
    max_port = HAL_GET_MAX_PORT();
    for (port = 0; port <= max_port; port++)
    {
        if (!HAL_IS_PORT_EXIST(port))
        {
            continue;
        }

        if ((ret = reg_array_field_write(RTL9602BVB_ACCEPT_MAX_LEN_CTRLr, port, REG_ARRAY_INDEX_NONE, portField, &index)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
            return ret;
        }
    }
    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_switch_maxPktLenLinkSpeed_set */


/* Module Name    : Switch     */
/* Sub-module Name: Management address and vlan configuration */
/* Function Name:
 *      _rtl9602bvb_switch_macAddr_set
 * Description:
 *      Set switch mac address configurations
 * Input:
 *      pMacAddr - Switch mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_INPUT  				- Invalid input parameter
 * Note:
 *      None
 */
int32 _rtl9602bvb_switch_macAddr_set(rtk_mac_t *pMacAddr)
{
    int32 ret;
    uint32 i, tmp[6];
	
    if(pMacAddr == NULL)
        return RT_ERR_NULL_POINTER;

    for (i=0;i<ETHER_ADDR_LEN;i++)
        tmp[i] = pMacAddr->octet[i];


    if ((ret = reg_field_write(RTL9602BVB_SWITCH_MACr, RTL9602BVB_SWITCH_MAC5f, &tmp[0])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_field_write(RTL9602BVB_SWITCH_MACr, RTL9602BVB_SWITCH_MAC4f, &tmp[1])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_field_write(RTL9602BVB_SWITCH_MACr, RTL9602BVB_SWITCH_MAC3f, &tmp[2])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_field_write(RTL9602BVB_SWITCH_MACr, RTL9602BVB_SWITCH_MAC2f, &tmp[3])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }	
     if ((ret = reg_field_write(RTL9602BVB_SWITCH_MACr, RTL9602BVB_SWITCH_MAC1f, &tmp[4])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }
     if ((ret = reg_field_write(RTL9602BVB_SWITCH_MACr, RTL9602BVB_SWITCH_MAC0f, &tmp[5])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }
	 
    return RT_ERR_OK;
}/*end of _rtl9602bvb_switch_macAddr_set*/

/* Function Name:
 *      _rtl9602bvb_switch_macAddr_get
 * Description:
 *      Get switch mac address configurations
 * Input:
 *      pMacAddr - Switch mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_INPUT  				- Invalid input parameter
 * Note:
 *      None
 */
int32 _rtl9602bvb_switch_macAddr_get(rtk_mac_t *pMacAddr)
{
    int32 ret;
    uint32 i, tmp[6];

    if ((ret = reg_field_read(RTL9602BVB_SWITCH_MACr, RTL9602BVB_SWITCH_MAC5f, &tmp[0])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_field_read(RTL9602BVB_SWITCH_MACr, RTL9602BVB_SWITCH_MAC4f, &tmp[1])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_field_read(RTL9602BVB_SWITCH_MACr, RTL9602BVB_SWITCH_MAC3f, &tmp[2])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_field_read(RTL9602BVB_SWITCH_MACr, RTL9602BVB_SWITCH_MAC2f, &tmp[3])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }	
     if ((ret = reg_field_read(RTL9602BVB_SWITCH_MACr, RTL9602BVB_SWITCH_MAC1f, &tmp[4])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }
     if ((ret = reg_field_read(RTL9602BVB_SWITCH_MACr, RTL9602BVB_SWITCH_MAC0f, &tmp[5])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }

    for (i=0;i<ETHER_ADDR_LEN;i++)
        pMacAddr->octet[i] = tmp[i];
	 
    return RT_ERR_OK;
}/*end of _rtl9602bvb_switch_macAddr_get*/



/* Function Name:
 *      dal_rtl9602bvb_switch_mgmtMacAddr_get
 * Description:
 *      Get MAC address of switch.
 * Input:
 *      None
 * Output:
 *      pMac - pointer to MAC address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_rtl9602bvb_switch_mgmtMacAddr_get(rtk_mac_t *pMac)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(switch_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pMac), RT_ERR_NULL_POINTER);

    if ((ret = _rtl9602bvb_switch_macAddr_get(pMac)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_switch_mgmtMacAddr_get */

/* Function Name:
 *      dal_rtl9602bvb_switch_mgmtMacAddr_set
 * Description:
 *      Set MAC address of switch.
 * Input:
 *      pMac - MAC address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_rtl9602bvb_switch_mgmtMacAddr_set(rtk_mac_t *pMac)
{

    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(switch_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pMac), RT_ERR_NULL_POINTER);

    if((pMac->octet[0] & BITMASK_1B) == 1)
        return RT_ERR_INPUT;

    if ((ret = _rtl9602bvb_switch_macAddr_set(pMac)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_switch_mgmtMacAddr_set */


/* Function Name:
 *      dal_rtl9602bvb_switch_chip_reset
 * Description:
 *      Reset switch chip
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_rtl9602bvb_switch_chip_reset(void){

    int32   ret;
    uint32 resetValue = 1;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(switch_init);

    if ((ret = reg_field_write(RTL9602BVB_SOFTWARE_RSTr,RTL9602BVB_CMD_CHIP_RST_PSf,&resetValue)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;

}


/* Function Name:
 *      dal_rtl9602bvb_switch_version_get
 * Description:
 *      Get chip version
 * Input:
 *      pChipId    - chip id
 *      pRev       - revision id
 *      pSubtype   - sub type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 */
int32
dal_rtl9602bvb_switch_version_get(uint32 *pChipId, uint32 *pRev, uint32 *pSubtype)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(switch_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pChipId), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pRev), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pSubtype), RT_ERR_NULL_POINTER);

    /* function body */
    *pChipId = chipId;
    *pRev = chipRev;
    *pSubtype = chipSubtype;

    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_switch_version_get */

/* Function Name:
  *      dal_rtl9602bvb_switch_maxPktLenByPort_get
  * Description:
  *      Get the max packet length setting of specific port
  * Input:
  *      port - speed type
  * Output:
  *      pLen - pointer to the max packet length
  * Return:
  *      RT_ERR_OK
  *      RT_ERR_FAILED
  *      RT_ERR_NULL_POINTER - input parameter may be null pointer
  *      RT_ERR_INPUT        - invalid enum speed type
  * Note:
  */
int32
dal_rtl9602bvb_switch_maxPktLenByPort_get(rtk_port_t port, uint32 *pLen)
{
	int32   ret;
	uint32  reg;
	uint32  field;
	
	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH),"%s",__FUNCTION__);

	/* check Init status */
    RT_INIT_CHK(switch_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
	RT_PARAM_CHK((NULL == pLen), RT_ERR_NULL_POINTER);

	/* function body */
	
	/*If PON port, use MAX_LENGTH_CFG1. Other ports, use MAX_LENGTH_CFG0*/
	if(HAL_IS_PON_PORT(port))
	{
		reg = RTL9602BVB_MAX_LENGTH_CFG1r;
		field = RTL9602BVB_ACCEPT_MAX_LENTH_CFG1f;
	}else{
		reg = RTL9602BVB_MAX_LENGTH_CFG0r;
		field = RTL9602BVB_ACCEPT_MAX_LENTH_CFG0f;
	}

	if((ret = reg_field_read(reg, field, pLen)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }

	return RT_ERR_OK;
} /*end of dal_rtl9602bvb_switch_maxPktLenByPort_get*/

/* Function Name:
  *      dal_rtl9602bvb_switch_maxPktLenByPort_set
  * Description:
  *      Set the max packet length of specific port
  * Input:
  *      port  - port
  *      len   - max packet length
  * Output:
  *      None
  * Return:
  *      RT_ERR_OK
  *      RT_ERR_FAILED
  *      RT_ERR_INPUT   - invalid enum speed type
  * Note:
  */
int32
dal_rtl9602bvb_switch_maxPktLenByPort_set(rtk_port_t port, uint32 len)
{
	int32   ret;
	uint32  cfgVal;
	uint32  reg;
	uint32  field;
	
	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH),"%s",__FUNCTION__);

	/* check Init status */
    RT_INIT_CHK(switch_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
	RT_PARAM_CHK((RTL9602BVB_PACEKT_LENGTH_MAX < len), RT_ERR_INPUT);

	/* function body */

	/*If PON port, use MAX_LENGTH_CFG1. Other ports, use MAX_LENGTH_CFG0*/
	if(HAL_IS_PON_PORT(port))
	{
		reg = RTL9602BVB_MAX_LENGTH_CFG1r;
		field = RTL9602BVB_ACCEPT_MAX_LENTH_CFG1f;
		cfgVal = 1;
	}else{
		reg = RTL9602BVB_MAX_LENGTH_CFG0r;
		field = RTL9602BVB_ACCEPT_MAX_LENTH_CFG0f;
		cfgVal = 0;
	}

	if((ret = reg_field_write(reg, field, &len)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }
	
	if((ret = reg_array_field_write(RTL9602BVB_ACCEPT_MAX_LEN_CTRLr, port, REG_ARRAY_INDEX_NONE, RTL9602BVB_MAX_LENGTH_GIGAf, &cfgVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }
	if((ret = reg_array_field_write(RTL9602BVB_ACCEPT_MAX_LEN_CTRLr, port, REG_ARRAY_INDEX_NONE, RTL9602BVB_MAX_LENGTH_10_100f, &cfgVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }
	
	return RT_ERR_OK;
}/*end of dal_rtl9602bvb_switch_maxPktLenByPort_set*/

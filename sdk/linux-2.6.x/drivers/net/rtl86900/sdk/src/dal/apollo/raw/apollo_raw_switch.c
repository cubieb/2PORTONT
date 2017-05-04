/*
 * Copyright (C) 2011 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : switch asic-level switch core API 
 * Feature : Switch Core related functions
 *
 */

#include <dal/apollo/raw/apollo_raw_switch.h>

const uint32 apollo_parser_field[16] = {CFG_PAR_FIELD_CTRL_00f, CFG_PAR_FIELD_CTRL_01f, CFG_PAR_FIELD_CTRL_02f, CFG_PAR_FIELD_CTRL_03f,
                    CFG_PAR_FIELD_CTRL_04f, CFG_PAR_FIELD_CTRL_05f, CFG_PAR_FIELD_CTRL_06f, CFG_PAR_FIELD_CTRL_07f,
                    CFG_PAR_FIELD_CTRL_08f, CFG_PAR_FIELD_CTRL_09f, CFG_PAR_FIELD_CTRL_10f, CFG_PAR_FIELD_CTRL_11f,
                    CFG_PAR_FIELD_CTRL_12f, CFG_PAR_FIELD_CTRL_13f, CFG_PAR_FIELD_CTRL_14f, CFG_PAR_FIELD_CTRL_15f};

const uint32 apollo_parser_register[16] = {PARSER_FIELD_SELTOR_CTRL_0_1r, PARSER_FIELD_SELTOR_CTRL_0_1r, PARSER_FIELD_SELTOR_CTRL_2_3r, PARSER_FIELD_SELTOR_CTRL_2_3r,    
                       PARSER_FIELD_SELTOR_CTRL_4_5r, PARSER_FIELD_SELTOR_CTRL_4_5r, PARSER_FIELD_SELTOR_CTRL_6_7r, PARSER_FIELD_SELTOR_CTRL_6_7r,    
                       PARSER_FIELD_SELTOR_CTRL_8_9r, PARSER_FIELD_SELTOR_CTRL_8_9r, PARSER_FIELD_SELTOR_CTRL_10_11r, PARSER_FIELD_SELTOR_CTRL_10_11r,    
                       PARSER_FIELD_SELTOR_CTRL_12_13r, PARSER_FIELD_SELTOR_CTRL_12_13r, PARSER_FIELD_SELTOR_CTRL_14_15r, PARSER_FIELD_SELTOR_CTRL_14_15r};


/* Function Name:
 *      apollo_raw_switch_macAddr_set
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
int32 apollo_raw_switch_macAddr_set(rtk_mac_t *pMacAddr)
{
    int32 ret;
    uint32 i, tmp[6];
	
    if(pMacAddr == NULL)
        return RT_ERR_NULL_POINTER;

    for (i=0;i<ETHER_ADDR_LEN;i++)
        tmp[i] = pMacAddr->octet[i];


    if ((ret = reg_field_write(SWITCH_MACr, SWITCH_MAC5f, &tmp[0])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_field_write(SWITCH_MACr, SWITCH_MAC4f, &tmp[1])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_field_write(SWITCH_MACr, SWITCH_MAC3f, &tmp[2])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_field_write(SWITCH_MACr, SWITCH_MAC2f, &tmp[3])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }	
     if ((ret = reg_field_write(SWITCH_MACr, SWITCH_MAC1f, &tmp[4])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }
     if ((ret = reg_field_write(SWITCH_MACr, SWITCH_MAC0f, &tmp[5])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }
	 
    return RT_ERR_OK;
}/*end of apollo_raw_switch_macAddr_set*/

/* Function Name:
 *      apollo_raw_switch_macAddr_get
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
int32 apollo_raw_switch_macAddr_get(rtk_mac_t *pMacAddr)
{
    int32 ret;
    uint32 i, tmp[6];

    if ((ret = reg_field_read(SWITCH_MACr, SWITCH_MAC5f, &tmp[0])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_field_read(SWITCH_MACr, SWITCH_MAC4f, &tmp[1])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_field_read(SWITCH_MACr, SWITCH_MAC3f, &tmp[2])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_field_read(SWITCH_MACr, SWITCH_MAC2f, &tmp[3])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }	
     if ((ret = reg_field_read(SWITCH_MACr, SWITCH_MAC1f, &tmp[4])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }
     if ((ret = reg_field_read(SWITCH_MACr, SWITCH_MAC0f, &tmp[5])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }

    for (i=0;i<ETHER_ADDR_LEN;i++)
        pMacAddr->octet[i] = tmp[i];
	 
    return RT_ERR_OK;
}/*end of apollo_raw_switch_macAddr_get*/



/* Function Name:
 *      apollo_raw_switch_maxPktLen_set
 * Description:
 *      Set accepted maximum length of reveiving packet configuration
 * Input:
 *      index - configuration index
 *      len   - max packet length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE  
 * Note:
 *      None
 */
int32 apollo_raw_switch_maxPktLen_set(uint32 index, uint32 len)
{
    int32 ret;
	uint32 regAddr;
    uint32 fieldIdx;
    
    
	RT_PARAM_CHK((APOLLO_MAXLENGTH_INDEX_NUM <= index), RT_ERR_OUT_OF_RANGE);
	RT_PARAM_CHK((APOLLO_PACEKT_LENGTH_MAX < len), RT_ERR_OUT_OF_RANGE);

	if( 1 == index)
	{
	    regAddr = MAX_LENGTH_CFG1r;
        fieldIdx = ACCEPT_MAX_LENTH_CFG1f;
	}    
	else if( 0 == index)
	{
	    regAddr = MAX_LENGTH_CFG0r;
        fieldIdx = ACCEPT_MAX_LENTH_CFG0f;
	}    
	
    if ((ret = reg_field_write(regAddr, fieldIdx, &len)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }

	return RT_ERR_OK;

} /* end of apollo_raw_switch_maxPktLen_set */

/* Function Name:
 *      apollo_raw_switch_maxPktLen_get
 * Description:
 *      Get accepted maximum length of reveiving packet configuration
 * Input:
 *      index - configuration index
 * Output:
 *      pLen   - max packet length
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE   
 *      RT_ERR_NULL_POINTER   
 * Note:
 *      None
 */
int32 apollo_raw_switch_maxPktLen_get(uint32 index, uint32 *pLen)
{
    int32 ret;
	uint32 regAddr;
    uint32 fieldIdx;

    /* parameter check */
    RT_PARAM_CHK((pLen==NULL), RT_ERR_NULL_POINTER);
	RT_PARAM_CHK((APOLLO_MAXLENGTH_INDEX_NUM <= index), RT_ERR_OUT_OF_RANGE);

	if( 1 == index)
	{
	    regAddr = MAX_LENGTH_CFG1r;
        fieldIdx = ACCEPT_MAX_LENTH_CFG1f;
	}    
	else if( 0 == index)
	{
	    regAddr = MAX_LENGTH_CFG0r;
        fieldIdx = ACCEPT_MAX_LENTH_CFG0f;
	}    
	
    if ((ret = reg_field_read(regAddr, fieldIdx, pLen)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }

	return RT_ERR_OK;

} /* end of apollo_raw_switch_maxPktLen_get */

/* Function Name:
 *      apollo_raw_switch_maxPktLenSpeed_set
 * Description:
 *      Set accepted maximum length in different speed
 * Input:
 *      port - port number
 *      speed - Speed of link up port
 *      index - configuration index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT  
 *      RT_ERR_PORT_ID  
 *      RT_ERR_OUT_OF_RANGE  
 * Note:
 *      None
 */
int32 apollo_raw_switch_maxPktLenSpeed_set(rtk_port_t port, apollo_raw_linkSpeed_t speed, uint32 index)
{
    int32 ret;
	uint32 regField;

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
	RT_PARAM_CHK((LINKSPEED_GIGA < speed), RT_ERR_INPUT);
	RT_PARAM_CHK((APOLLO_MAXLENGTH_INDEX_NUM <= index), RT_ERR_OUT_OF_RANGE);


	if( LINKSPEED_10M == speed || LINKSPEED_100M == speed)
		regField = MAX_LENGTH_10_100f;
	else if( LINKSPEED_GIGA == speed)
		regField = MAX_LENGTH_GIGAf;
	
	
    if ((ret = reg_array_field_write(ACCEPT_MAX_LEN_CTRLr, port, REG_ARRAY_INDEX_NONE, regField, &index)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }

	return RT_ERR_OK;

} /* end of apollo_raw_switch_maxPktLen_set */

/* Function Name:
 *      apollo_raw_switch_maxPktLenSpeed_get
 * Description:
 *      Get accepted maximum length in different speed
 * Input:
 *      port - port number
 *      speed - Speed of link up port
 * Output:
 *      pIndex - configuration index
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT  
 *      RT_ERR_PORT_ID  
 *      RT_ERR_NULL_POINTER  
 * Note:
 *      None
 */
int32 apollo_raw_switch_maxPktLenSpeed_get(rtk_port_t port, apollo_raw_linkSpeed_t speed, uint32 *pIndex)
{
    int32 ret;
	uint32 regField;

    RT_PARAM_CHK((pIndex==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);


	if( speed == LINKSPEED_10M || speed == LINKSPEED_100M)
		regField = MAX_LENGTH_10_100f;
	else if(speed == LINKSPEED_GIGA)
		regField = MAX_LENGTH_GIGAf;
	
	
    if ((ret = reg_array_field_read(ACCEPT_MAX_LEN_CTRLr, port, REG_ARRAY_INDEX_NONE, regField, pIndex)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }

	return RT_ERR_OK;

} /* end of apollo_raw_switch_maxPktLen_get */


/* Function Name:
 *      apollo_raw_switch_cpuTagFmt_set
 * Description:
 *      Set CPU-tag format
 * Input:
 *      format - CPU tag format
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT  
 * Note:
 *      None
 */
int32 apollo_raw_switch_cpuTagMode_set(apollo_raw_cpuTagMode_t format)
{
    int32 ret;

	RT_PARAM_CHK((RAW_CPUTAGMODE_END <= format), RT_ERR_INPUT);

    if ((ret = reg_field_write(MAC_CPU_TAG_CTRLr, TAG_FORMATf, &format)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }

	return RT_ERR_OK;

} /* end of apollo_raw_switch_cpuTagFmt_set */

/* Function Name:
 *      apollo_raw_switch_cpuTagFmt_get
 * Description:
 *      Get CPU-tag format
 * Input:
 *      None
 * Output:
 *      pFormat - CPU tag format
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_switch_cpuTagMode_get(apollo_raw_cpuTagMode_t *pFormat)
{
    int32 ret;

    RT_PARAM_CHK((pFormat==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(MAC_CPU_TAG_CTRLr, TAG_FORMATf, pFormat)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }

	return RT_ERR_OK;

} /* end of apollo_raw_switch_cpuTagFmt_get */



/* Function Name:
 *      apollo_raw_switch_cpuTagAware_set
 * Description:
 *      Set CPU tag ingress aware configuration
 * Input:
 *      port 	- port number
 *      enable 	- enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT  
 *      RT_ERR_PORT_ID  
 * Note:
 *      None
 */
int32 apollo_raw_switch_cpuTagAware_set(rtk_port_t port, rtk_enable_t enable)
{
    int32 ret;

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
	RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    if ((ret = reg_array_field_write(MAC_CPU_TAG_AWARE_CTRLr, port, REG_ARRAY_INDEX_NONE, ENf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }

	return RT_ERR_OK;

} /* end of apollo_raw_switch_cpuTagAware_set */

/* Function Name:
 *      apollo_raw_switch_cpuTagAware_get
 * Description:
 *      Get CPU tag ingress aware configuration
 * Input:
 *      port - port number
 * Output:
 *      pEenable - enable function
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT  
 *      RT_ERR_PORT_ID  
 *      RT_ERR_NULL_POINTER  
 * Note:
 *      None
 */
int32 apollo_raw_switch_cpuTagAware_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    int32 ret;

    RT_PARAM_CHK((pEnable==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);

    if ((ret = reg_array_field_read(MAC_CPU_TAG_AWARE_CTRLr, port, REG_ARRAY_INDEX_NONE, ENf, pEnable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH | MOD_DAL), "");
        return ret;
    }

	return RT_ERR_OK;

} /* end of apollo_raw_switch_cpuTagAware_set */


/* Function Name:
 *      apollo_raw_switch_48pass1Enable_set
 * Description:
 *      Set 48 pass 1 status  
 * Input:
 *      enable 		- enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_switch_48pass1Enable_set(rtk_enable_t enable)
{
    int32 ret;

	RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    if ((ret = reg_field_write(CFG_BACKPRESSUREr, EN_48_PASS_1f, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_switch_48pass1Enable_set */

/* Function Name:
 *      apollo_raw_switch_48pass1Enable_get
 * Description:
 *      Get 48 pass 1 status  
 * Input:
 *      None
 * Output:
 *      pEnable 		- enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_switch_48pass1Enable_get(rtk_enable_t *pEnable)
{
    int32 ret;

    RT_PARAM_CHK((pEnable==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(CFG_BACKPRESSUREr, EN_48_PASS_1f, pEnable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_switch_48pass1Enable_get */

/* Function Name:
 *      apollo_raw_switch_shortIpgEnable_set
 * Description:
 *      Set short IPG status  
 * Input:
 *      enable 		- enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_switch_shortIpgEnable_set(rtk_enable_t enable)
{
    int32 ret;

	RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    if ((ret = reg_field_write(SWITCH_CTRLr, SHORT_IPGf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_switch_shortIpgEnable_set */

/* Function Name:
 *      apollo_raw_switch_shortIpgEnable_get
 * Description:
 *      Get short IPG status  
 * Input:
 *      None
 * Output:
 *      pEnable 		- enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_switch_shortIpgEnable_get(rtk_enable_t *pEnable)
{
    int32 ret;

    RT_PARAM_CHK((pEnable==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(SWITCH_CTRLr, SHORT_IPGf, pEnable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_switch_shortIpgEnable_get */

/* Function Name:
 *      apollo_raw_switch_ipgCompensation_set
 * Description:
 *      Set IPG compensation 
 * Input:
 *      mode 		- compensation
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_switch_ipgCompensation_set(apollo_raw_ipgCompMode_t mode)
{
    int32 ret;

	RT_PARAM_CHK((RAW_IPGCOMPMODE_END <= mode), RT_ERR_INPUT);

    if ((ret = reg_field_write(CFG_UNHIOLr, IPG_COMPENSATIONf, &mode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_switch_ipgCompensation_set */

/* Function Name:
 *      apollo_raw_switch_ipgCompensation_get
 * Description:
 *      Get short IPG status  
 * Input:
 *      None
 * Output:
 *      pMode 		- compensation
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_switch_ipgCompensation_get(apollo_raw_ipgCompMode_t *pMode)
{
    int32 ret;

    RT_PARAM_CHK((pMode==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(CFG_UNHIOLr, IPG_COMPENSATIONf, pMode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_switch_ipgCompensation_get */

/* Function Name:
 *      apollo_raw_switch_parserFieldSelector_set
 * Description:
 *      Set parser field selector configuration 
 * Input:
 *      index 		- field selctor index
 *      mode 		- selector mode 
 *      offset 		- data parsering latch offset
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_CHIP_NOT_SUPPORTED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 apollo_raw_switch_parserFieldSelector_set(uint32 index, apollo_raw_parserFieldSelctorMode_t mode, uint32 offset)
{
    int32 ret;
    uint32 data;
    uint32 reg;
    uint32 field;
    
	RT_PARAM_CHK((RAW_FIELDSELMODE_END <= mode), RT_ERR_CHIP_NOT_SUPPORTED);
	RT_PARAM_CHK((APOLLO_FIELD_SELECTOR_MAX < index), RT_ERR_OUT_OF_RANGE);
	RT_PARAM_CHK((APOLLO_FIELD_SELECTOR_OFFSET_MAX < offset), RT_ERR_OUT_OF_RANGE);

    reg = apollo_parser_register[index];
    field = apollo_parser_field[index];
    data = mode;
    data = (mode << 8) | (offset & 0xFF);
    
    if ((ret = reg_field_write(reg, field, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_switch_parserFieldSelector_set */

/* Function Name:
 *      apollo_raw_switch_parserFieldSelector_get
 * Description:
 *      Get parser field selector configuration 
 * Input:
 *      index 		- field selctor index
 * Output:
 *      pMode 		- selector mode 
 *      pOffset 		- data parsering latch offset
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 apollo_raw_switch_parserFieldSelector_get(uint32 index, apollo_raw_parserFieldSelctorMode_t *pMode, uint32 *pOffset)
{

    int32 ret;
    uint32 reg;
    uint32 field;
    uint32 data;

    reg = apollo_parser_register[index];
    field = apollo_parser_field[index];
    
    if ((ret = reg_field_read(reg, field, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }

    *pOffset = data & 0xFF;
    *pMode = data >> 8;
    
    return RT_ERR_OK;
} /* end of apollo_raw_switch_parserFieldSelector_get */


/* Function Name:
 *      apollo_raw_switch_rxCrcCheck_set
 * Description:
 *      Set RX CRC check
 * Input:
 *      port                   - Physical port number (0~6)
 *      enable                - enable state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_switch_rxCrcCheck_set(uint32 port, rtk_enable_t enable)
{
    int32   ret;
    uint32 regdata;
	
    if(port >= APOLLO_PORTNO)
        return RT_ERR_PORT_ID;   

    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    regdata = (enable == ENABLED) ? 0 : 1;
	
    /*parameter check*/
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);	
    
    if ((ret = reg_array_field_write(P_MISCr, port, REG_ARRAY_INDEX_NONE, CRC_SKIPf, &regdata)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_switch_rxCrcCheck_set */

/* Function Name:
 *      apollo_raw_switch_rxCrcCheck_get
 * Description:
 *      Get RX CRC check status
 * Input:
 *      port                   - Physical port number (0~6)
 * Output:
 *      pEnable              - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_switch_rxCrcCheck_get(uint32 port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32 regdata;
	
    if(port >= APOLLO_PORTNO)
        return RT_ERR_PORT_ID;    
    
    /*parameter check*/
    RT_PARAM_CHK((pEnable == NULL), RT_ERR_NULL_POINTER);
    
    if ((ret = reg_array_field_read(P_MISCr, port, REG_ARRAY_INDEX_NONE, CRC_SKIPf, &regdata)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }

    *pEnable = (regdata == 0) ? ENABLED : DISABLED;

    return RT_ERR_OK;
} /* end of apollo_raw_switch_rxCrcCheck_get */



/* Function Name:
 *      apollo_raw_switch_smallTagIpg_set
 * Description:
 *      Set small IPG for tag insertion
 * Input:
 *      port                   - Physical port number (0~6)
 *      enable                - enable state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_switch_smallTagIpg_set(uint32 port, rtk_enable_t enable)
{
    int32   ret;

    if(port >= APOLLO_PORTNO)
        return RT_ERR_PORT_ID;   
	
    /*parameter check*/
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);	
    
    if ((ret = reg_array_field_write(P_MISCr, port, REG_ARRAY_INDEX_NONE, SMALL_TAG_IPGf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_switch_smallTagIpg_set */

/* Function Name:
 *      apollo_raw_switch_smallTagIpg_get
 * Description:
 *      Get small IPG for tag insertion
 * Input:
 *      port                   - Physical port number (0~6)
 * Output:
 *      pEnable              - state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_switch_smallTagIpg_get(uint32 port, uint32 *pEnable)
{
    int32   ret;
    
    if(port >= APOLLO_PORTNO)
        return RT_ERR_PORT_ID;    
    
    /*parameter check*/
    RT_PARAM_CHK((pEnable == NULL), RT_ERR_NULL_POINTER);
    
    if ((ret = reg_array_field_read(P_MISCr, port, REG_ARRAY_INDEX_NONE, SMALL_TAG_IPGf, pEnable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_switch_smallTagIpg_get */

/* Function Name:
 *      apollo_raw_switch_smallPkt_set
 * Description:
 *      Enable received special packet which packet length is smaller than 64 bytes, 
 *      or received packet is not byte alignment and L2 CRC errored packet
 * Input:
 *      port                   - Physical port number (0~6)
 *      enable                - enable state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_switch_smallPkt_set(uint32 port, rtk_enable_t enable)
{
    int32   ret;

    if(port >= APOLLO_PORTNO)
        return RT_ERR_PORT_ID;   
	
    /*parameter check*/
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);	
    
    if ((ret = reg_array_field_write(P_MISCr, port, REG_ARRAY_INDEX_NONE, RX_SPCf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_switch_smallPkt_set */

/* Function Name:
 *      apollo_raw_switch_smallPkt_get
 * Description:
 *      Enable received special packet which packet length is smaller than 64 bytes, 
 *      or received packet is not byte alignment and L2 CRC errored packet
 * Input:
 *      port                   - Physical port number (0~6)
 * Output:
 *      pEnable              - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_switch_smallPkt_get(uint32 port, uint32 *pEnable)
{
    int32   ret;
    
    if(port >= APOLLO_PORTNO)
        return RT_ERR_PORT_ID;    
    
    /*parameter check*/
    RT_PARAM_CHK((pEnable == NULL), RT_ERR_NULL_POINTER);
    
    if ((ret = reg_array_field_read(P_MISCr, port, REG_ARRAY_INDEX_NONE, RX_SPCf, pEnable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_switch_smallPkt_get */


/* Function Name:
 *      apollo_raw_switch_chipReset_set
 * Description:
 *      Reset module.
 * Input:
 *      reset         - reset items
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_CHIP_NOT_SUPPORTED
 * Note:
 *      None
 */
int32 apollo_raw_switch_chipReset_set(apollo_raw_chipReset_t reset)
{
    int32 ret;
   uint32 enable = ENABLED;

    switch(reset)
    {
        case RAW_SW_GLOBAL_RST:
            if ((ret = reg_field_write(CHIP_RSTr, SW_RSTf, &enable)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
                return ret;
            }
            break;
        case RAW_SW_CHIP_RST:
            if ((ret = reg_field_write(SOFTWARE_RSTr, CMD_CHIP_RST_PSf, &enable)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
                return ret;
            }
            break;        
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }
    return RT_ERR_OK;
} /* end of apollo_raw_switch_chipReset_set */


/* Function Name:
 *      apollo_raw_switch_backPressureEnable_set
 * Description:
 *      Set back-pressure status  
 * Input:
 *      enable 		- enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_switch_backPressureEnable_set(rtk_enable_t enable)
{
    int32 ret;

	RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    if ((ret = reg_field_write(CFG_BACKPRESSUREr, EN_BACKPRESSUREf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_switch_backPressureEnable_set */

/* Function Name:
 *      apollo_raw_switch_backPressureEnable_get
 * Description:
 *      Get backupressure status  
 * Input:
 *      None
 * Output:
 *      pEnable 		- enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_switch_backPressureEnable_get(rtk_enable_t *pEnable)
{
    int32 ret;

    RT_PARAM_CHK((pEnable==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(CFG_BACKPRESSUREr, EN_BACKPRESSUREf, pEnable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_switch_backPressureEnable_get */

/* Function Name:
 *      apollo_raw_switch_backPressure_set
 * Description:
 *      Set back-pressure status  
 * Input:
 *      state 		- status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_switch_backPressure_set(apollo_raw_backPressure_t state)
{
    int32 ret;

    RT_PARAM_CHK((RAW_BACKPRESSURE_END <= state), RT_ERR_INPUT);

    if ((ret = reg_field_write(CFG_BACKPRESSUREr, LONGTXEf, &state)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_switch_backPressureEnable_set */

/* Function Name:
 *      apollo_raw_switch_backPressureEnable_get
 * Description:
 *      Get backupressure status  
 * Input:
 *      None
 * Output:
 *      pState 		- status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_switch_backPressure_get(apollo_raw_backPressure_t *pState)
{
    int32 ret;

    RT_PARAM_CHK((pState==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(CFG_BACKPRESSUREr, LONGTXEf, pState)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_switch_backPressureEnable_get */


/* Function Name:
 *      apollo_raw_switch_limitPause_set
 * Description:
 *      Set limit to pause frame
 * Input:
 *      enable                - enable state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_switch_limitPause_set(rtk_enable_t enable)
{
    int32   ret;
    uint32 regdata; 

    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    regdata = (enable == ENABLED) ? 0 : 1;
	
    /*parameter check*/
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);	
    
    if ((ret = reg_field_write(SWITCH_CTRLr, PAUSE_MAX128f, &regdata)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_switch_limitPause_set */

/* Function Name:
 *      apollo_raw_switch_limitPause_get
 * Description:
 *      Get limit to pause frame
 * Input:
 *      port                   - Physical port number (0~6)
 * Output:
 *      pEnable              - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_switch_limitPause_get(rtk_enable_t *pEnable)
{
    int32   ret;
    uint32 regdata; 
    
    /*parameter check*/
    RT_PARAM_CHK((pEnable == NULL), RT_ERR_NULL_POINTER);
    
    if ((ret = reg_field_read(SWITCH_CTRLr, PAUSE_MAX128f, &regdata)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }

    *pEnable = (regdata == 0) ? ENABLED : DISABLED;

    return RT_ERR_OK;
} /* end of apollo_raw_switch_limitPause_get */

/* Function Name:
 *      apollo_raw_switch_globalRxCrcCheck_set
 * Description:
 *      Set global RCR check
 * Input:
 *      enable                - enable state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_switch_globalRxCrcCheck_set(rtk_enable_t enable)
{
    int32   ret;
    uint32 regdata; 

    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    regdata = (enable == ENABLED) ? 0 : 1;
	
    /*parameter check*/
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);	
    
    if ((ret = reg_field_write(CFG_BACKPRESSUREr, EN_BYPASS_ERRORf, &regdata)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_switch_globalRxCrcCheck_set */

/* Function Name:
 *      apollo_raw_switch_globalRxCrcCheck_get
 * Description:
 *      Get global RCR check
 * Input:
 *      port                   - Physical port number (0~6)
 * Output:
 *      pEnable              - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_switch_globalRxCrcCheck_get(rtk_enable_t *pEnable)
{
    int32   ret;
    uint32 regdata; 
    
    /*parameter check*/
    RT_PARAM_CHK((pEnable == NULL), RT_ERR_NULL_POINTER);
    
    if ((ret = reg_field_read(CFG_BACKPRESSUREr, EN_BYPASS_ERRORf, &regdata)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SWITCH|MOD_DAL), "");
        return ret;
    }

    *pEnable = (regdata == 0) ? ENABLED : DISABLED;

    return RT_ERR_OK;
} /* end of apollo_raw_switch_globalRxCrcCheck_get */



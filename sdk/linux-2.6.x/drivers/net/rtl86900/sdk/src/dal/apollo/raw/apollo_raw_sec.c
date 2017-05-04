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
 * Purpose : switch asic-level security API 
 * Feature : The file have include the following module and sub-modules
 *           1) Attack prevention
 *
 */

#include <dal/apollo/raw/apollo_raw_sec.h>


/* Function Name:
 *      apollo_raw_sec_portAttackPreventEnable_set
 * Description:
 *      Set the attack prevention status of the specific port
 * Input:
 *      port   		- port id
 *      enable 		- enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 * Note:
 *      None
 */
int32 apollo_raw_sec_portAttackPreventEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    int32 ret;

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
	RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);
    
    if ((ret = reg_array_field_write(DOS_ENr, port, REG_ARRAY_INDEX_NONE, ENf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SEC), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_sec_portAttackPreventEnable_set */

/* Function Name:
 *      apollo_raw_sec_portAttackPreventEnable_get
 * Description:
 *      Get the attack prevention status of the specific port
 * Input:
 *      port   		- port id
 * Output:
 *      pEnable 	- enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       
 *      RT_ERR_NULL_POINTER      
 * Note:
 *      None
 */
int32 apollo_raw_sec_portAttackPreventEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    int32 ret;

    /* parameter check */
    RT_PARAM_CHK((pEnable==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);

    
    if ((ret = reg_array_field_read(DOS_ENr, port, REG_ARRAY_INDEX_NONE, ENf, pEnable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SEC), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_sec_portAttackPreventEnable_get */

/* Function Name:
 *      apollo_raw_sec_dslvcAttackPreventEnable_set
 * Description:
 *      Set the attack prevention status of the specific dsl vc
 * Input:
 *      port   		- port id
 *      enable 		- enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_DSL_VC      
 * Note:
 *      None
 */
int32 apollo_raw_sec_dslvcAttackPreventEnable_set(rtk_port_t vc, rtk_enable_t enable)
{
    int32 ret;

    /* parameter check */
    RT_PARAM_CHK((APOLLO_VCPORTNOMAX < vc), RT_ERR_PORT_ID);
	RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);
    
    if ((ret = reg_array_field_write(DOS_DSL_ENr, REG_ARRAY_INDEX_NONE, vc, ENf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SEC), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_sec_dslvcAttackPreventEnable_get */

/* Function Name:
 *      apollo_raw_sec_dslvcAttackPreventEnable_get
 * Description:
 *      Get the attack prevention status of the specific dsl vc
 * Input:
 *      port   		- port id
 * Output:
 *      pEnable 	- enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER       
 *      RT_ERR_DSL_VC       
 * Note:
 *      None
 */
int32 apollo_raw_sec_dslvcAttackPreventEnable_get(rtk_port_t vc, rtk_enable_t *pEnable)
{
    int32 ret;

    /* parameter check */
    RT_PARAM_CHK((pEnable==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((APOLLO_VCPORTNOMAX < vc), RT_ERR_PORT_ID);

    
    if ((ret = reg_array_field_read(DOS_DSL_ENr, REG_ARRAY_INDEX_NONE, vc, ENf, pEnable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SEC), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_sec_dslvcAttackPreventEnable_get */



/* Function Name:
 *      apollo_raw_sec_attackPreventEnable_set
 * Description:
 *      Set action for each kind of attack.
 * Input:
 *      attackType 		- type of attack
 *      enable 			- enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_CHIP_NOT_SUPPORTED           
 * Note:
 *      None
 */
int32 apollo_raw_sec_attackPreventEnable_set(rtk_sec_attackType_t attackType, rtk_enable_t enable)
{
    int32 ret;
    uint32 field;
    
    RT_PARAM_CHK((ATTACK_TYPE_END <= attackType), RT_ERR_INPUT);
	RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

	switch(attackType)
	{
		case DAEQSA_DENY:
			field = DOS_DAEQSAf;
			break;
		case LAND_DENY:
			field = DOS_LANDATTACKSf;
			break;
		case BLAT_DENY:
			field = DOS_BLATATTACKSf;
			break;
		case SYNFIN_DENY:
			field = DOS_SYNFINSCANf;
			break;
		case XMA_DENY:
			field = DOS_XMASCANf;
			break;
		case NULLSCAN_DENY:
			field = DOS_NULLSCANf;
			break;
		case SYN_SPORTL1024_DENY:
			field = DOS_SYN1024f;
			break;
		case TCPHDR_MIN_CHECK:
			field = DOS_TCPSHORTHDRf;
			break;
		case TCP_FRAG_OFF_MIN_CHECK:
			field = DOS_TCPFRAGERRORf;
			break;
		case ICMP_FRAG_PKTS_DENY:
			field = DOS_ICMPFRAGMENTf;
			break;
		case POD_DENY:
			field = DOS_PINGOFDEATHf;
			break;
		case UDPDOMB_DENY:
			field = DOS_UDPBOMBf;
			break;
		case SYNWITHDATA_DENY:
			field = DOS_SYNWITHDATAf;
			break;
		case SYNFLOOD_DENY:
			field = DOS_SYNFLOODf;
			break;
		case FINFLOOD_DENY:
			field = DOS_FINFLOODf;
			break;
		case ICMPFLOOD_DENY:
			field = DOS_ICMPFLOODf;
			break;
		default:
            return RT_ERR_CHIP_NOT_SUPPORTED;					
	}

    if ((ret = reg_field_write(DOS_CFGr,field, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SEC | MOD_DAL), "");
        return ret;
    }
   
    return RT_ERR_OK;
} /* end of apollo_raw_sec_attackPreventEnable_set */

/* Function Name:
 *      apollo_raw_sec_attackPreventAction_get
 * Description:
 *      Get action for each kind of attack.
 * Input:
 *      attackType 		- type of attack
 * Output:
 *      pEnable 		- enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_CHIP_NOT_SUPPORTED           
 *      RT_ERR_NULL_POINTER           
 * Note:
 *      None
 */
int32 apollo_raw_sec_attackPreventEnable_get(rtk_sec_attackType_t attackType, rtk_enable_t *pEnable)
{
    int32 ret;
    uint32 field;
    
    RT_PARAM_CHK((ATTACK_TYPE_END <= attackType), RT_ERR_INPUT);
    RT_PARAM_CHK((pEnable==NULL), RT_ERR_NULL_POINTER);

	switch(attackType)
	{
		case DAEQSA_DENY:
			field = DOS_DAEQSAf;
			break;
		case LAND_DENY:
			field = DOS_LANDATTACKSf;
			break;
		case BLAT_DENY:
			field = DOS_BLATATTACKSf;
			break;
		case SYNFIN_DENY:
			field = DOS_SYNFINSCANf;
			break;
		case XMA_DENY:
			field = DOS_XMASCANf;
			break;
		case NULLSCAN_DENY:
			field = DOS_NULLSCANf;
			break;
		case SYN_SPORTL1024_DENY:
			field = DOS_SYN1024f;
			break;
		case TCPHDR_MIN_CHECK:
			field = DOS_TCPSHORTHDRf;
			break;
		case TCP_FRAG_OFF_MIN_CHECK:
			field = DOS_TCPFRAGERRORf;
			break;
		case ICMP_FRAG_PKTS_DENY:
			field = DOS_ICMPFRAGMENTf;
			break;
		case POD_DENY:
			field = DOS_PINGOFDEATHf;
			break;
		case UDPDOMB_DENY:
			field = DOS_UDPBOMBf;
			break;
		case SYNWITHDATA_DENY:
			field = DOS_SYNWITHDATAf;
			break;
		case SYNFLOOD_DENY:
			field = DOS_SYNFLOODf;
			break;
		case FINFLOOD_DENY:
			field = DOS_FINFLOODf;
			break;
		case ICMPFLOOD_DENY:
			field = DOS_ICMPFLOODf;
			break;
		default:
            return RT_ERR_CHIP_NOT_SUPPORTED;					
	}

    if ((ret = reg_field_read(DOS_CFGr,field, pEnable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SEC | MOD_DAL), "");
        return ret;
    }
   
    return RT_ERR_OK;
} /* end of apollo_raw_sec_attackPreventEnable_get */


/* Function Name:
 *      apollo_raw_sec_attackPreventAction_set
 * Description:
 *      Set action for each kind of attack.
 * Input:
 *      attackType 		- type of attack
 *      action        	- DOS action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_CHIP_NOT_SUPPORTED           
 * Note:
 *      None
 */
int32 apollo_raw_sec_attackPreventAction_set(rtk_sec_attackType_t attackType, rtk_action_t action)
{
    int32 ret;
    uint32 field;
	apollo_raw_dosAction_t dos_act;

    RT_PARAM_CHK((ATTACK_TYPE_END <= attackType), RT_ERR_INPUT);
    RT_PARAM_CHK((ACTION_END <= action), RT_ERR_INPUT);

	switch(attackType)
	{
		case DAEQSA_DENY:
			field = DOS_DAEQSA_ACTf;
			break;
		case LAND_DENY:
			field = DOS_LANDATTACKS_ACTf;
			break;
		case BLAT_DENY:
			field = DOS_BLATATTACKS_ACTf;
			break;
		case SYNFIN_DENY:
			field = DOS_SYNFINSCAN_ACTf;
			break;
		case XMA_DENY:
			field = DOS_XMASCAN_ACTf;
			break;
		case NULLSCAN_DENY:
			field = DOS_NULLSCAN_ACTf;
			break;
		case SYN_SPORTL1024_DENY:
			field = DOS_SYN1024_ACTf;
			break;
		case TCPHDR_MIN_CHECK:
			field = DOS_TCPSHORTHDR_ACTf;
			break;
		case TCP_FRAG_OFF_MIN_CHECK:
			field = DOS_TCPFRAGERROR_ACTf;
			break;
		case ICMP_FRAG_PKTS_DENY:
			field = DOS_ICMPFRAGMENT_ACTf;
			break;
		case POD_DENY:
			field = DOS_PINGOFDEATH_ACTf;
			break;
		case UDPDOMB_DENY:
			field = DOS_UDPBOMB_ACTf;
			break;
		case SYNWITHDATA_DENY:
			field = DOS_SYNWITHDATA_ACTf;
			break;
		case SYNFLOOD_DENY:
			field = DOS_SYNFLOOD_ACTf;
			break;
		case FINFLOOD_DENY:
			field = DOS_FINFLOOD_ACTf;
			break;
		case ICMPFLOOD_DENY:
			field = DOS_ICMPFLOOD_ACTf;
			break;
		default:
            return RT_ERR_CHIP_NOT_SUPPORTED;					
	}

	switch(action)
	{
	 	case ACTION_TRAP2CPU:
	 		dos_act = RAW_DOS_ACT_TRAP;
			break;
	 	case ACTION_DROP:
	 		dos_act = RAW_DOS_ACT_DROP;
			break;
		default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
	}

    if ((ret = reg_field_write(DOS_CFGr,field, &dos_act)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SEC | MOD_DAL), "");
        return ret;
    }
   
    return RT_ERR_OK;
} /* end of apollo_raw_sec_attackPreventAction_set */

/* Function Name:
 *      apollo_raw_sec_attackPreventAction_get
 * Description:
 *      Get action for each kind of attack.
 * Input:
 *      attackType 		- type of attack
 * Output:
 *      pAction        	- DOS action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_CHIP_NOT_SUPPORTED           
 *      RT_ERR_NULL_POINTER           
 * Note:
 *      None
 */
int32 apollo_raw_sec_attackPreventAction_get(rtk_sec_attackType_t attackType, rtk_action_t* pAction)
{
    int32 ret;
    uint32 field;
	apollo_raw_dosAction_t dos_act;

    RT_PARAM_CHK((ATTACK_TYPE_END <= attackType), RT_ERR_INPUT);
    RT_PARAM_CHK((pAction==NULL), RT_ERR_NULL_POINTER);
	
	switch(attackType)
	{
		case DAEQSA_DENY:
			field = DOS_DAEQSA_ACTf;
			break;
		case LAND_DENY:
			field = DOS_LANDATTACKS_ACTf;
			break;
		case BLAT_DENY:
			field = DOS_BLATATTACKS_ACTf;
			break;
		case SYNFIN_DENY:
			field = DOS_SYNFINSCAN_ACTf;
			break;
		case XMA_DENY:
			field = DOS_XMASCAN_ACTf;
			break;
		case NULLSCAN_DENY:
			field = DOS_NULLSCAN_ACTf;
			break;
		case SYN_SPORTL1024_DENY:
			field = DOS_SYN1024_ACTf;
			break;
		case TCPHDR_MIN_CHECK:
			field = DOS_TCPSHORTHDR_ACTf;
			break;
		case TCP_FRAG_OFF_MIN_CHECK:
			field = DOS_TCPFRAGERROR_ACTf;
			break;
		case ICMP_FRAG_PKTS_DENY:
			field = DOS_ICMPFRAGMENT_ACTf;
			break;
		case POD_DENY:
			field = DOS_PINGOFDEATH_ACTf;
			break;
		case UDPDOMB_DENY:
			field = DOS_UDPBOMB_ACTf;
			break;
		case SYNWITHDATA_DENY:
			field = DOS_SYNWITHDATA_ACTf;
			break;
		case SYNFLOOD_DENY:
			field = DOS_SYNFLOOD_ACTf;
			break;
		case FINFLOOD_DENY:
			field = DOS_FINFLOOD_ACTf;
			break;
		case ICMPFLOOD_DENY:
			field = DOS_ICMPFLOOD_ACTf;
			break;
		default:
            return RT_ERR_CHIP_NOT_SUPPORTED;					
	}

    if ((ret = reg_field_read(DOS_CFGr,field, &dos_act)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_SEC | MOD_DAL), "");
        return ret;
    }
   
	switch(dos_act)
	{
	 	case RAW_DOS_ACT_TRAP:
	 		*pAction = ACTION_TRAP2CPU;
			break;
	 	case RAW_DOS_ACT_DROP:
	 		*pAction = ACTION_DROP;
			break;
		default:
            return RT_ERR_FAILED;
	}
    
    return RT_ERR_OK;
} /* end of apollo_raw_sec_attackPreventAction_get */

/* Function Name:
 *      apollo_raw_sec_floodThreshold_set
 * Description:
 *      Set system-based flood threshold.
 * Input:
 *      flood             - flood type
 *      threshold         - flood threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_sec_floodThreshold_set(rtk_sec_attackType_t attackType, uint32 threshold)
{
    int32   ret;

    RT_PARAM_CHK((APOLLO_SEC_FLOOD_THRESHOLD_MAX < threshold), RT_ERR_INPUT);
    RT_PARAM_CHK((ATTACK_TYPE_END <= attackType), RT_ERR_INPUT);

	switch(attackType)
	{
	    case SYNFLOOD_DENY:
            if ((ret = reg_field_write(DOS_SYNFLOOD_THr, THf, &threshold)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_SEC), "");
                return ret;
            }
	        break;
        case FINFLOOD_DENY:
            if ((ret = reg_field_write(DOS_FINFLOOD_THr, THf, &threshold)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_SEC), "");
                return ret;
            }
            break;
        case ICMPFLOOD_DENY:
            if ((ret = reg_field_write(DOS_ICMPFLOOD_THr, THf, &threshold)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_SEC), "");
                return ret;
            }
            break;
        default:
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_sec_floodThreshold_set */

/* Function Name:
 *      apollo_raw_sec_floodThreshold_get
 * Description:
 *      Get SYN flood threshold.
 * Input:
 *      flood             - flood type
 * Output:
 *      threshold         - SYN flood threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_sec_floodThreshold_get(rtk_sec_attackType_t attackType, uint32 *pThreshold)
{
    int32   ret;
    
    RT_PARAM_CHK((pThreshold == NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((ATTACK_TYPE_END <= attackType), RT_ERR_INPUT);

	switch(attackType)
	{
	    case SYNFLOOD_DENY:
            if ((ret = reg_field_read(DOS_SYNFLOOD_THr, THf, pThreshold)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_SEC), "");
                return ret;
            }
	        break;
        case FINFLOOD_DENY:
            if ((ret = reg_field_read(DOS_FINFLOOD_THr, THf, pThreshold)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_SEC), "");
                return ret;
            }    
            break;
        case ICMPFLOOD_DENY:
            if ((ret = reg_field_read(DOS_ICMPFLOOD_THr, THf, pThreshold)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_SEC), "");
                return ret;
            }    
            break;
        default:
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_sec_floodThreshold_get */


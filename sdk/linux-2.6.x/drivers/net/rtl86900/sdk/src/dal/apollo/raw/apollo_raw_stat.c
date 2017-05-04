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
 * Purpose : Definition those public statistic APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) statistic counter reset
 *           2) statistic counter get
 *
 */

#include <dal/apollo/raw/apollo_raw_stat.h>


/* Function Name:
 *      apollo_raw_stat_global_reset
 * Description:
 *      Reset the global counters
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32 apollo_raw_stat_global_reset(void)
{
    int32 ret;
    uint32 index;
    uint32 value;
    rtk_enable_t enable;
	rtk_port_t port;

    /*Check if ASIC is still reseting MIB or not*/
    if ((ret = reg_field_read(STAT_RSTr, RST_STATf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }
    if (value != 0)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return RT_ERR_BUSYWAIT_TIMEOUT;
    }

	/*Set ACL counter to reset*/
    enable = ENABLED;
    for (index = 0; index <= APOLLO_ACL_MIB_MAX; index++)
    {
        if ((ret = reg_array_field_write(STAT_ACL_CNT_RSTr, REG_ARRAY_INDEX_NONE, index, ENf, &enable)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
            return ret;
        }
    }

	/*Set Port counter to reset*/
	value = 1;
	for(port = 0; port <= APOLLO_PORTIDMAX; port ++)
    {
	    if ((ret = reg_array_field_write(STAT_PORT_RSTr, port, REG_ARRAY_INDEX_NONE, RST_PORT_MIBf, &value)) != RT_ERR_OK)
	    {
	        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
	        return ret;
	    }
	}

	/*Set Global counter to reset*/
	value = 1;
    if ((ret = reg_field_write(STAT_RSTr, RST_GLOBAL_MIBf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }


    /*Reset command*/
	value = 1;
    if ((ret = reg_field_write(EPON_STAT_RSTr, RST_CMDf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_cypress_stat_global_reset */

/* Function Name:
 *      apollo_raw_stat_port_reset
 * Description:
 *      Reset the port counters
 * Input:
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 * Note:
 *      None
 */
int32 apollo_raw_stat_port_reset(rtk_port_t port)
{
    int32 ret;
    uint32 value;

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);

    /*Check if ASIC is still reseting MIB or not*/
    if ((ret = reg_field_read(STAT_RSTr, RST_STATf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }
    if (value != 0)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return RT_ERR_BUSYWAIT_TIMEOUT;
    }

	/*Set Port counter to reset*/
	value = 1;
    if ((ret = reg_array_field_write(STAT_PORT_RSTr, port, REG_ARRAY_INDEX_NONE, RST_PORT_MIBf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }

    /*Reset command*/
	value = 1;
    if ((ret = reg_field_write(EPON_STAT_RSTr, RST_CMDf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_stat_acl_reset */

/* Function Name:
 *      apollo_raw_stat_acl_reset
 * Description:
 *      Reset the acl counters
 * Input:
 *      index - index of ACL counter index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_stat_acl_reset(uint32 index)
{
    int32 ret;
    uint32 value;
    rtk_enable_t enable;

    RT_PARAM_CHK((APOLLO_ACL_MIB_MAX < index), RT_ERR_INPUT);

    /*Check if ASIC is still reseting MIB or not*/
    if ((ret = reg_field_read(STAT_RSTr, RST_STATf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }
    if (value != 0)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return RT_ERR_BUSYWAIT_TIMEOUT;
    }

	/*Set ACL counter to reset*/
    enable = ENABLED;
    if ((ret = reg_array_field_write(STAT_ACL_CNT_RSTr, REG_ARRAY_INDEX_NONE, index, ENf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }

    /*Reset command*/
	value = 1;
    if ((ret = reg_field_write(EPON_STAT_RSTr, RST_CMDf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_stat_acl_reset */

/* Function Name:
 *      apollo_raw_stat_acl_mode_set
 * Description:
 *      Set the acl counters mode for 32-bits or 64-bits counter
 * Input:
 *      index 		- index of ACL counter
 *      mode 		- counter mode of ACL counter
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_stat_acl_mode_set(uint32 index, rtk_stat_logCnt_mode_t mode)
{
    int32 ret;
    uint32 idx;

    RT_PARAM_CHK((APOLLO_ACL_MIB_MAX <= index), RT_ERR_INPUT);
    RT_PARAM_CHK((index&1), RT_ERR_INPUT);
    RT_PARAM_CHK((STAT_LOG_MODE_END <= mode), RT_ERR_INPUT);

    idx = index / 2;
    if ((ret = reg_array_field_write(STAT_ACL_CNT_MODEr, REG_ARRAY_INDEX_NONE, idx, MODEf, &mode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_stat_acl_mode_set */

/* Function Name:
 *      apollo_raw_stat_acl_mode_get
 * Description:
 *      Get the acl counters mode for 32-bits or 64-bits counter
 * Input:
 *      index 		- index of ACL counter
 * Output:
 *      pMode 		- counter mode of ACL counter
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_stat_acl_mode_get(uint32 index, rtk_stat_logCnt_mode_t* pMode)
{
    int32 ret;
    uint32 idx;

    RT_PARAM_CHK((APOLLO_ACL_MIB_MAX <= index), RT_ERR_INPUT);
    RT_PARAM_CHK((index&1), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);

    idx = index / 2;
    if ((ret = reg_array_field_read(STAT_ACL_CNT_MODEr, REG_ARRAY_INDEX_NONE, idx, MODEf, pMode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_stat_acl_mode_get */

/* Function Name:
 *      apollo_raw_stat_acl_type_set
 * Description:
 *      Set the acl counters type for packet or byte counter
 * Input:
 *      index 		- index of ACL counter
 *      type		- counter type of ACL counter
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_stat_acl_type_set(uint32 index, rtk_stat_logCnt_type_t type)
{
    int32 ret;
    uint32 idx;

    RT_PARAM_CHK((APOLLO_ACL_MIB_MAX <= index), RT_ERR_INPUT);
    RT_PARAM_CHK((STAT_LOG_TYPE_END <= type), RT_ERR_INPUT);
    RT_PARAM_CHK((index&1), RT_ERR_INPUT);

    idx = index / 2;
    if ((ret = reg_array_field_write(STAT_ACL_CNT_TYPEr, REG_ARRAY_INDEX_NONE, idx, TYPEf, &type)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_stat_acl_type_set */

/* Function Name:
 *      apollo_raw_stat_acl_type_get
 * Description:
 *      Get the acl counters type for packet or byte counter
 * Input:
 *      index 		- index of ACL counter
 * Output:
 *      pType 		- counter type of ACL counter
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_stat_acl_type_get(uint32 index, rtk_stat_logCnt_type_t* pType)
{
    int32 ret;
    uint32 idx;

    RT_PARAM_CHK((APOLLO_ACL_MIB_MAX <= index), RT_ERR_INPUT);
    RT_PARAM_CHK((index&1), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pType), RT_ERR_NULL_POINTER);

    idx = index / 2;
    if ((ret = reg_array_field_read(STAT_ACL_CNT_TYPEr, REG_ARRAY_INDEX_NONE, idx, TYPEf, pType)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_stat_acl_type_get */


/* Function Name:
 *      apollo_raw_stat_port_get
 * Description:
 *      Get one specified port counter in the specified device.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      cntr_idx - specified port counter index
 * Output:
 *      pCntr   - pointer buffer of counter value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_PORT_ID
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 apollo_raw_stat_port_get(rtk_port_t port, rtk_stat_port_type_t cntr_idx, uint64 *pCntr)
{
    int32  ret;
    uint32 cntr_64Flag;
    uint32 cntr;
    uint32 reg;
    uint32 fieldL;
    uint32 fieldH;
    uint64 cntr64;


    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(cntr_idx >= MIB_PORT_CNTR_END, RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pCntr), RT_ERR_NULL_POINTER);

	cntr_64Flag = FALSE;
    switch(cntr_idx)
    {
    	case DOT1D_TP_PORT_IN_DISCARDS_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = DOT1DTPPORTINDISCARDSf;

            break;
    	case IF_IN_OCTETS_INDEX:
			reg = STAT_PORT_RX_MIBr;
			fieldH = IFINOCTETS_Hf;
			fieldL = IFINOCTETS_Lf;
			cntr_64Flag = TRUE;

            break;
    	case IF_IN_UCAST_PKTS_INDEX:
			reg = STAT_PORT_RX_MIBr;
			fieldL = IFINUCASTPKTSf;

            break;
    	case IF_IN_MULTICAST_PKTS_INDEX:
			reg = STAT_PORT_RX_MIBr;
			fieldL = IFINMULTICASTPKTSf;

            break;
    	case IF_IN_BROADCAST_PKTS_INDEX:
			reg = STAT_PORT_RX_MIBr;
			fieldL = IFINBROADCASTPKTSf;

            break;
    	case IF_OUT_OCTETS_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = IFOUTOCTETS_Lf;
			fieldH = IFOUTOCTETS_Hf;
			cntr_64Flag = TRUE;

            break;

        case IF_OUT_UCAST_PKTS_CNT_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = IFOUTUCASTPKTSf;

            break;
        case IF_OUT_MULTICAST_PKTS_CNT_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = IFOUTMULTICASTPKTSf;

            break;
        case IF_OUT_BROADCAST_PKTS_CNT_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = IFOUTBROADCASTPKTSf;

            break;
        case IF_OUT_DISCARDS_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = IFOUTDISCARDSf;

            break;
        case DOT3_STATS_SINGLE_COLLISION_FRAMES_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = DOT3STATSSINGLECOLLISIONFRAMESf;

            break;
        case DOT3_STATS_MULTIPLE_COLLISION_FRAMES_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = DOT3STATSMULTIPLECOLLISIONFRAMESf;

            break;
        case DOT3_STATS_DEFERRED_TRANSMISSIONS_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = DOT3STATSDEFERREDTRANSMISSIONSf;

            break;
        case DOT3_STATS_LATE_COLLISIONS_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = DOT3STATSLATECOLLISIONSf;

            break;
        case DOT3_STATS_EXCESSIVE_COLLISIONS_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = DOT3STATSEXCESSIVECOLLISIONSf;

           	break;
        case DOT3_STATS_SYMBOL_ERRORS_INDEX:
			reg = STAT_PORT_RX_MIBr;
			fieldL = DOT3STATSSYMBOLERRORSf;

           	break;
        case DOT3_CONTROL_IN_UNKNOWN_OPCODES_INDEX:
			reg = STAT_PORT_RX_MIBr;
			fieldL = DOT3CONTROLINUNKNOWNOPCODESf;

           	break;
        case DOT3_IN_PAUSE_FRAMES_INDEX:
			reg = STAT_PORT_RX_MIBr;
			fieldL = DOT3INPAUSEFRAMESf;

           	break;
        case DOT3_OUT_PAUSE_FRAMES_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = DOT3OUTPAUSEFRAMESf;

           	break;
        case ETHER_STATS_DROP_EVENTS_INDEX:
			reg = STAT_PORT_RX_MIBr;
			fieldL = ETHERSTATSDROPEVENTSf;

           	break;
        case ETHER_STATS_TX_BROADCAST_PKTS_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = TX_ETHERSTATSBROADCASTPKTSf;

           	break;

        case ETHER_STATS_TX_MULTICAST_PKTS_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = TX_ETHERSTATSMULTICASTPKTSf;

           	break;
        case ETHER_STATS_CRC_ALIGN_ERRORS_INDEX:
			reg = STAT_PORT_RX_MIBr;
			fieldL = ETHERSTATSCRCALIGNERRORSf;

           	break;

        case ETHER_STATS_TX_UNDER_SIZE_PKTS_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = TX_ETHERSTATSUNDERSIZEPKTSf;

           	break;
        case ETHER_STATS_RX_UNDER_SIZE_PKTS_INDEX:
			reg = STAT_PORT_RX_MIBr;
			fieldL = RX_ETHERSTATSUNDERSIZEPKTSf;

           	break;
        case ETHER_STATS_RX_UNDER_SIZE_DROP_PKTS_INDEX:
			reg = STAT_PORT_RX_MIBr;
			fieldL = RX_ETHERSTATSUNDERSIZEDROPPKTSf;

           	break;

        case ETHER_STATS_TX_OVERSIZE_PKTS_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = TX_ETHERSTATSOVERSIZEPKTSf;

           	break;

        case ETHER_STATS_RX_OVERSIZE_PKTS_INDEX:
			reg = STAT_PORT_RX_MIBr;
			fieldL = RX_ETHERSTATSOVERSIZEPKTSf;

           	break;
        case ETHER_STATS_FRAGMENTS_INDEX:
			reg = STAT_PORT_RX_MIBr;
			fieldL = ETHERSTATSFRAGMENTSf;

           	break;
        case ETHER_STATS_JABBERS_INDEX:
			reg = STAT_PORT_RX_MIBr;
			fieldL = ETHERSTATSJABBERSf;

           	break;
        case ETHER_STATS_COLLISIONS_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = ETHERSTATSCOLLISIONSf;

           	break;

        case ETHER_STATS_TX_PKTS_64OCTETS_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = TX_ETHERSTATSPKTS64OCTETSf;

           	break;
        case ETHER_STATS_RX_PKTS_64OCTETS_INDEX:
			reg = STAT_PORT_RX_MIBr;
			fieldL = RX_ETHERSTATSPKTS64OCTETSf;

           	break;
        case ETHER_STATS_TX_PKTS_65TO127OCTETS_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = TX_ETHERSTATSPKTS65TO127OCTETSf;

           	break;
        case ETHER_STATS_RX_PKTS_65TO127OCTETS_INDEX:
			reg = STAT_PORT_RX_MIBr;
			fieldL = RX_ETHERSTATSPKTS65TO127OCTETSf;

           	break;
        case ETHER_STATS_TX_PKTS_128TO255OCTETS_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = TX_ETHERSTATSPKTS128TO255OCTETSf;

           	break;
        case ETHER_STATS_RX_PKTS_128TO255OCTETS_INDEX:
			reg = STAT_PORT_RX_MIBr;
			fieldL = RX_ETHERSTATSPKTS128TO255OCTETSf;

           	break;
        case ETHER_STATS_TX_PKTS_256TO511OCTETS_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = TX_ETHERSTATSPKTS256TO511OCTETSf;

           	break;
        case ETHER_STATS_RX_PKTS_256TO511OCTETS_INDEX:
			reg = STAT_PORT_RX_MIBr;
			fieldL = RX_ETHERSTATSPKTS256TO511OCTETSf;

           	break;
        case ETHER_STATS_TX_PKTS_512TO1023OCTETS_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = TX_ETHERSTATSPKTS512TO1023OCTETSf;

           	break;
        case ETHER_STATS_RX_PKTS_512TO1023OCTETS_INDEX:
			reg = STAT_PORT_RX_MIBr;
			fieldL = RX_ETHERSTATSPKTS512TO1023OCTETSf;

           	break;
        case ETHER_STATS_TX_PKTS_1024TO1518OCTETS_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = TX_ETHERSTATSPKTS1024TO1518OCTETSf;

           	break;
        case ETHER_STATS_RX_PKTS_1024TO1518OCTETS_INDEX:
			reg = STAT_PORT_RX_MIBr;
			fieldL = RX_ETHERSTATSPKTS1024TO1518OCTETSf;

           	break;
        case ETHER_STATS_TX_PKTS_1519TOMAXOCTETS_INDEX:
			reg = STAT_PORT_TX_MIBr;
			fieldL = TX_ETHERSTATSPKTS1519TOMAXOCTETSf;

           	break;
        case ETHER_STATS_RX_PKTS_1519TOMAXOCTETS_INDEX:
			reg = STAT_PORT_RX_MIBr;
			fieldL = RX_ETHERSTATSPKTS1519TOMAXOCTETSf;

           	break;
        case IN_OAM_PDU_PKTS_INDEX:
			reg = STAT_PORT_OAM_MIBr;
			fieldL = INOAMPDUPKTSf;

           	break;
        case OUT_OAM_PDU_PKTS_INDEX:
			reg = STAT_PORT_OAM_MIBr;
			fieldL = OUTOAMPDUPKTSf;

           	break;
        default:
            return RT_ERR_INPUT;;
    }


	if(cntr_64Flag)
    {
	    if ((ret = reg_array_field_read(reg, port, REG_ARRAY_INDEX_NONE,fieldH, &cntr)) != RT_ERR_OK)
	    {
	        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
	        return ret;
	    }

		cntr64 = cntr;
	}
	else
		cntr64 = 0;


	if ((ret = reg_array_field_read(reg, port, REG_ARRAY_INDEX_NONE,fieldL, &cntr)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }

	cntr64 = (cntr64 << 32) | cntr;

	*pCntr = cntr64;

    return RT_ERR_OK;
} /* end of apollo_raw_stat_port_get */


/* Function Name:
 *      apollo_raw_stat_global_get
 * Description:
 *      Get one specified global counter in the specified device.
 * Input:
 *      cntr_idx - specified global counter index
 * Output:
 *      pCntr   - pointer buffer of counter value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER             - input parameter may be null pointer
 *      RT_ERR_STAT_GLOBAL_CNTR_FAIL    - Could not retrieve/reset Global Counter
 *      RT_ERR_STAT_INVALID_GLOBAL_CNTR - Invalid Global Counter
 * Note:
 *      None
 */
int32 apollo_raw_stat_global_get(rtk_stat_global_type_t cntr_idx, uint64 *pCntr)
{
    int32 ret;
    uint32 cntr;

    RT_PARAM_CHK((MIB_GLOBAL_CNTR_END <= cntr_idx), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pCntr), RT_ERR_NULL_POINTER);

    switch(cntr_idx)
    {
        case DOT1D_TP_LEARNED_ENTRY_DISCARDS_INDEX:
			if ((ret = reg_read(STAT_BRIDGE_DOT1DTPLEARNEDENTRYDISCARDSr, &cntr)) != RT_ERR_OK)
		    {
		        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
		        return ret;
		    }
            break;
        default:
            return RT_ERR_INPUT;;
    }

    *pCntr = cntr;

    return RT_ERR_OK;
} /*dal_cypress_stat_global_get  */

/* Function Name:
 *      apollo_raw_stat_acl_get
 * Description:
 *      Get acl counters
 * Input:
 *      index - index of ACL counter index
 * Output:
 *      pCntr   - pointer buffer of counter value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_stat_acl_get(uint32 index, uint64 *pCntr)
{
    int32 ret = RT_ERR_FAILED;
    uint32 cntr;

    RT_PARAM_CHK((APOLLO_ACL_MIB_MAX < index), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pCntr), RT_ERR_NULL_POINTER);

    if ((ret = reg_array_read(STAT_ACL_CNTr, REG_ARRAY_INDEX_NONE, index, &cntr)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }

    *pCntr = cntr;

    return RT_ERR_OK;
} /* end of apollo_raw_stat_acl_get */

/* Function Name:
 *      apollo_raw_stat_pktInfo_get
 * Description:
 *      Get the newest packet trap/drop reason
 * Input:
 *      port - port index
 * Output:
 *      pCode   - the newest packet trap/drop reason code
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 * Note:
 *      None
 */
int32 apollo_raw_stat_pktInfo_get(rtk_port_t port, uint32 *pCode)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pCode), RT_ERR_NULL_POINTER);

    if ((ret = reg_array_field_read(STAT_PRVTE_DROP_COUNTERr, port, REG_ARRAY_INDEX_NONE, PKT_INFOf, pCode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_stat_pktInfo_get */

/* Function Name:
 *      apollo_raw_stat_rst_mib_value_set
 * Description:
 *      Set MIB counter reset value
 * Input:
 *      rst_value        - MIB counter reset value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_stat_rst_mib_value_set(rtk_mib_rst_value_t rst_value)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK((STAT_MIB_RST_END <= rst_value), RT_ERR_INPUT);

    if ((ret = reg_field_write(STAT_RSTr, RST_MIB_VALf, &rst_value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_stat_rst_mib_value_set */

/* Function Name:
 *      apollo_raw_stat_rst_mib_value_get
 * Description:
 *      Get mib reset value
 * Input:
 *      None
 * Output:
 *      pRst_value       - pointer buffer of counter mode of ACL counter
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_stat_rst_mib_value_get(rtk_mib_rst_value_t *pRst_value)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK((NULL == pRst_value), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(STAT_RSTr, RST_MIB_VALf, pRst_value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_stat_rst_mib_value_get */


/* Function Name:
 *      apollo_raw_stat_acl_counter_get
 * Description:
 *      Get counters for ACL action
 * Input:
 *      index 		- index of ACL counter
 * Output:
 *      pCnt 		- pointer buffer of ACL counter
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_stat_acl_counter_get(uint32 index, uint32 *pCnt)
{
    int32 ret;

    RT_PARAM_CHK((APOLLO_ACL_MIB_MAX < index), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pCnt), RT_ERR_NULL_POINTER);

    if ((ret = reg_array_field_read(STAT_ACL_CNTr, REG_ARRAY_INDEX_NONE, index, LOGGINGCOUNTERf, pCnt)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_stat_acl_counter_get */

/* Function Name:
 *      apollo_raw_stat_mib_cnt_mode_get
 * Description:
 *      Get the MIB data update mode
 * Input:
 *      None
 * Output:
 *      pCnt_mode   - pointer buffer of MIB data update mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_stat_mib_cnt_mode_get(rtk_mib_count_mode_t *pCnt_mode)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK((NULL == pCnt_mode), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(STAT_CTRLr, CNTING_MODEf, pCnt_mode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_stat_mib_cnt_mode_get */

/* Function Name:
 *      apollo_raw_stat_mib_cnt_mode_set
 * Description:
 *      Set MIB data update mode
 * Input:
 *      cnt_mode        - MIB counter update mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_stat_mib_cnt_mode_set(rtk_mib_count_mode_t cnt_mode)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK((STAT_MIB_COUNT_MODE_END <= cnt_mode), RT_ERR_INPUT);

    if ((ret = reg_field_write(STAT_CTRLr, CNTING_MODEf, &cnt_mode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_stat_mib_cnt_mode_set */


/* Function Name:
 *      apollo_raw_stat_mib_latch_timer_get
 * Description:
 *      Get the MIB latch timer
 * Input:
 *      None
 * Output:
 *      pTimer   - pointer buffer of MIB latch timer
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_stat_mib_latch_timer_get(uint32 *pTimer)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK((NULL == pTimer), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(STAT_CTRLr, LATCH_TIMERf, pTimer)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_stat_mib_latch_timer_get */

/* Function Name:
 *      apollo_raw_stat_mib_latch_timer_set
 * Description:
 *      Set MIB data update mode
 * Input:
 *      timer        - MIB latch timer
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_stat_mib_latch_timer_set(uint32 timer)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK((APOLLO_MIB_LATCH_TIMER_MAX < timer), RT_ERR_INPUT);

    if ((ret = reg_field_write(STAT_CTRLr, LATCH_TIMERf, &timer)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_stat_mib_latch_timer_set */

/* Function Name:
 *      apollo_raw_stat_mib_sync_mode_get
 * Description:
 *      Get the MIB register data update mode
 * Input:
 *      None
 * Output:
 *      pSync_mode   - pointer buffer of MIB register data update mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_stat_mib_sync_mode_get(rtk_mib_sync_mode_t *pSync_mode)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK((NULL == pSync_mode), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(STAT_CTRLr, SYNC_MODEf, pSync_mode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_stat_mib_sync_mode_get */

/* Function Name:
 *      apollo_raw_stat_mib_sync_mode_set
 * Description:
 *      Set MIB register data update mode
 * Input:
 *      sync_mode        - MIB register data update mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_stat_mib_sync_mode_set(rtk_mib_sync_mode_t sync_mode)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK((STAT_MIB_SYNC_MODE_END <= sync_mode), RT_ERR_INPUT);

    if ((ret = reg_field_write(STAT_CTRLr, SYNC_MODEf, &sync_mode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_stat_mib_sync_mode_set */

/* Function Name:
 *      apollo_raw_stat_mib_count_tag_length_get
 * Description:
 *      Get count Tag length in tx/rx packet state
 * Input:
 *      direction - count tx or rx tag length
 * Output:
 *      pState   - pointer buffer of count tx/rx tag length state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_stat_mib_count_tag_length_get(rtk_mib_tag_cnt_dir_t direction, rtk_mib_tag_cnt_state_t *pState)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK((STAT_MIB_TAG_CNT_DIR_END <= direction), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pState), RT_ERR_NULL_POINTER);

    if( APOLLO_MIB_TAG_CNT_DIR_TX == direction)
    {
        if ((ret = reg_field_read(STAT_CTRLr, TX_CNT_CTAGf, pState)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
            return ret;
        }
    }
    else
    {
        if ((ret = reg_field_read(STAT_CTRLr, RX_CNT_CTAGf, pState)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of apollo_raw_stat_mib_count_tag_length_get */

/* Function Name:
 *      apollo_raw_stat_mib_count_tag_length_set
 * Description:
 *      Set MIB register data update mode
 * Input:
 *      direction - count tx or rx tag length
 *      enable    - count tag length state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_stat_mib_count_tag_length_set(rtk_mib_tag_cnt_dir_t direction, rtk_mib_tag_cnt_state_t state)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK((STAT_MIB_TAG_CNT_DIR_END <= direction), RT_ERR_INPUT);
    RT_PARAM_CHK((STAT_MIB_TAG_CNT_STATE_END <= state), RT_ERR_INPUT);

    if( APOLLO_MIB_TAG_CNT_DIR_TX == direction)
    {
        if ((ret = reg_field_write(STAT_CTRLr, TX_CNT_CTAGf, &state)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
            return ret;
        }
    }
    else
    {
        if ((ret = reg_field_write(STAT_CTRLr, RX_CNT_CTAGf, &state)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of apollo_raw_stat_mib_count_tag_length_set */


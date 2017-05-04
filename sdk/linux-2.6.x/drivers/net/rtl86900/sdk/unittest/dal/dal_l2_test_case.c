#include <osal/lib.h>
#include <osal/print.h>
#include <ioal/mem32.h>
#include <hal/common/halctrl.h>
#include <common/error.h>
#include <dal/dal_l2_test_case.h>
#include <rtk/l2.h>
#include <common/unittest_util.h>

int32 dal_l2_age_test(uint32 caseNo)
{
    uint32 timeW;
    uint32 timeR;

    /*error input check*/
    /*out of range*/
    if(rtk_l2_aging_set(HAL_L2_AGING_TIME_MAX() + 1) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /*null pointer*/
    if(rtk_l2_aging_get(NULL) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /* Get/set */
    for(timeW = 100; timeW <= HAL_L2_AGING_TIME_MAX(); timeW+=100)
    {
        if(rtk_l2_aging_set(timeW) != RT_ERR_OK)
        {
            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
            return RT_ERR_FAILED;
        }

        if(rtk_l2_aging_get(&timeR) != RT_ERR_OK)
        {
            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
            return RT_ERR_FAILED;
        }

        if(timeW != timeR)
        {
            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
            return RT_ERR_FAILED;
        }
    }

    return RT_ERR_OK;
}

int32 dal_l2_extPortEgrFilterMask_test(uint32 caseNo)
{
    rtk_portmask_t portmaskW;
    rtk_portmask_t portmaskR;


    portmaskW.bits[0] = 0x0;
    if(rtk_l2_extPortEgrFilterMask_set(&portmaskW) == RT_ERR_DRIVER_NOT_FOUND)
    {
        return RT_ERR_OK;
    }

    /*error input check*/
    /*out of range*/
    portmaskW.bits[0] = 0xFF;
    if(rtk_l2_extPortEgrFilterMask_set(&portmaskW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /*null pointer*/
    if(rtk_l2_extPortEgrFilterMask_get(NULL) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /* Get/set */
    for(portmaskW.bits[0] = 0x00; HAL_IS_EXTPORTMASK_VALID(portmaskW); portmaskW.bits[0]++)
    {
        if(!RTK_PORTMASK_IS_PORT_SET(portmaskW, HAL_GET_EXT_CPU_PORT()))
        {
            if(rtk_l2_extPortEgrFilterMask_set(&portmaskW) != RT_ERR_OK)
            {
                osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                return RT_ERR_FAILED;
            }

            if(rtk_l2_extPortEgrFilterMask_get(&portmaskR) != RT_ERR_OK)
            {
                osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                return RT_ERR_FAILED;
            }

            if(RTK_PORTMASK_COMPARE(portmaskW, portmaskR) != 0)
            {
                osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                return RT_ERR_FAILED;
            }
        }
    }

    return RT_ERR_OK;
}

int32 dal_l2_flushLinkDownPortAddrEnable_test(uint32 caseNo)
{
    rtk_enable_t stateW;
    rtk_enable_t stateR;

    /*error input check*/
    /*out of range*/
    if(rtk_l2_flushLinkDownPortAddrEnable_set(RTK_ENABLE_END) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /*null pointer*/
    if(rtk_l2_flushLinkDownPortAddrEnable_get(NULL) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    for(stateW = DISABLED; stateW < RTK_ENABLE_END; stateW++)
    {
        if(rtk_l2_flushLinkDownPortAddrEnable_set(stateW) != RT_ERR_OK)
        {
            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
            return RT_ERR_FAILED;
        }

        if(rtk_l2_flushLinkDownPortAddrEnable_get(&stateR) != RT_ERR_OK)
        {
            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
            return RT_ERR_FAILED;
        }

        if(stateW != stateR)
        {
            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
            return RT_ERR_FAILED;
        }
    }

    /* Get/set */
    return RT_ERR_OK;
}

int32 dal_l2_illegalPortMoveAction_test(uint32 caseNo)
{
    rtk_port_t port;
    rtk_action_t actW;
    rtk_action_t actR;
    

    if(rtk_l2_illegalPortMoveAction_set(HAL_GET_MAX_PORT(), ACTION_FORWARD) == RT_ERR_DRIVER_NOT_FOUND)
    {
        return RT_ERR_OK;
    }
    
    /*error input check*/
    /*out of range*/
    if(rtk_l2_illegalPortMoveAction_set(HAL_GET_MAX_PORT() + 1, ACTION_FORWARD) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    if(rtk_l2_illegalPortMoveAction_set(0, ACTION_END) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /*null pointer*/
    if(rtk_l2_illegalPortMoveAction_get(0, NULL) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /* Get/set */
    HAL_SCAN_ALL_PORT(port)
    {
        for(actW = ACTION_FORWARD; actW < ACTION_END; actW++)
        {
            if( (actW == ACTION_FORWARD) || (actW == ACTION_DROP) || (actW == ACTION_TRAP2CPU) || (actW == ACTION_COPY2CPU) )
            {
                if(rtk_l2_illegalPortMoveAction_set(port, actW) != RT_ERR_OK)
                {
                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                    return RT_ERR_FAILED;
                }

                if(rtk_l2_illegalPortMoveAction_get(port, &actR) != RT_ERR_OK)
                {
                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                    return RT_ERR_FAILED;
                }

                if(actW != actR)
                {
                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                    return RT_ERR_FAILED;
                }
            }
        }
    }


    return RT_ERR_OK;
}

int32 dal_l2_portAgingEnable_test(uint32 caseNo)
{
    rtk_port_t port;
    rtk_enable_t stateW;
    rtk_enable_t stateR;

    /*error input check*/
    /*out of range*/
    if(rtk_l2_portAgingEnable_set(HAL_GET_MAX_PORT() + 1, ENABLED) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    if(rtk_l2_portAgingEnable_set(0, RTK_ENABLE_END) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /*null pointer*/
    if(rtk_l2_portAgingEnable_get(0, NULL) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /* Get/set */
    HAL_SCAN_ALL_PORT(port)
    {
        for(stateW = DISABLED; stateW < RTK_ENABLE_END; stateW++)
        {
            if(rtk_l2_portAgingEnable_set(port, stateW) != RT_ERR_OK)
            {
                osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                return RT_ERR_FAILED;
            }

            if(rtk_l2_portAgingEnable_get(port, &stateR) != RT_ERR_OK)
            {
                osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                return RT_ERR_FAILED;
            }

            if(stateW != stateR)
            {
                osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                return RT_ERR_FAILED;
            }
        }
    }

    return RT_ERR_OK;
}

int32 dal_l2_ipmcGroupLookupMissHash_test(uint32 caseNo)
{
    rtk_l2_ipmcHashOp_t hashW;
    rtk_l2_ipmcHashOp_t hashR;

    if(rtk_l2_ipmcGroupLookupMissHash_set(HASH_DIP_AND_SIP) == RT_ERR_DRIVER_NOT_FOUND)
    {
        return RT_ERR_OK;
    }
    
    /*error input check*/
    /*out of range*/
    if(rtk_l2_ipmcGroupLookupMissHash_set(HASH_END) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /*null pointer*/
    if(rtk_l2_ipmcGroupLookupMissHash_get(NULL) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /* Get/set */
    for(hashW = HASH_DIP_AND_SIP; hashW < HASH_END; hashW++)
    {
        if(rtk_l2_ipmcGroupLookupMissHash_set(hashW) != RT_ERR_OK)
        {
            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
            return RT_ERR_FAILED;
        }

        if(rtk_l2_ipmcGroupLookupMissHash_get(&hashR) != RT_ERR_OK)
        {
            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
            return RT_ERR_FAILED;
        }

        if(hashW != hashR)
        {
            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
            return RT_ERR_FAILED;
        }
    }

    return RT_ERR_OK;
}

int32 dal_l2_ipmcMode_test(uint32 caseNo)
{
    rtk_l2_ipmcMode_t modeW;
    rtk_l2_ipmcMode_t modeR;
    /*error input check*/
    /*out of range*/
#if CONFIG_SDK_RTL9602C
    for(modeW = LOOKUP_ON_MAC_AND_VID_FID; modeW < IPMC_MODE_END; modeW ++)
    {
        switch(modeW)
        {
        case LOOKUP_ON_MAC_AND_VID_FID:
        case LOOKUP_ON_DIP_AND_SIP:
        case LOOKUP_ON_DIP_AND_VID:
            continue;
        case LOOKUP_ON_DIP:
        case LOOKUP_ON_DIP_AND_CVID:
        case IPMC_MODE_END:
            break;
        default:
            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
            return RT_ERR_FAILED;
        }
        if(rtk_l2_ipmcMode_set(modeW) == RT_ERR_OK)
        {
            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
            return RT_ERR_FAILED;
        }
    }
#else
    if(rtk_l2_ipmcMode_set(IPMC_MODE_END) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }
#endif

    /*null pointer*/
    if(rtk_l2_ipmcMode_get(NULL) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /* Get/set */
    for(modeW = LOOKUP_ON_MAC_AND_VID_FID; modeW < IPMC_MODE_END; modeW++)
    {
#ifdef CONFIG_SDK_RTL9602C
        switch(modeW)
        {
        case LOOKUP_ON_MAC_AND_VID_FID:
        case LOOKUP_ON_DIP_AND_SIP:
        case LOOKUP_ON_DIP_AND_VID:
            break;;
        case LOOKUP_ON_DIP:
        case LOOKUP_ON_DIP_AND_CVID:
        case IPMC_MODE_END:
            continue;
        default:
            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
            return RT_ERR_FAILED;
        }
#endif

        if(rtk_l2_ipmcMode_set(modeW) != RT_ERR_OK)
        {
            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
            return RT_ERR_FAILED;
        }

        if(rtk_l2_ipmcMode_get(&modeR) != RT_ERR_OK)
        {
            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
            return RT_ERR_FAILED;
        }

        if(modeW != modeR)
        {
            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
            return RT_ERR_FAILED;
        }
    }

    return RT_ERR_OK;
}

int32 dal_l2_limitLearningCnt_test(uint32 caseNo)
{
    uint32 cntW;
    uint32 cntR;

    if(rtk_l2_limitLearningCnt_set(1) == RT_ERR_DRIVER_NOT_FOUND)
    {
        return RT_ERR_OK;
    }

    /*error input check*/
    /*out of range*/
    if(rtk_l2_limitLearningCnt_set(HAL_L2_LEARN_LIMIT_CNT_MAX() + 1) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /*null pointer*/
    if(rtk_l2_limitLearningCnt_get(NULL) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    if(rtk_l2_learningCnt_get(NULL) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /* Get/set */
    for(cntW = 0; cntW <= HAL_L2_LEARN_LIMIT_CNT_MAX(); cntW+=32 )
    {
        if(rtk_l2_limitLearningCnt_set(cntW) != RT_ERR_OK)
        {
            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
            return RT_ERR_FAILED;
        }

        if(rtk_l2_limitLearningCnt_get(&cntR) != RT_ERR_OK)
        {
            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
            return RT_ERR_FAILED;
        }

        if(cntW != cntR)
        {
            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
            return RT_ERR_FAILED;
        }
    }

    if(rtk_l2_learningCnt_get(&cntR) != RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
}

int32 dal_l2_limitLearningCntAction_test(uint32 caseNo)
{
    rtk_l2_limitLearnCntAction_t actW;
    rtk_l2_limitLearnCntAction_t actR;

    if(rtk_l2_limitLearningCnt_set(1) == RT_ERR_DRIVER_NOT_FOUND)
    {
        return RT_ERR_OK;
    }

    /*error input check*/
    /*out of range*/
    if(rtk_l2_limitLearningCntAction_set(LIMIT_LEARN_CNT_ACTION_END) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /*null pointer*/
    if(rtk_l2_limitLearningCntAction_get(NULL) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /* Get/set */
    for(actW = LIMIT_LEARN_CNT_ACTION_DROP; actW < LIMIT_LEARN_CNT_ACTION_END; actW++)
    {
        if( (actW == LIMIT_LEARN_CNT_ACTION_DROP) ||
            (actW == LIMIT_LEARN_CNT_ACTION_FORWARD) ||
            (actW == LIMIT_LEARN_CNT_ACTION_TO_CPU) ||
            (actW == LIMIT_LEARN_CNT_ACTION_COPY_CPU) )
        {
            if(rtk_l2_limitLearningCntAction_set(actW) != RT_ERR_OK)
            {
                osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                return RT_ERR_FAILED;
            }

            if(rtk_l2_limitLearningCntAction_get(&actR) != RT_ERR_OK)
            {
                osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                return RT_ERR_FAILED;
            }

            if(actW != actW)
            {
                osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                return RT_ERR_FAILED;
            }
        }
    }

    return RT_ERR_OK;
}


int32 dal_l2_srcPortEgrFilterMask_test(uint32 caseNo)
{
    rtk_portmask_t portmaskW;
    rtk_portmask_t portmaskR;

    /*error input check*/
    /*out of range*/
    portmaskW.bits[0] = 0xFF;
    if(rtk_l2_srcPortEgrFilterMask_set(&portmaskW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /*null pointer*/
    if(rtk_l2_srcPortEgrFilterMask_get(NULL) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /* Get/set */
    for(portmaskW.bits[0] = 0x00; HAL_IS_PORTMASK_VALID(portmaskW); portmaskW.bits[0]++)
    {
        if(rtk_l2_srcPortEgrFilterMask_set(&portmaskW) != RT_ERR_OK)
        {
            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
            return RT_ERR_FAILED;
        }

        if(rtk_l2_srcPortEgrFilterMask_get(&portmaskR) != RT_ERR_OK)
        {
            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
            return RT_ERR_FAILED;
        }

        if(RTK_PORTMASK_COMPARE(portmaskW, portmaskR) != 0)
        {
            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
            return RT_ERR_FAILED;
        }
    }

    return RT_ERR_OK;

}

int32 dal_l2_newMacOp_test(uint32 caseNo)
{
    rtk_port_t port;
    rtk_action_t actW;
    rtk_action_t actR;
    rtk_l2_newMacLrnMode_t modeR;

    /*error input check*/
    /*out of range*/
    if(rtk_l2_newMacOp_set(HAL_GET_MAX_PORT() + 1, HARDWARE_LEARNING, ACTION_FORWARD) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    if(rtk_l2_newMacOp_set(0, SOFTWARE_LEARNING, ACTION_FORWARD) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    if(rtk_l2_newMacOp_set(0, NOT_LEARNING, ACTION_FORWARD) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    if(rtk_l2_newMacOp_set(0, HARDWARE_LEARNING, ACTION_END) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /*null pointer*/
    if(rtk_l2_newMacOp_get(0, NULL, NULL) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /* Get/set */
    HAL_SCAN_ALL_PORT(port)
    {
        for(actW = ACTION_FORWARD; actW <= ACTION_END; actW++)
        {
            if( (actW == ACTION_FORWARD) || (actW == ACTION_DROP) || (actW == ACTION_TRAP2CPU) || (actW == ACTION_COPY2CPU) )
            {
                if(rtk_l2_newMacOp_set(port, HARDWARE_LEARNING, actW) != RT_ERR_OK)
                {
                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                    return RT_ERR_FAILED;
                }

                if(rtk_l2_newMacOp_get(port, &modeR, &actR) != RT_ERR_OK)
                {
                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                    return RT_ERR_FAILED;
                }

                if(actW != actR)
                {
                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                    return RT_ERR_FAILED;
                }
            }
        }
    }

    return RT_ERR_OK;
}


int32 dal_l2_lookupMissAction_test(uint32 caseNo)
{
    rtk_l2_lookupMissType_t type;
    rtk_action_t actW;
    rtk_action_t actR;

    /*error input check*/
    /*out of range*/
    if(rtk_l2_lookupMissAction_set(DLF_TYPE_END, ACTION_FORWARD) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    if(rtk_l2_lookupMissAction_set(DLF_TYPE_MCAST, ACTION_END) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /*null pointer*/
    if(rtk_l2_lookupMissAction_get(DLF_TYPE_MCAST, NULL) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /* Get/set */
    for(type = DLF_TYPE_IPMC; type < DLF_TYPE_END; type++)
    {
        if(type != DLF_TYPE_BCAST)
        {
            for(actW = ACTION_FORWARD; actW < ACTION_END; actW++)
            {
                if( (actW == ACTION_FORWARD) || (actW == ACTION_DROP) || (actW == ACTION_TRAP2CPU) || ((actW == ACTION_DROP_EXCLUDE_RMA) && (type == DLF_TYPE_MCAST)) )
                {
                    if(rtk_l2_lookupMissAction_set(type, actW) != RT_ERR_OK)
                    {
                        osal_printf("\n %d %d", type, actW);
                        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                        return RT_ERR_FAILED;
                    }

                    if(rtk_l2_lookupMissAction_get(type, &actR) != RT_ERR_OK)
                    {
                        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                        return RT_ERR_FAILED;
                    }

                    if(actW != actR)
                    {
                        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                        return RT_ERR_FAILED;
                    }
                }
            }
        }
    }

    return RT_ERR_OK;
}

int32 dal_l2_lookupMissFloodPortMask_test(uint32 caseNo)
{
    rtk_l2_lookupMissType_t type;
    rtk_portmask_t portmaskW;
    rtk_portmask_t portmaskR;

    /*error input check*/
    /*out of range*/
    RTK_PORTMASK_RESET(portmaskW);
    if(rtk_l2_lookupMissFloodPortMask_set(DLF_TYPE_END, &portmaskW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    portmaskW.bits[0] = 0xFF;
    if(rtk_l2_lookupMissFloodPortMask_set(DLF_TYPE_MCAST, &portmaskW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /*null pointer*/
    if(rtk_l2_lookupMissFloodPortMask_get(DLF_TYPE_MCAST, NULL) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /* Get/set */
    for(type = DLF_TYPE_IPMC; type < DLF_TYPE_END; type++)
    {
        for(portmaskW.bits[0] = 0x00; HAL_IS_PORTMASK_VALID(portmaskW); portmaskW.bits[0]++)
        {
            if(rtk_l2_lookupMissFloodPortMask_set(type, &portmaskW) != RT_ERR_OK)
            {
                osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                return RT_ERR_FAILED;
            }

            if(rtk_l2_lookupMissFloodPortMask_get(type, &portmaskR) != RT_ERR_OK)
            {
                osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                return RT_ERR_FAILED;
            }

            if(RTK_PORTMASK_COMPARE(portmaskW, portmaskR) != 0)
            {
                osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                return RT_ERR_FAILED;
            }
        }
    }

    return RT_ERR_OK;
}

int32 dal_l2_addr_test(uint32 caseNo)
{
    rtk_vlan_t  vid;
    uint32      mac;
    uint32      fid;
    uint32      efid;
    rtk_port_t  port;
    uint32      ext_port;
    uint32      flags;          /* Refer to RTK_L2_UCAST_FLAG_XXX */
    uint32      priority;
    uint8       auth;

    int32       index;

    rtk_l2_ucastAddr_t l2AddrW;
    rtk_l2_ucastAddr_t l2AddrR;

    int32 retVal;

    /*error input check*/
    /*out of range*/
    osal_memset(&l2AddrW, 0x00, sizeof(rtk_l2_ucastAddr_t));
    l2AddrW.vid = RTK_VLAN_ID_MAX + 1;
    if(rtk_l2_addr_add(&l2AddrW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    osal_memset(&l2AddrW, 0x00, sizeof(rtk_l2_ucastAddr_t));
    l2AddrW.mac.octet[0] = 0x01;
    if(rtk_l2_addr_add(&l2AddrW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

#ifdef CONFIG_SDK_RTL9601B    
    osal_memset(&l2AddrW, 0x00, sizeof(rtk_l2_ucastAddr_t));
    l2AddrW.ctag_vid = RTK_VLAN_ID_MAX + 1;
    if(rtk_l2_addr_add(&l2AddrW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }
#else
    osal_memset(&l2AddrW, 0x00, sizeof(rtk_l2_ucastAddr_t));
    l2AddrW.fid = HAL_VLAN_FID_MAX() + 1;
    if(rtk_l2_addr_add(&l2AddrW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

#if !defined(CONFIG_SDK_RTL9602C)
    osal_memset(&l2AddrW, 0x00, sizeof(rtk_l2_ucastAddr_t));
    l2AddrW.efid = HAL_ENHANCED_FID_MAX() + 1;
    if(rtk_l2_addr_add(&l2AddrW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }
#endif

    osal_memset(&l2AddrW, 0x00, sizeof(rtk_l2_ucastAddr_t));
    l2AddrW.priority = HAL_INTERNAL_PRIORITY_MAX() + 1;
    if(rtk_l2_addr_add(&l2AddrW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    osal_memset(&l2AddrW, 0x00, sizeof(rtk_l2_ucastAddr_t));
    l2AddrW.auth = RTK_ENABLE_END;
    if(rtk_l2_addr_add(&l2AddrW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }
    
    osal_memset(&l2AddrW, 0x00, sizeof(rtk_l2_ucastAddr_t));
    l2AddrW.ext_port = HAL_GET_MAX_EXT_PORT() + 1;
    if(rtk_l2_addr_add(&l2AddrW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }
#endif

    osal_memset(&l2AddrW, 0x00, sizeof(rtk_l2_ucastAddr_t));
    l2AddrW.port = HAL_GET_MAX_PORT() + 1;
    if(rtk_l2_addr_add(&l2AddrW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    osal_memset(&l2AddrW, 0x00, sizeof(rtk_l2_ucastAddr_t));
    l2AddrW.flags = RTK_L2_UCAST_FLAG_ALL + 1;
    if(rtk_l2_addr_add(&l2AddrW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    osal_memset(&l2AddrW, 0x00, sizeof(rtk_l2_ucastAddr_t));
    l2AddrW.age = 8;
    if(rtk_l2_addr_add(&l2AddrW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /*null pointer*/
    if(rtk_l2_addr_get(NULL) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /* Get/set */
    for(vid = 1; vid <= RTK_VLAN_ID_MAX; vid+=4094)
    {
        for(mac = 0x01; mac <= 0xFF; mac += 0xFE)
        {
            for(fid = 0; fid <= HAL_VLAN_FID_MAX(); fid+=HAL_VLAN_FID_MAX())
            {
                for(efid = 0; efid <= HAL_ENHANCED_FID_MAX(); efid+=HAL_ENHANCED_FID_MAX())
                {
                    for(port = 0; port <= HAL_GET_MAX_PORT(); port+=HAL_GET_MAX_PORT())
                    {
                        for(ext_port = 0; ext_port <= HAL_GET_MAX_EXT_PORT(); ext_port+=HAL_GET_MAX_EXT_PORT())
                        {
                            if(ext_port != HAL_GET_EXT_CPU_PORT())
                            {
                                for(flags = RTK_L2_UCAST_FLAG_SA_BLOCK; flags <= RTK_L2_UCAST_FLAG_ALL; flags = (flags << 1))
                                {
                                    for(priority = 0; priority <= HAL_INTERNAL_PRIORITY_MAX(); priority+=HAL_INTERNAL_PRIORITY_MAX())
                                    {
                                        for(auth = 0; auth < RTK_ENABLE_END; auth++)
                                        {
                                            osal_memset(&l2AddrW, 0x00, sizeof(rtk_l2_ucastAddr_t));
                                            l2AddrW.vid             = vid;
                                            l2AddrW.mac.octet[5]    = mac;
                                            l2AddrW.port            = port;
                                            l2AddrW.flags           = flags | RTK_L2_UCAST_FLAG_STATIC;
                                            l2AddrW.age             = 1;
                                        #ifdef CONFIG_SDK_RTL9601B 
                                            l2AddrW.ctag_vid        = fid;
                                        #elif defined(CONFIG_SDK_RTL9602C)
                                            l2AddrW.priority        = priority;
                                            l2AddrW.auth            = auth;
                                            l2AddrW.fid             = fid;
                                            l2AddrW.ext_port        = ext_port;
                                        #else
                                            l2AddrW.priority        = priority;
                                            l2AddrW.auth            = auth;
                                            l2AddrW.fid             = fid;
                                            l2AddrW.efid            = efid;
                                            l2AddrW.ext_port        = ext_port;
                                        #endif    
                                            /* Create */
                                            if((retVal = rtk_l2_addr_add(&l2AddrW)) != RT_ERR_OK)
                                            {
                                                osal_printf("\n %s %d 0x%X\n",__FUNCTION__,__LINE__, retVal);
                                                return RT_ERR_FAILED;
                                            }

                                            /* Get */
                                            osal_memset(&l2AddrR, 0x00, sizeof(rtk_l2_ucastAddr_t));
                                            l2AddrR.vid             = vid;
                                            l2AddrR.mac.octet[5]    = mac;
                                        #ifdef CONFIG_SDK_RTL9601B
                                        #elif defined(CONFIG_SDK_RTL9602C)
                                            l2AddrR.fid             = fid;
                                        #else            
                                            l2AddrR.fid             = fid;
                                            l2AddrR.efid            = efid;
                                        #endif
                                            l2AddrR.flags           = flags | RTK_L2_UCAST_FLAG_STATIC;
                                            if((retVal = rtk_l2_addr_get(&l2AddrR)) != RT_ERR_OK)
                                            {
                                                osal_printf("\n %s %d 0x%X\n",__FUNCTION__,__LINE__, retVal);
                                                return RT_ERR_FAILED;
                                            }

                                            if(flags & RTK_L2_UCAST_FLAG_IVL)
                                                l2AddrW.fid = l2AddrR.fid;
                                            else
                                                l2AddrW.vid = l2AddrR.vid;

                                            l2AddrW.index = l2AddrR.index;

                                            if(osal_memcmp(&l2AddrW, &l2AddrR, sizeof(rtk_l2_ucastAddr_t)) != 0)
                                            {
                                                osal_printf("\n l2AddrW.vid: %d\n",l2AddrW.vid);
                                                osal_printf("\n l2AddrW.fid: %d\n",l2AddrW.fid);
                                                osal_printf("\n l2AddrW.port: %d\n",l2AddrW.port);
                                                osal_printf("\n l2AddrW.flags: %d\n",l2AddrW.flags);
                                                osal_printf("\n l2AddrW.index: %d\n",l2AddrW.index);
                                                
                                                osal_printf("\n l2AddrR.vid: %d\n",l2AddrR.vid);
                                                osal_printf("\n l2AddrR.fid: %d\n",l2AddrR.fid);
                                                osal_printf("\n l2AddrR.port: %d\n",l2AddrR.port);
                                                osal_printf("\n l2AddrR.flags: %d\n",l2AddrR.flags);
                                                osal_printf("\n l2AddrR.index: %d\n",l2AddrR.index);
                                                
                                                
                                                osal_printf("\n %s %d 0x%X\n",__FUNCTION__,__LINE__, retVal);
                                                return RT_ERR_FAILED;
                                            }

                                            /* Get Next */
                                            index = 0;
                                            if((retVal = rtk_l2_nextValidAddr_get(&index, &l2AddrR)) != RT_ERR_OK)
                                            {
                                                osal_printf("\n %s %d 0x%X\n",__FUNCTION__,__LINE__, retVal);
                                                return RT_ERR_FAILED;
                                            }

                                            if(osal_memcmp(&l2AddrW, &l2AddrR, sizeof(rtk_l2_ucastAddr_t)) != 0)
                                            {
                                                osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                                return RT_ERR_FAILED;
                                            }

                                            /* Get Next on port*/
                                            index = 0;
                                            if((retVal = rtk_l2_nextValidAddrOnPort_get(port, &index, &l2AddrR)) != RT_ERR_OK)
                                            {
                                                osal_printf("\n %s %d 0x%X\n",__FUNCTION__,__LINE__, retVal);
                                                return RT_ERR_FAILED;
                                            }

                                            if(osal_memcmp(&l2AddrW, &l2AddrR, sizeof(rtk_l2_ucastAddr_t)) != 0)
                                            {
                                                osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                                return RT_ERR_FAILED;
                                            }

                                            /* Delete */
                                            if((retVal = rtk_l2_addr_del(&l2AddrR)) != RT_ERR_OK)
                                            {
                                                osal_printf("\n %s %d 0x%X\n",__FUNCTION__,__LINE__, retVal);
                                                return RT_ERR_FAILED;
                                            }

                                            /* Confirm the entry is deleted */
                                            osal_memset(&l2AddrR, 0x00, sizeof(rtk_l2_ucastAddr_t));
                                            l2AddrR.vid             = vid;
                                            l2AddrR.mac.octet[5]    = mac;
                                            l2AddrR.fid             = fid;
                                        #if defined(CONFIG_SDK_RTL9602C)
                                        #else
                                            l2AddrR.efid            = efid;
                                        #endif
                                            l2AddrR.flags           = flags | RTK_L2_UCAST_FLAG_STATIC;
                                            if(rtk_l2_addr_get(&l2AddrR) == RT_ERR_OK)
                                            {
                                                osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                                return RT_ERR_FAILED;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }


    return RT_ERR_OK;
}


int32 dal_l2_mcastAddr_test(uint32 caseNo)
{
    uint16          vid;
    uint32          mac;
    uint32          fid;
    rtk_portmask_t  portmask;
    rtk_portmask_t  ext_portmask;
    uint32          flags;
    uint32          priority;

    int32           index;

    rtk_l2_mcastAddr_t  l2AddrW;
    rtk_l2_mcastAddr_t  l2AddrR;

    /*error input check*/
    /*out of range*/
    osal_memset(&l2AddrW, 0x00, sizeof(rtk_l2_mcastAddr_t));
    l2AddrW.mac.octet[0] = 0x01;
    l2AddrW.vid = RTK_VLAN_ID_MAX + 1;
    if(rtk_l2_mcastAddr_add(&l2AddrW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    osal_memset(&l2AddrW, 0x00, sizeof(rtk_l2_mcastAddr_t));
    l2AddrW.mac.octet[0] = 0x00;
    if(rtk_l2_mcastAddr_add(&l2AddrW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    osal_memset(&l2AddrW, 0x00, sizeof(rtk_l2_mcastAddr_t));
    l2AddrW.mac.octet[0] = 0x01;
    l2AddrW.fid = HAL_VLAN_FID_MAX() + 1;
    if(rtk_l2_mcastAddr_add(&l2AddrW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    osal_memset(&l2AddrW, 0x00, sizeof(rtk_l2_mcastAddr_t));
    l2AddrW.mac.octet[0] = 0x01;
    l2AddrW.portmask.bits[0] = 0xFF;
    if(rtk_l2_mcastAddr_add(&l2AddrW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    osal_memset(&l2AddrW, 0x00, sizeof(rtk_l2_mcastAddr_t));
    l2AddrW.mac.octet[0] = 0x01;
    l2AddrW.ext_portmask.bits[0] = 0xFF;
    if(rtk_l2_mcastAddr_add(&l2AddrW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    osal_memset(&l2AddrW, 0x00, sizeof(rtk_l2_mcastAddr_t));
    l2AddrW.mac.octet[0] = 0x01;
    l2AddrW.flags = RTK_L2_MCAST_FLAG_ALL + 1;
    if(rtk_l2_mcastAddr_add(&l2AddrW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    osal_memset(&l2AddrW, 0x00, sizeof(rtk_l2_mcastAddr_t));
    l2AddrW.mac.octet[0] = 0x01;
    l2AddrW.priority = HAL_INTERNAL_PRIORITY_MAX() + 1;
    if(rtk_l2_mcastAddr_add(&l2AddrW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /*null pointer*/
    if(rtk_l2_mcastAddr_get(NULL) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /* Get/set */
    for(vid = 1; vid <= RTK_VLAN_ID_MAX; vid+=4094)
    {
        for(mac = 0x01; mac <= 0xFF; mac+=0xFE)
        {
            for(fid = 0; fid <= HAL_VLAN_FID_MAX(); fid+=HAL_VLAN_FID_MAX())
            {
                for(portmask.bits[0] = 0x1; HAL_IS_PORTMASK_VALID(portmask); portmask.bits[0] = portmask.bits[0] << 1)
                {
                    for(ext_portmask.bits[0] = 0x1; HAL_IS_EXTPORTMASK_VALID(ext_portmask); ext_portmask.bits[0] = ext_portmask.bits[0] << 1)
                    {
                        if(!RTK_PORTMASK_IS_PORT_SET(ext_portmask, HAL_GET_EXT_CPU_PORT()))
                        {
                            for(flags = RTK_L2_MCAST_FLAG_FWD_PRI; flags <= RTK_L2_MCAST_FLAG_ALL; flags = (flags << 1))
                            {
                                for(priority = 0; priority <= HAL_INTERNAL_PRIORITY_MAX(); priority+=HAL_INTERNAL_PRIORITY_MAX())
                                {
                                    /* Create */
                                    osal_memset(&l2AddrW, 0x00, sizeof(rtk_l2_mcastAddr_t));
                                    l2AddrW.vid = vid;
                                    l2AddrW.mac.octet[0] = 0x01;
                                    l2AddrW.mac.octet[5] = mac;
                                    l2AddrW.fid = fid;
                                    RTK_PORTMASK_ASSIGN(l2AddrW.portmask, portmask);
                                    RTK_PORTMASK_ASSIGN(l2AddrW.ext_portmask, ext_portmask);
                                    l2AddrW.flags = flags;
                                    l2AddrW.priority = priority;
                                    if(rtk_l2_mcastAddr_add(&l2AddrW) != RT_ERR_OK)
                                    {
                                        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                        return RT_ERR_FAILED;
                                    }

                                    /* Get */
                                    osal_memset(&l2AddrR, 0x00, sizeof(rtk_l2_mcastAddr_t));
                                    l2AddrR.vid = vid;
                                    l2AddrR.mac.octet[0] = 0x01;
                                    l2AddrR.mac.octet[5] = mac;
                                    l2AddrR.fid = fid;
                                    l2AddrR.flags = flags;
                                    if(rtk_l2_mcastAddr_get(&l2AddrR) != RT_ERR_OK)
                                    {
                                        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                        return RT_ERR_FAILED;
                                    }

                                    if(flags & RTK_L2_MCAST_FLAG_IVL)
                                        l2AddrW.fid = l2AddrR.fid;
                                    else
                                        l2AddrW.vid = l2AddrR.vid;

                                    l2AddrW.index = l2AddrR.index;

                                    if(osal_memcmp(&l2AddrW, &l2AddrR, sizeof(rtk_l2_mcastAddr_t)) != 0)
                                    {
                                        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                        return RT_ERR_FAILED;
                                    }

                                    /* Get Next*/
                                    index = 0;
                                    osal_memset(&l2AddrR, 0x00, sizeof(rtk_l2_mcastAddr_t));
                                    if(rtk_l2_nextValidMcastAddr_get(&index, &l2AddrR) != RT_ERR_OK)
                                    {
                                        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                        return RT_ERR_FAILED;
                                    }

                                    if(osal_memcmp(&l2AddrW, &l2AddrR, sizeof(rtk_l2_mcastAddr_t)) != 0)
                                    {
                                        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                        return RT_ERR_FAILED;
                                    }

                                    /* Delete */
                                    if(rtk_l2_mcastAddr_del(&l2AddrR) != RT_ERR_OK)
                                    {
                                        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                        return RT_ERR_FAILED;
                                    }

                                    /* Confirm the entry is deleted or not */
                                    osal_memset(&l2AddrR, 0x00, sizeof(rtk_l2_mcastAddr_t));
                                    l2AddrR.vid = vid;
                                    l2AddrR.mac.octet[0] = 0x01;
                                    l2AddrR.mac.octet[5] = mac;
                                    l2AddrR.fid = fid;
                                    l2AddrR.flags = flags;
                                    if(rtk_l2_mcastAddr_get(&l2AddrR) == RT_ERR_OK)
                                    {
                                        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                        return RT_ERR_FAILED;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }


    return RT_ERR_OK;
}

int32 dal_l2_ipMcastAddr_test(uint32 caseNo)
{
    ipaddr_t        dip;
    ipaddr_t        sip;
    uint16          vid;
    rtk_portmask_t  portmask;
    rtk_portmask_t  ext_portmask;
    uint32          l3_trans_index;
    uint32          flags;          /* Refer to RTK_L2_IPMCAST_FLAG_XXX */
    uint32          priority;

    int32           index;

    rtk_l2_ipMcastAddr_t ipAddrW;
    rtk_l2_ipMcastAddr_t ipAddrR;

    /*error input check*/
    /*out of range*/
    osal_memset(&ipAddrW, 0x00, sizeof(rtk_l2_ipMcastAddr_t));
    ipAddrW.dip = 0x00000000;
    if(rtk_l2_ipMcastAddr_add(&ipAddrW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    rtk_l2_ipmcMode_set(LOOKUP_ON_DIP_AND_VID);
    osal_memset(&ipAddrW, 0x00, sizeof(rtk_l2_ipMcastAddr_t));
    ipAddrW.dip = 0xE0000001;
    ipAddrW.vid = RTK_VLAN_ID_MAX + 1;
    if(rtk_l2_ipMcastAddr_add(&ipAddrW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }
    rtk_l2_ipmcMode_set(LOOKUP_ON_DIP_AND_SIP);

    osal_memset(&ipAddrW, 0x00, sizeof(rtk_l2_ipMcastAddr_t));
    ipAddrW.dip = 0xE0000001;
    ipAddrW.priority = HAL_INTERNAL_PRIORITY_MAX() + 1;
    if(rtk_l2_ipMcastAddr_add(&ipAddrW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /*null pointer*/
    if(rtk_l2_ipMcastAddr_get(NULL) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /* Get/set in DIP Only */
    for(dip = 0xE0000000; dip <= 0xEFFFFFFF; dip+=0x0FFFFFFF)
    {
        for(l3_trans_index = 0; l3_trans_index <= 0x0FFFFFFF; l3_trans_index += 0x0FFFFFFF)
        {
            for(portmask.bits[0] = 0x1; HAL_IS_PORTMASK_VALID(portmask); portmask.bits[0] = portmask.bits[0] << 1)
            {
                for(ext_portmask.bits[0] = 0x1; HAL_IS_EXTPORTMASK_VALID(ext_portmask); ext_portmask.bits[0] = ext_portmask.bits[0] << 1)
                {
                    if(!RTK_PORTMASK_IS_PORT_SET(ext_portmask, HAL_GET_EXT_CPU_PORT()))
                    {
                        for(flags = RTK_L2_IPMCAST_FLAG_FWD_PRI; flags <= RTK_L2_IPMCAST_FLAG_ALL; flags = (flags << 1))
                        {
                            for(priority = 0; priority <= HAL_INTERNAL_PRIORITY_MAX(); priority+=HAL_INTERNAL_PRIORITY_MAX())
                            {
                                /* Create */
                                osal_memset(&ipAddrW, 0x00, sizeof(rtk_l2_ipMcastAddr_t));
                                ipAddrW.dip = dip;
                                RTK_PORTMASK_ASSIGN(ipAddrW.portmask, portmask);
                                RTK_PORTMASK_ASSIGN(ipAddrW.ext_portmask, ext_portmask);
                                ipAddrW.l3_trans_index = l3_trans_index;
                                ipAddrW.flags = flags | RTK_L2_IPMCAST_FLAG_DIP_ONLY | RTK_L2_IPMCAST_FLAG_STATIC;
                                ipAddrW.priority = priority;
                                if(rtk_l2_ipMcastAddr_add(&ipAddrW) != RT_ERR_OK)
                                {
                                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                    return RT_ERR_FAILED;
                                }

                                /* Get */
                                osal_memset(&ipAddrR, 0x00, sizeof(rtk_l2_ipMcastAddr_t));
                                ipAddrR.dip = dip;
                                ipAddrR.flags = flags | RTK_L2_IPMCAST_FLAG_DIP_ONLY | RTK_L2_IPMCAST_FLAG_STATIC;
                                if(rtk_l2_ipMcastAddr_get(&ipAddrR) != RT_ERR_OK)
                                {

                                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                    return RT_ERR_FAILED;
                                }

                                ipAddrW.index = ipAddrR.index;
                                if(osal_memcmp(&ipAddrW, &ipAddrR, sizeof(rtk_l2_ipMcastAddr_t)) != 0)
                                {
                                    osal_printf("\n 0x%08X 0x%08X %d 0x%x 0x%x %d 0x%02X %d %d",
                                    ipAddrW.dip,
                                    ipAddrW.sip,
                                    ipAddrW.vid,
                                    ipAddrW.portmask.bits[0],
                                    ipAddrW.ext_portmask.bits[0],
                                    ipAddrW.l3_trans_index,
                                    ipAddrW.flags,
                                    ipAddrW.priority,
                                    ipAddrW.index);
                                    osal_printf("\n 0x%08X 0x%08X %d 0x%x 0x%x %d 0x%02X %d %d",
                                    ipAddrR.dip,
                                    ipAddrR.sip,
                                    ipAddrR.vid,
                                    ipAddrR.portmask.bits[0],
                                    ipAddrR.ext_portmask.bits[0],
                                    ipAddrR.l3_trans_index,
                                    ipAddrR.flags,
                                    ipAddrR.priority,
                                    ipAddrR.index);
                                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                    return RT_ERR_FAILED;
                                }

                                /* Get next */
                                osal_memset(&ipAddrR, 0x00, sizeof(rtk_l2_ipMcastAddr_t));
                                index = 0;
                                if(rtk_l2_nextValidIpMcastAddr_get(&index, &ipAddrR) != RT_ERR_OK)
                                {
                                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                    return RT_ERR_FAILED;
                                }

                                if(osal_memcmp(&ipAddrW, &ipAddrR, sizeof(rtk_l2_ipMcastAddr_t)) != 0)
                                {
                                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                    return RT_ERR_FAILED;
                                }

                                /* Delete */
                                if(rtk_l2_ipMcastAddr_del(&ipAddrR) != RT_ERR_OK)
                                {
                                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                    return RT_ERR_FAILED;
                                }

                                /* Confirm */
                                osal_memset(&ipAddrR, 0x00, sizeof(rtk_l2_ipMcastAddr_t));
                                ipAddrR.dip = dip;
                                ipAddrR.flags = flags | RTK_L2_IPMCAST_FLAG_DIP_ONLY | RTK_L2_IPMCAST_FLAG_STATIC;
                                if(rtk_l2_ipMcastAddr_get(&ipAddrR) == RT_ERR_OK)
                                {
                                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                    return RT_ERR_FAILED;
                                }
                            }
                        }
                    }
                }
            }
        }
    }


    /* Get/set in DIP + VID*/
    rtk_l2_ipmcMode_set(LOOKUP_ON_DIP_AND_VID);
    for(dip = 0xE0000000; dip <= 0xEFFFFFFF; dip+=0x0FFFFFFF)
    {
        for(vid = 1; vid <= RTK_VLAN_ID_MAX; vid += 4093)
        {
            for(portmask.bits[0] = 0x1; HAL_IS_PORTMASK_VALID(portmask); portmask.bits[0] = portmask.bits[0] << 1)
            {
                for(ext_portmask.bits[0] = 0x1; HAL_IS_EXTPORTMASK_VALID(ext_portmask); portmask.bits[0] = ext_portmask.bits[0] << 1)
                {
                    if(!RTK_PORTMASK_IS_PORT_SET(ext_portmask, HAL_GET_EXT_CPU_PORT()))
                    {
                        for(flags = 0; flags <= RTK_L2_IPMCAST_FLAG_FWD_PRI; flags++) /* Only one flag */
                        {
                            for(priority = 0; priority <= HAL_INTERNAL_PRIORITY_MAX(); priority+=HAL_INTERNAL_PRIORITY_MAX())
                            {
                                /* Create */
                                osal_memset(&ipAddrW, 0x00, sizeof(rtk_l2_ipMcastAddr_t));
                                ipAddrW.dip = dip;
                                ipAddrW.vid = vid;
                                RTK_PORTMASK_ASSIGN(ipAddrW.portmask, portmask);
                                RTK_PORTMASK_ASSIGN(ipAddrW.ext_portmask, ext_portmask);
                                ipAddrW.flags = flags | RTK_L2_IPMCAST_FLAG_STATIC;
                                ipAddrW.priority = priority;
                                if(rtk_l2_ipMcastAddr_add(&ipAddrW) != RT_ERR_OK)
                                {
                                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                    return RT_ERR_FAILED;
                                }

                                /* Get */
                                osal_memset(&ipAddrR, 0x00, sizeof(rtk_l2_ipMcastAddr_t));
                                ipAddrR.dip = dip;
                                ipAddrR.vid = vid;
                                ipAddrR.flags = flags | RTK_L2_IPMCAST_FLAG_STATIC;
                                if(rtk_l2_ipMcastAddr_get(&ipAddrR) != RT_ERR_OK)
                                {
                                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                    return RT_ERR_FAILED;
                                }

                                ipAddrW.index = ipAddrR.index;
                                if(osal_memcmp(&ipAddrW, &ipAddrR, sizeof(rtk_l2_ipMcastAddr_t)) != 0)
                                {
                                    osal_printf("\n 0x%08X 0x%08X %d 0x%x 0x%x %d 0x%02X %d %d",
                                    ipAddrW.dip,
                                    ipAddrW.sip,
                                    ipAddrW.vid,
                                    ipAddrW.portmask.bits[0],
                                    ipAddrW.ext_portmask.bits[0],
                                    ipAddrW.l3_trans_index,
                                    ipAddrW.flags,
                                    ipAddrW.priority,
                                    ipAddrW.index);
                                    osal_printf("\n 0x%08X 0x%08X %d 0x%x 0x%x %d 0x%02X %d %d",
                                    ipAddrR.dip,
                                    ipAddrR.sip,
                                    ipAddrR.vid,
                                    ipAddrR.portmask.bits[0],
                                    ipAddrR.ext_portmask.bits[0],
                                    ipAddrR.l3_trans_index,
                                    ipAddrR.flags,
                                    ipAddrR.priority,
                                    ipAddrR.index);
                                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                    return RT_ERR_FAILED;
                                }

                                /* Get next */
                                osal_memset(&ipAddrR, 0x00, sizeof(rtk_l2_ipMcastAddr_t));
                                index = 0;
                                if(rtk_l2_nextValidIpMcastAddr_get(&index, &ipAddrR) != RT_ERR_OK)
                                {
                                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                    return RT_ERR_FAILED;
                                }

                                if(osal_memcmp(&ipAddrW, &ipAddrR, sizeof(rtk_l2_ipMcastAddr_t)) != 0)
                                {
                                    osal_printf("\n 0x%08X 0x%08X %d 0x%x 0x%x %d 0x%02X %d %d",
                                    ipAddrW.dip,
                                    ipAddrW.sip,
                                    ipAddrW.vid,
                                    ipAddrW.portmask.bits[0],
                                    ipAddrW.ext_portmask.bits[0],
                                    ipAddrW.l3_trans_index,
                                    ipAddrW.flags,
                                    ipAddrW.priority,
                                    ipAddrW.index);
                                    osal_printf("\n 0x%08X 0x%08X %d 0x%x 0x%x %d 0x%02X %d %d",
                                    ipAddrR.dip,
                                    ipAddrR.sip,
                                    ipAddrR.vid,
                                    ipAddrR.portmask.bits[0],
                                    ipAddrR.ext_portmask.bits[0],
                                    ipAddrR.l3_trans_index,
                                    ipAddrR.flags,
                                    ipAddrR.priority,
                                    ipAddrR.index);
                                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                    return RT_ERR_FAILED;
                                }

                                /* Delete */
                                if(rtk_l2_ipMcastAddr_del(&ipAddrR) != RT_ERR_OK)
                                {
                                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                    return RT_ERR_FAILED;
                                }

                                /* Confirm */
                                osal_memset(&ipAddrR, 0x00, sizeof(rtk_l2_ipMcastAddr_t));
                                ipAddrR.dip = dip;
                                ipAddrR.vid = vid;
                                ipAddrR.flags = flags | RTK_L2_IPMCAST_FLAG_STATIC;
                                if(rtk_l2_ipMcastAddr_get(&ipAddrR) == RT_ERR_OK)
                                {
                                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                    return RT_ERR_FAILED;
                                }
                            }
                        }
                    }
                }
            }
        }
    }


    /* Get/set in DIP + SIP*/
    rtk_l2_ipmcMode_set(LOOKUP_ON_DIP_AND_SIP);
     for(dip = 0xE0000000; dip <= 0xEFFFFFFF; dip+=0x0FFFFFFF)
    {
        for(sip = 0x00000001; sip <= 0xA0000001; sip += 0xA0000000)
        {
            for(portmask.bits[0] = 0x1; HAL_IS_PORTMASK_VALID(portmask); portmask.bits[0] = portmask.bits[0] << 1)
            {
                for(ext_portmask.bits[0] = 0x1; HAL_IS_EXTPORTMASK_VALID(ext_portmask); portmask.bits[0] = ext_portmask.bits[0] << 1)
                {
                    if(!RTK_PORTMASK_IS_PORT_SET(ext_portmask, HAL_GET_EXT_CPU_PORT()))
                    {
                        for(flags = 0; flags <= RTK_L2_IPMCAST_FLAG_FWD_PRI; flags++) /* Only one flag */
                        {
                            for(priority = 0; priority <= HAL_INTERNAL_PRIORITY_MAX(); priority+=HAL_INTERNAL_PRIORITY_MAX())
                            {
                                /* Create */
                                osal_memset(&ipAddrW, 0x00, sizeof(rtk_l2_ipMcastAddr_t));
                                ipAddrW.dip = dip;
                                ipAddrW.sip = sip;
                                RTK_PORTMASK_ASSIGN(ipAddrW.portmask, portmask);
                                RTK_PORTMASK_ASSIGN(ipAddrW.ext_portmask, ext_portmask);
                                ipAddrW.flags = flags | RTK_L2_IPMCAST_FLAG_STATIC;
                                ipAddrW.priority = priority;
                                if(rtk_l2_ipMcastAddr_add(&ipAddrW) != RT_ERR_OK)
                                {
                                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                    return RT_ERR_FAILED;
                                }

                                /* Get */
                                osal_memset(&ipAddrR, 0x00, sizeof(rtk_l2_ipMcastAddr_t));
                                ipAddrR.dip = dip;
                                ipAddrR.sip = sip;
                                ipAddrR.flags = flags | RTK_L2_IPMCAST_FLAG_STATIC;
                                if(rtk_l2_ipMcastAddr_get(&ipAddrR) != RT_ERR_OK)
                                {
                                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                    return RT_ERR_FAILED;
                                }

                                ipAddrW.index = ipAddrR.index;
                                if(osal_memcmp(&ipAddrW, &ipAddrR, sizeof(rtk_l2_ipMcastAddr_t)) != 0)
                                {
                                    osal_printf("\n 0x%08X 0x%08X %d 0x%x 0x%x %d 0x%02X %d %d",
                                    ipAddrW.dip,
                                    ipAddrW.sip,
                                    ipAddrW.vid,
                                    ipAddrW.portmask.bits[0],
                                    ipAddrW.ext_portmask.bits[0],
                                    ipAddrW.l3_trans_index,
                                    ipAddrW.flags,
                                    ipAddrW.priority,
                                    ipAddrW.index);
                                    osal_printf("\n 0x%08X 0x%08X %d 0x%x 0x%x %d 0x%02X %d %d",
                                    ipAddrR.dip,
                                    ipAddrR.sip,
                                    ipAddrR.vid,
                                    ipAddrR.portmask.bits[0],
                                    ipAddrR.ext_portmask.bits[0],
                                    ipAddrR.l3_trans_index,
                                    ipAddrR.flags,
                                    ipAddrR.priority,
                                    ipAddrR.index);
                                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                    return RT_ERR_FAILED;
                                }

                                /* Get next */
                                osal_memset(&ipAddrR, 0x00, sizeof(rtk_l2_ipMcastAddr_t));
                                index = 0;
                                if(rtk_l2_nextValidIpMcastAddr_get(&index, &ipAddrR) != RT_ERR_OK)
                                {
                                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                    return RT_ERR_FAILED;
                                }

                                if(osal_memcmp(&ipAddrW, &ipAddrR, sizeof(rtk_l2_ipMcastAddr_t)) != 0)
                                {
                                    osal_printf("\n 0x%08X 0x%08X %d 0x%x 0x%x %d 0x%02X %d %d",
                                    ipAddrW.dip,
                                    ipAddrW.sip,
                                    ipAddrW.vid,
                                    ipAddrW.portmask.bits[0],
                                    ipAddrW.ext_portmask.bits[0],
                                    ipAddrW.l3_trans_index,
                                    ipAddrW.flags,
                                    ipAddrW.priority,
                                    ipAddrW.index);
                                    osal_printf("\n 0x%08X 0x%08X %d 0x%x 0x%x %d 0x%02X %d %d",
                                    ipAddrR.dip,
                                    ipAddrR.sip,
                                    ipAddrR.vid,
                                    ipAddrR.portmask.bits[0],
                                    ipAddrR.ext_portmask.bits[0],
                                    ipAddrR.l3_trans_index,
                                    ipAddrR.flags,
                                    ipAddrR.priority,
                                    ipAddrR.index);
                                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                    return RT_ERR_FAILED;
                                }

                                /* Delete */
                                if(rtk_l2_ipMcastAddr_del(&ipAddrR) != RT_ERR_OK)
                                {
                                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                    return RT_ERR_FAILED;
                                }

                                /* Confirm */
                                osal_memset(&ipAddrR, 0x00, sizeof(rtk_l2_ipMcastAddr_t));
                                ipAddrR.dip = dip;
                                ipAddrR.sip = sip;
                                ipAddrR.flags = flags | RTK_L2_IPMCAST_FLAG_STATIC;
                                if(rtk_l2_ipMcastAddr_get(&ipAddrR) == RT_ERR_OK)
                                {
                                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                                    return RT_ERR_FAILED;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return RT_ERR_OK;
}

int32 dal_l2_ipmcGroup_test(uint32 caseNo)
{
    int32 ret;
    ipaddr_t gip;
    rtk_portmask_t portmaskW;
    rtk_portmask_t portmaskR;

    /*error input check*/
    /*out of range*/
    gip = 0x00000000;
    portmaskW.bits[0] = 0;
    if(rtk_l2_ipmcGroup_add(gip, &portmaskW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    gip = 0xE0000001;
    portmaskW.bits[0] = 0xFF;
    if(rtk_l2_ipmcGroup_add(gip, &portmaskW) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /* delete */
    if(rtk_l2_ipmcGroup_del(gip) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /*null pointer*/
    if(rtk_l2_ipmcGroup_get(gip, NULL) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /* Get/set */
    for(gip = 0xE0000001; gip <= 0xEFFFFFFF; gip+=0x0FFFFFFE)
    {
        for(portmaskW.bits[0] = 0x00; HAL_IS_PORTMASK_VALID(portmaskW); portmaskW.bits[0]++)
        {
            if( (ret = rtk_l2_ipmcGroup_add(gip, &portmaskW)) != RT_ERR_OK)
            {
                osal_printf("\n %s %d 0x%X\n",__FUNCTION__,__LINE__, ret);
                return RT_ERR_FAILED;
            }

            if(rtk_l2_ipmcGroup_get(gip, &portmaskR) != RT_ERR_OK)
            {
                osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                return RT_ERR_FAILED;
            }

            if(RTK_PORTMASK_COMPARE(portmaskW, portmaskR) != 0)
            {
                osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                return RT_ERR_FAILED;
            }

            if(rtk_l2_ipmcGroup_del(gip) != RT_ERR_OK)
            {
                osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                return RT_ERR_FAILED;
            }

            if(rtk_l2_ipmcGroup_get(gip, &portmaskR) == RT_ERR_OK)
            {
                osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                return RT_ERR_FAILED;
            }
        }
    }

    return RT_ERR_OK;
}


int32 dal_l2_portLimitLearningCnt_test(uint32 caseNo)
{
    rtk_port_t port;
    int32 ret;
    uint32 cntW;
    uint32 cntR;

    /*error input check*/
    /*out of range*/
    if( (ret = rtk_l2_portLimitLearningCnt_set(HAL_GET_MAX_PORT() + 1, HAL_L2_LEARN_LIMIT_CNT_MAX())) == RT_ERR_OK)
    {
        osal_printf("\n %s %d (0x%X)\n",__FUNCTION__,__LINE__,ret);
        return RT_ERR_FAILED;
    }

    if( (ret = rtk_l2_portLimitLearningCnt_set(0, HAL_L2_LEARN_LIMIT_CNT_MAX() + 1)) == RT_ERR_OK)
    {
        osal_printf("\n %s %d (0x%X)\n",__FUNCTION__,__LINE__,ret);
        return RT_ERR_FAILED;
    }

    /*null pointer*/
    if( (ret = rtk_l2_portLimitLearningCnt_get(0, NULL)) == RT_ERR_OK)
    {
        osal_printf("\n %s %d (0x%X)\n",__FUNCTION__,__LINE__,ret);
        return RT_ERR_FAILED;
    }

    if( (ret = rtk_l2_portLearningCnt_get(0, NULL)) == RT_ERR_OK)
    {
        osal_printf("\n %s %d (0x%X)\n",__FUNCTION__,__LINE__,ret);
        return RT_ERR_FAILED;
    }

    /* Get/set */
    HAL_SCAN_ALL_PORT(port)
    {
        for(cntW = 0; cntW <= HAL_L2_LEARN_LIMIT_CNT_MAX(); cntW+=123)
        {
            if( (ret = rtk_l2_portLimitLearningCnt_set(port, cntW)) != RT_ERR_OK)
            {
                osal_printf("\n %s %d (0x%X)\n",__FUNCTION__,__LINE__,ret);
                return RT_ERR_FAILED;
            }

            if( (ret = rtk_l2_portLimitLearningCnt_get(port, &cntR)) != RT_ERR_OK)
            {
                osal_printf("\n %s %d (0x%X)\n",__FUNCTION__,__LINE__,ret);
                return RT_ERR_FAILED;
            }

            if(cntW != cntR)
            {
                osal_printf("\n %s %d (0x%X)\n",__FUNCTION__,__LINE__,ret);
                return RT_ERR_FAILED;
            }

            if( (ret = rtk_l2_portLearningCnt_get(port, &cntR)) != RT_ERR_OK)
            {
                osal_printf("\n %s %d (0x%X)\n",__FUNCTION__,__LINE__,ret);
                return RT_ERR_FAILED;
            }
        }
    }

    return RT_ERR_OK;
}

int32 dal_l2_portLimitLearningCntAction_test(uint32 caseNo)
{
    rtk_port_t port;
    rtk_l2_limitLearnCntAction_t actW;
    rtk_l2_limitLearnCntAction_t actR;

    /*error input check*/
    /*out of range*/
    if(rtk_l2_portLimitLearningCntAction_set(HAL_GET_MAX_PORT() + 1, LIMIT_LEARN_CNT_ACTION_FORWARD) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    if(rtk_l2_portLimitLearningCntAction_set(0, LIMIT_LEARN_CNT_ACTION_END) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /*null pointer*/
    if(rtk_l2_portLimitLearningCntAction_get(0, NULL) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /* Get/set */
    HAL_SCAN_ALL_PORT(port)
    {
        for(actW = LIMIT_LEARN_CNT_ACTION_DROP; actW < LIMIT_LEARN_CNT_ACTION_END; actW++)
        {
            if( (actW == LIMIT_LEARN_CNT_ACTION_DROP) ||
                (actW == LIMIT_LEARN_CNT_ACTION_FORWARD) ||
                (actW == LIMIT_LEARN_CNT_ACTION_TO_CPU) ||
                (actW == LIMIT_LEARN_CNT_ACTION_COPY_CPU) )
            {
                if(rtk_l2_portLimitLearningCntAction_set(port, actW) != RT_ERR_OK)
                {
                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                    return RT_ERR_FAILED;
                }

                if(rtk_l2_portLimitLearningCntAction_get(port, &actR) != RT_ERR_OK)
                {
                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                    return RT_ERR_FAILED;
                }

                if(actW != actW)
                {
                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                    return RT_ERR_FAILED;
                }
            }
        }
    }

    return RT_ERR_OK;
}

int32 dal_l2_portLimitLearningOverStatus_test(uint32 caseNo)
{
    rtk_port_t port;
    uint32 status;

    /*error input check*/
    /*out of range*/
    if(rtk_l2_portLimitLearningOverStatus_get(HAL_GET_MAX_PORT() + 1, &status) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    if(rtk_l2_portLimitLearningOverStatus_clear(HAL_GET_MAX_PORT() + 1) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /*null pointer*/
    if(rtk_l2_portLimitLearningOverStatus_get(0, NULL) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /* Get/set */
    HAL_SCAN_ALL_PORT(port)
    {
        if(rtk_l2_portLimitLearningOverStatus_get(port, &status) != RT_ERR_OK)
        {
            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
            return RT_ERR_FAILED;
        }

        if(rtk_l2_portLimitLearningOverStatus_clear(port) != RT_ERR_OK)
        {
            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
            return RT_ERR_FAILED;
        }
    }


    return RT_ERR_OK;
}

int32 dal_l2_limitLearningOverStatus_test(uint32 caseNo)
{
    uint32 status;

    if(rtk_l2_limitLearningOverStatus_get(&status) == RT_ERR_DRIVER_NOT_FOUND)
    {
        return RT_ERR_OK;
    }

    /*error input check*/
    /*out of range*/

    /*null pointer*/
    if(rtk_l2_limitLearningOverStatus_get(NULL) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /* Get/set */
    if(rtk_l2_limitLearningOverStatus_get(&status) != RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    if(rtk_l2_limitLearningOverStatus_clear() != RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
}

int32 dal_l2_portLookupMissAction_test(uint32 caseNo)
{
    rtk_port_t port;
    rtk_l2_lookupMissType_t type;
    rtk_action_t actW;
    rtk_action_t actR;

    /*error input check*/
    /*out of range*/
    if(rtk_l2_portLookupMissAction_set(HAL_GET_MAX_PORT() + 1, DLF_TYPE_MCAST, ACTION_FORWARD) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    if(rtk_l2_portLookupMissAction_set(0, DLF_TYPE_END, ACTION_FORWARD) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    if(rtk_l2_portLookupMissAction_set(0, DLF_TYPE_MCAST, ACTION_END) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /*null pointer*/
    if(rtk_l2_portLookupMissAction_get(0, DLF_TYPE_MCAST, NULL) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /* Get/set */
    HAL_SCAN_ALL_PORT(port)
    {
        for(type = DLF_TYPE_IPMC; type < DLF_TYPE_END; type++)
        {
            if(type != DLF_TYPE_BCAST)
            {
                for(actW = ACTION_FORWARD; actW < ACTION_END; actW++)
                {
                    if( (actW == ACTION_FORWARD) || (actW == ACTION_DROP) || (actW == ACTION_TRAP2CPU) || ((actW == ACTION_DROP_EXCLUDE_RMA) && (type == DLF_TYPE_MCAST)) )
                    {
                        if(rtk_l2_portLookupMissAction_set(port, type, actW) != RT_ERR_OK)
                        {
                            osal_printf("\n %d %d", type, actW);
                            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                            return RT_ERR_FAILED;
                        }

                        if(rtk_l2_portLookupMissAction_get(port, type, &actR) != RT_ERR_OK)
                        {
                            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                            return RT_ERR_FAILED;
                        }

                        if(actW != actR)
                        {
                            osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                            return RT_ERR_FAILED;
                        }
                    }
                }
            }
        }
    }

    return RT_ERR_OK;
}

int32 dal_l2_portIpmcAction_test(uint32 caseNo)
{
    rtk_port_t port;
    rtk_action_t actW;
    rtk_action_t actR;

    /*error input check*/
    /*out of range*/
    if(rtk_l2_portIpmcAction_set(HAL_GET_MAX_PORT() + 1, ACTION_FORWARD) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    if(rtk_l2_portIpmcAction_set(HAL_GET_MAX_PORT(), ACTION_END) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    if(rtk_l2_portIpmcAction_get(HAL_GET_MAX_PORT() + 1, &actR) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    if(rtk_l2_portIpmcAction_get(0, NULL) == RT_ERR_OK)
    {
        osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
        return RT_ERR_FAILED;
    }

    /* Get/set */
    HAL_SCAN_ALL_PORT(port)
    {
        for(actW = ACTION_FORWARD; actW < ACTION_END; actW++)
        {
            if( (actW == ACTION_FORWARD) || (actW == ACTION_DROP) )
            {
                if(rtk_l2_portIpmcAction_set(port, actW) != RT_ERR_OK)
                {
                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                    return RT_ERR_FAILED;
                }

                if(rtk_l2_portIpmcAction_get(port, &actR) != RT_ERR_OK)
                {
                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                    return RT_ERR_FAILED;
                }

                if(actW != actR)
                {
                    osal_printf("\n %s %d\n",__FUNCTION__,__LINE__);
                    return RT_ERR_FAILED;
                }
            }
        }
    }

    return RT_ERR_OK;
}

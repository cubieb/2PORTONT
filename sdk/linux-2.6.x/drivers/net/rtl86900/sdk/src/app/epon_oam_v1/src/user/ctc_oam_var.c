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
 * $Date: 2013-05-03 17:35:27 +0800 (週五, 03 五月 2013) $
 *
 * Purpose : Define the standard OAM callback and CTC extended OAM callback
 *
 * Feature : Provide standard and CTC related callback functions
 *
 */

/*
 * Include Files
 */
/* Standard include */
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <linux/reboot.h>
/* EPON OAM include */
#include "epon_oam_config.h"
#include "epon_oam_err.h"
#include "epon_oam_db.h"
#include "epon_oam_dbg.h"
#include "epon_oam_rx.h"
/* User specific include */
#include "ctc_oam.h"
#include "ctc_oam_var.h"

#include <rtk/port.h>
#include <rtk/vlan.h>
#include <rtk/epon.h>
#include <rtk/rldp.h>
#include <hal/common/halctrl.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
/* MODIFY ME - use actual database instead */
ctc_dbaThreshold_t dbaThresholdDb = {
    .numQSet = 1,
    .reportMap[0] = 0xff,
    .queueSet[0].queueThreshold[0] = 0xffff,
    .queueSet[0].queueThreshold[1] = 0xffff,
    .queueSet[0].queueThreshold[2] = 0xffff,
    .queueSet[0].queueThreshold[3] = 0xffff,
    .queueSet[0].queueThreshold[4] = 0xffff,
    .queueSet[0].queueThreshold[5] = 0xffff,
    .queueSet[0].queueThreshold[6] = 0xffff,
    .queueSet[0].queueThreshold[7] = 0xffff,
};

/*
 * Macro Definition
 */

/*
 * Function Declaration
 */
/* Dummy variable set function, always return OK.
 * This function is for debug usage only
 */
int ctc_oam_varCb_dummy_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* caller prepare all the resource needed, 
                                          * handler won't need to allocate anything
                                          */
{
    /* Pretend set operation complete */
    pVarContainer->varWidth = 0x80;

    return EPON_OAM_ERR_OK;
}

/* 0x07/0x0025 - aPhyAdminState */
int ctc_oam_varCb_aPhyAdminState_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler allocate data field in container, 
                                          * caller will release it
                                          */
{
    int ret;
    unsigned int getValue;
    rtk_enable_t enable;

    if(NULL == pVarInstant)
    {
        return EPON_OAM_ERR_NOT_FOUND;
    }

    pVarContainer->varDesc = varDesc;
    pVarContainer->varWidth = CTC_OAM_VAR_APHYADMINSTATE_LEN; /* Fix value from CTC standard */
    pVarContainer->pVarData = (unsigned char *) malloc(pVarContainer->varWidth * sizeof(unsigned char));
    if(NULL == pVarContainer->pVarData)
    {
        return EPON_OAM_ERR_MEM;
    }

    if(!CTC_INSTANT_IS_ALLPORTS(pVarInstant) && CTC_INSTANT_IS_ETHERNET(pVarInstant))
    {
        ret = rtk_port_adminEnable_get(pVarInstant->parse.uniPort.portNo - 1, &enable);
        if(RT_ERR_OK == ret)
        {
            if(DISABLED == enable)
            {
                getValue = CTC_OAM_VAR_APHYADMINSTATE_DISABLE;
            }
            else
            {
                getValue = CTC_OAM_VAR_APHYADMINSTATE_ENABLE;
            }
            CTC_BUF_ENCODE32(pVarContainer->pVarData, &getValue);
        }
        else
        {
            return EPON_OAM_ERR_UNKNOWN;
        }
    }

    return EPON_OAM_ERR_OK;
}

/* 0x07/0x004F - aAutoNegAdminState */
int ctc_oam_varCb_aAutoNegAdminState_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler allocate data field in container, 
                                          * caller will release it
                                          */
{
    int ret;
    unsigned int getValue;
    rtk_enable_t enable;

    if(NULL == pVarInstant)
    {
        return EPON_OAM_ERR_NOT_FOUND;
    }

    pVarContainer->varDesc = varDesc;
    pVarContainer->varWidth = CTC_OAM_VAR_AAUTONEGADMINSTATE_LEN; /* Fix value from CTC standard */
    pVarContainer->pVarData = (unsigned char *) malloc(pVarContainer->varWidth * sizeof(unsigned char));
    if(NULL == pVarContainer->pVarData)
    {
        return EPON_OAM_ERR_MEM;
    }

    if(!CTC_INSTANT_IS_ALLPORTS(pVarInstant) && CTC_INSTANT_IS_ETHERNET(pVarInstant))
    {
        ret = rtk_port_phyAutoNegoEnable_get(pVarInstant->parse.uniPort.portNo - 1, &enable);
        if(RT_ERR_OK == ret)
        {
            if(DISABLED == enable)
            {
                getValue = CTC_OAM_VAR_AAUTONEGADMINSTATE_DISABLE;
            }
            else
            {
                getValue = CTC_OAM_VAR_AAUTONEGADMINSTATE_ENABLE;
            }
            CTC_BUF_ENCODE32(pVarContainer->pVarData, &getValue);
        }
        else
        {
            return EPON_OAM_ERR_UNKNOWN;
        }
    }

    return EPON_OAM_ERR_OK;
}

/* 0x07/0x0052 - aAutoNegLocalTechnologyAbility */
int ctc_oam_varCb_aAutoNegLocalTechnologyAbility_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler allocate data field in container, 
                                          * caller will release it
                                          */
{
    pVarContainer->varDesc = varDesc;
    pVarContainer->varWidth = 4 + 12;
    pVarContainer->pVarData = (unsigned char *) malloc(pVarContainer->varWidth * sizeof(unsigned char));
    if(NULL == pVarContainer->pVarData)
    {
        return EPON_OAM_ERR_MEM;
    }

    /* Value List extract from 802.3 30A
     * global (0),          --reserved for future use.
     * other (1),           --undefined
     * unknown (2),         --initializing, true ability not yet known.
     * 10BASE-T (14),       --10BASE-T as defined in Clause 14
     * 10BASE-TFD (142),    --Full duplex 10BASE-T as defined in Clauses
                              14 and 31
     * 100BASE-T4 (23),     --100BASE-T4 as defined in Clause 23
     * 100BASE-TX (25),     --100BASE-TX as defined in Clause 25
     * 100BASE-TXFD (252),  --Full duplex 100BASE-TX as defined in Clauses
                              25 and 31
     * 10GBASE-KX4 (483),   --10GBASE-KX4 PHY as defined in Clause 71
     * 10GBASE-KR (495),    --10GBASE-KR PHY as defined in Clause 72
     * 10GBASE-T (55),      --10GBASE-T PHY as defined in Clause 55
     * FDX PAUSE (312),     --PAUSE operation for full duplex links as 
                              defined in Annex 31B
     * FDX APAUSE (313),    --Asymmetric PAUSE operation for full duplex
                              links as defined in Clause 37 and Annex 31B
     * FDX SPAUSE (314),    --Symmetric PAUSE operation for full duplex
                              links as defined in Clause 37 and Annex 31B
     * FDX BPAUSE (315),    --Asymmetric and Symmetric PAUSE operation for
                              full duplex links as defined in Clause 37
                              and Annex 31B
     * 100BASE-T2 (32),     --100BASE-T2 as defined in Clause 32
     * 100BASE-T2FD (322),  --Full duplex 100BASE-T2 as defined in Clauses
                              31 and 32
     * 1000BASE-X (36),     --1000BASE-X as defined in Clause 36
     * 1000BASE-XFD (362),  --Full duplex 1000BASE-X as defined in Clause 36
     * 1000BASE-KX (393),   --1000BASE-KX PHY as defined in Clause 70
     * 1000BASE-T (40),     --1000BASE-T UTP PHY as defined in Clause 40
     * 1000BASE-TFD (402),  --Full duplex 1000BASE-T UTP PHY to be defined
                              in Clause 40
     * Rem Fault1 (37),     --Remote fault bit 1 (RF1) as specified in Clause 37
     * Rem Fault2 (372),    --Remote fault bit 2 (RF1) as specified in Clause 37
     * isoethernet (8029)   --802.9 ISLAN-16T
     */

    /* For demo system, pretend we support 10GBASE-T/100BASE-TX/1000BASE-T */
    /* Number of the Enumerated - 3 enumerated */
    pVarContainer->pVarData[0] = 0x00;
    pVarContainer->pVarData[1] = 0x00;
    pVarContainer->pVarData[2] = 0x00;
    pVarContainer->pVarData[3] = 0x03;

    /* 1st Enumerated - 1000BASE-T */
    pVarContainer->pVarData[4] = 0x00;
    pVarContainer->pVarData[5] = 0x00;
    pVarContainer->pVarData[6] = 0x00;
    pVarContainer->pVarData[7] = 0x28;

    /* 2nd Enumerated - 100BASE-TX */
    pVarContainer->pVarData[8] = 0x00;
    pVarContainer->pVarData[9] = 0x00;
    pVarContainer->pVarData[10] = 0x00;
    pVarContainer->pVarData[11] = 0x19;

    /* 3rd Enumerated - 10GBASE-T */
    pVarContainer->pVarData[12] = 0x00;
    pVarContainer->pVarData[13] = 0x00;
    pVarContainer->pVarData[14] = 0x00;
    pVarContainer->pVarData[15] = 0x0E;

    return EPON_OAM_ERR_OK;
}

/* 0x07/0x0053 - aAutoNegAdvertisedTechnologyAbility */
int ctc_oam_varCb_aAutoNegAdvertisedTechnologyAbility_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler allocate data field in container, 
                                          * caller will release it
                                          */
{
    pVarContainer->varDesc = varDesc;
    pVarContainer->varWidth = 4 + 12;
    pVarContainer->pVarData = (unsigned char *) malloc(pVarContainer->varWidth * sizeof(unsigned char));
    if(NULL == pVarContainer->pVarData)
    {
        return EPON_OAM_ERR_MEM;
    }

    /* Value List extract from 802.3 30A
     * global (0),          --reserved for future use.
     * other (1),           --undefined
     * unknown (2),         --initializing, true ability not yet known.
     * 10BASE-T (14),       --10BASE-T as defined in Clause 14
     * 10BASE-TFD (142),    --Full duplex 10BASE-T as defined in Clauses
                              14 and 31
     * 100BASE-T4 (23),     --100BASE-T4 as defined in Clause 23
     * 100BASE-TX (25),     --100BASE-TX as defined in Clause 25
     * 100BASE-TXFD (252),  --Full duplex 100BASE-TX as defined in Clauses
                              25 and 31
     * 10GBASE-KX4 (483),   --10GBASE-KX4 PHY as defined in Clause 71
     * 10GBASE-KR (495),    --10GBASE-KR PHY as defined in Clause 72
     * 10GBASE-T (55),      --10GBASE-T PHY as defined in Clause 55
     * FDX PAUSE (312),     --PAUSE operation for full duplex links as 
                              defined in Annex 31B
     * FDX APAUSE (313),    --Asymmetric PAUSE operation for full duplex
                              links as defined in Clause 37 and Annex 31B
     * FDX SPAUSE (314),    --Symmetric PAUSE operation for full duplex
                              links as defined in Clause 37 and Annex 31B
     * FDX BPAUSE (315),    --Asymmetric and Symmetric PAUSE operation for
                              full duplex links as defined in Clause 37
                              and Annex 31B
     * 100BASE-T2 (32),     --100BASE-T2 as defined in Clause 32
     * 100BASE-T2FD (322),  --Full duplex 100BASE-T2 as defined in Clauses
                              31 and 32
     * 1000BASE-X (36),     --1000BASE-X as defined in Clause 36
     * 1000BASE-XFD (362),  --Full duplex 1000BASE-X as defined in Clause 36
     * 1000BASE-KX (393),   --1000BASE-KX PHY as defined in Clause 70
     * 1000BASE-T (40),     --1000BASE-T UTP PHY as defined in Clause 40
     * 1000BASE-TFD (402),  --Full duplex 1000BASE-T UTP PHY to be defined
                              in Clause 40
     * Rem Fault1 (37),     --Remote fault bit 1 (RF1) as specified in Clause 37
     * Rem Fault2 (372),    --Remote fault bit 2 (RF1) as specified in Clause 37
     * isoethernet (8029)   --802.9 ISLAN-16T
     */

    /* Chip support 10GBASE-T/100BASE-TX/1000BASE-T for all UTP ports */
    /* Number of the Enumerated - 3 enumerated */
    pVarContainer->pVarData[0] = 0x00;
    pVarContainer->pVarData[1] = 0x00;
    pVarContainer->pVarData[2] = 0x00;
    pVarContainer->pVarData[3] = 0x03;

    /* 1st Enumerated - 1000BASE-T */
    pVarContainer->pVarData[4] = 0x00;
    pVarContainer->pVarData[5] = 0x00;
    pVarContainer->pVarData[6] = 0x00;
    pVarContainer->pVarData[7] = 0x28;

    /* 2nd Enumerated - 100BASE-TX */
    pVarContainer->pVarData[8] = 0x00;
    pVarContainer->pVarData[9] = 0x00;
    pVarContainer->pVarData[10] = 0x00;
    pVarContainer->pVarData[11] = 0x19;

    /* 3rd Enumerated - 10GBASE-T */
    pVarContainer->pVarData[12] = 0x00;
    pVarContainer->pVarData[13] = 0x00;
    pVarContainer->pVarData[14] = 0x00;
    pVarContainer->pVarData[15] = 0x0E;

    return EPON_OAM_ERR_OK;
}

/* 0x07/0x0139 - aFECAbility */
int ctc_oam_varCb_aFECAbility_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler allocate data field in container, 
                                          * caller will release it
                                          */
{
    unsigned int getValue;

    pVarContainer->varDesc = varDesc;
    pVarContainer->varWidth = CTC_OAM_VAR_AFECABILITY_LEN; /* Fix value from CTC standard */
    pVarContainer->pVarData = (unsigned char *) malloc(pVarContainer->varWidth * sizeof(unsigned char));
    if(NULL == pVarContainer->pVarData)
    {
        return EPON_OAM_ERR_MEM;
    }

    /* Value - always return FEC is supported */
    getValue = CTC_OAM_VAR_AFECABILITY_SUPPORTED;
    pVarContainer->pVarData[0] = ((unsigned char *)&getValue)[0];
    pVarContainer->pVarData[1] = ((unsigned char *)&getValue)[1];
    pVarContainer->pVarData[2] = ((unsigned char *)&getValue)[2];
    pVarContainer->pVarData[3] = ((unsigned char *)&getValue)[3];

    return EPON_OAM_ERR_OK;
}

/* 0x07/0x013A - aFECmode */
int ctc_oam_varCb_aFecMode_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler allocate data field in container, 
                                          * caller will release it
                                          */
{
    int ret;
    rtk_enable_t usState;
    unsigned int getValue;

    pVarContainer->varDesc = varDesc;
    pVarContainer->varWidth = CTC_OAM_VAR_AFECMODE_LEN; /* Fix value from CTC standard */
    pVarContainer->pVarData = (unsigned char *) malloc(pVarContainer->varWidth * sizeof(unsigned char));
    if(NULL == pVarContainer->pVarData)
    {
        return EPON_OAM_ERR_MEM;
    }

    ret = rtk_epon_usFecState_get(&usState);
    if(RT_ERR_OK != ret)
    {
        return EPON_OAM_ERR_UNKNOWN;
    }

 
    {
        if(ENABLED == usState)
        {
            getValue = CTC_OAM_VAR_AFECMODE_ENABLED;
        }
        else
        {
            getValue = CTC_OAM_VAR_AFECMODE_DISABLED;
        }
    }

    /* Construct return value */
    CTC_BUF_ENCODE32(pVarContainer->pVarData, &getValue);

    return EPON_OAM_ERR_OK;
}

/* 0x07/0x013A - aFECmode */
int ctc_oam_varCb_aFecMode_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* caller prepare all the resource needed, 
                                          * handler won't need to allocate anything
                                          */
{
    int ret;
    unsigned int setValue;

    if(CTC_OAM_VAR_AFECMODE_LEN != pVarContainer->varWidth)
    {
        return EPON_OAM_ERR_UNKNOWN;
    }
    CTC_BUF_PARSE32(pVarContainer->pVarData, &setValue);

    /* Configure both US and DS FEC */
    switch(setValue)
    {
    case CTC_OAM_VAR_AFECMODE_ENABLED:
        do {
            ret = rtk_epon_usFecState_set(ENABLED);
            if(RT_ERR_OK != ret)
            {
                pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
                break;
            }
            pVarContainer->varWidth = CTC_OAM_VAR_RESP_SETOK;
        } while(0);
        break;
    case CTC_OAM_VAR_AFECMODE_DISABLED:
        do {
            ret = rtk_epon_usFecState_set(DISABLED);
            if(RT_ERR_OK != ret)
            {
                pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
                break;
            }
            pVarContainer->varWidth = CTC_OAM_VAR_RESP_SETOK;
        } while(0);
        break;
    case CTC_OAM_VAR_AFECMODE_UNKNOWN:
    default:
        pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARBADPARAM;
        break;
    }

    return EPON_OAM_ERR_OK;
}

/* 0x09/0x0005 - acPhyAdminControl */
int ctc_oam_varCb_acPhyAdminControl_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* caller prepare all the resource needed, 
                                          * handler won't need to allocate anything
                                          */
{
    int ret;
    unsigned int port;
    unsigned int setValue;
    rtk_enable_t enable;

    if(CTC_OAM_VAR_ACPHYADMINCONTROL_LEN != pVarContainer->varWidth)
    {
        return EPON_OAM_ERR_UNKNOWN;
    }

    if(NULL == pVarInstant)
    {
        return EPON_OAM_ERR_NOT_FOUND;
    }

    if(CTC_INSTANT_IS_ETHERNET(pVarInstant))
    {
        CTC_BUF_PARSE32(pVarContainer->pVarData, &setValue);
        if(CTC_OAM_VAR_ACPHYADMINCONTROL_DEACTIVE == setValue)
        {
            enable = DISABLED;
        }
        else if(CTC_OAM_VAR_ACPHYADMINCONTROL_ACTIVE == setValue)
        {
            enable = ENABLED;
        }
        else
        {
            pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARBADPARAM;
            return EPON_OAM_ERR_OK;
        }

        if(CTC_INSTANT_IS_ALLPORTS(pVarInstant))
        {
            HAL_SCAN_ALL_PORT(port)
            {
                ret = rtk_port_adminEnable_set(port, enable);
                if(RT_ERR_OK != ret)
                {
                    pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
                    return EPON_OAM_ERR_OK;
                }
            }
        }
        else
        {
            ret = rtk_port_adminEnable_set(pVarInstant->parse.uniPort.portNo - 1, enable);
            if(RT_ERR_OK != ret)
            {
                pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
                return EPON_OAM_ERR_OK;
            }
        }
    }
    pVarContainer->varWidth = CTC_OAM_VAR_RESP_SETOK;

    return EPON_OAM_ERR_OK;
}

/* 0x09/0x000B - acAutoNegRestartAutoConfig */
int ctc_oam_varCb_acAutoNegRestartAutoConfig_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* caller prepare all the resource needed, 
                                          * handler won't need to allocate anything
                                          */
{
    int ret;
    unsigned int port;
    rtk_enable_t enable;

    if(CTC_OAM_VAR_ACAUTONEGRESTARTAUTOCONFIG_LEN != pVarContainer->varWidth)
    {
        return EPON_OAM_ERR_UNKNOWN;
    }

    if(NULL == pVarInstant)
    {
        return EPON_OAM_ERR_NOT_FOUND;
    }

    if(CTC_INSTANT_IS_ETHERNET(pVarInstant))
    {
        if(CTC_INSTANT_IS_ALLPORTS(pVarInstant))
        {
            HAL_SCAN_ALL_PORT(port)
            {
                ret = rtk_port_phyAutoNegoEnable_get(port, &enable);
                if(RT_ERR_OK != ret)
                {
                    pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
                    return EPON_OAM_ERR_OK;
                }
                ret = rtk_port_phyAutoNegoEnable_set(port, enable);
                if(RT_ERR_OK != ret)
                {
                    pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
                    return EPON_OAM_ERR_OK;
                }
            }
        }
        else
        {
            ret = rtk_port_phyAutoNegoEnable_get(pVarInstant->parse.uniPort.portNo - 1, &enable);
            if(RT_ERR_OK != ret)
            {
                pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
                return EPON_OAM_ERR_OK;
            }
            ret = rtk_port_phyAutoNegoEnable_set(pVarInstant->parse.uniPort.portNo - 1, enable);
            if(RT_ERR_OK != ret)
            {
                pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
                return EPON_OAM_ERR_OK;
            }
        }
    }
    pVarContainer->varWidth = CTC_OAM_VAR_RESP_SETOK;

    return EPON_OAM_ERR_OK;
}

/* 0x09/0x000C - acAutoNegAdminControl */
int ctc_oam_varCb_acAutoNegAdminControl_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* caller prepare all the resource needed, 
                                          * handler won't need to allocate anything
                                          */
{
    int ret;
    unsigned int port;
    unsigned int setValue;
    rtk_enable_t enable;

    if(CTC_OAM_VAR_ACAUTONEGADMINCONTROL_LEN != pVarContainer->varWidth)
    {
        return EPON_OAM_ERR_UNKNOWN;
    }

    if(NULL == pVarInstant)
    {
        return EPON_OAM_ERR_NOT_FOUND;
    }

    if(CTC_INSTANT_IS_ETHERNET(pVarInstant))
    {
        CTC_BUF_PARSE32(pVarContainer->pVarData, &setValue);
        if(CTC_OAM_VAR_ACAUTONEGADMINCONTROL_DEACTIVE == setValue)
        {
            enable = DISABLED;
        }
        else if(CTC_OAM_VAR_ACAUTONEGADMINCONTROL_ACTIVE == setValue)
        {
            enable = ENABLED;
        }
        else
        {
            pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARBADPARAM;
            return EPON_OAM_ERR_OK;
        }

        if(CTC_INSTANT_IS_ALLPORTS(pVarInstant))
        {
            HAL_SCAN_ALL_PORT(port)
            {
                ret = rtk_port_phyAutoNegoEnable_set(port, enable);
                if(RT_ERR_OK != ret)
                {
                    pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
                    return EPON_OAM_ERR_OK;
                }
            }
        }
        else
        {
            ret = rtk_port_phyAutoNegoEnable_set(pVarInstant->parse.uniPort.portNo - 1, enable);
            if(RT_ERR_OK != ret)
            {
                pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
                return EPON_OAM_ERR_OK;
            }
        }
    }
    pVarContainer->varWidth = CTC_OAM_VAR_RESP_SETOK;

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x0001 - ONU SN */
int ctc_oam_varCb_onuSn_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    unsigned char *pPtr;
    unsigned char venderStr[] = "RTK";
    unsigned char hwVerStr[] = "sample";
    oam_config_t oamConfig;
    extern ctc_swDlBootInfo_t bootInfo;

    pVarContainer->varDesc = varDesc;
    pVarContainer->varWidth = 38; /* Fix value from CTC standard */
    pVarContainer->pVarData = (unsigned char *) malloc(pVarContainer->varWidth * sizeof(unsigned char));
    if(NULL == pVarContainer->pVarData)
    {
        return EPON_OAM_ERR_MEM;
    }

    pPtr = pVarContainer->pVarData;

    /* Vendor ID - RTK */
    memcpy(pVarContainer->pVarData, venderStr,
        sizeof(venderStr) > 4 ? 4 : sizeof(venderStr));
    pPtr += 4;
    
    /* ONU model - blank model */
    memset(pPtr, 0x0, 4);
    pPtr += 4;

    /* ONU ID - Local device MAC address */
    /* Get MAC address from EPON OAM stack configuration */
    epon_oam_config_get(llidIdx, &oamConfig);
    memcpy(pPtr, oamConfig.macAddr, sizeof(oamConfig.macAddr));
    pPtr += 6;

    /* MODIFY ME - Get actual HW version */
    /* Hardware Version - dummy HW version */
    memcpy(pPtr, hwVerStr, sizeof(hwVerStr) > 8 ? 8 : sizeof(hwVerStr));
    pPtr += 8;

    /* MODIFY ME - Get actual SW version */
    /* Software Version - dummy SW version */
    memcpy(pPtr, bootInfo.version, strlen(bootInfo.version) > 15 ? 15 : strlen(bootInfo.version));
    pPtr[strlen(bootInfo.version) > 15 ? 15 : strlen(bootInfo.version)] = '\0';
    pPtr += 16;

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x0002 - FirmwareVer */
int ctc_oam_varCb_firmwareVer_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    pVarContainer->varDesc = varDesc;
    pVarContainer->varWidth = 0x20; /* __DATE__ + __TIME__ size */
    pVarContainer->pVarData = (unsigned char *) malloc(pVarContainer->varWidth * sizeof(unsigned char));
    if(NULL == pVarContainer->pVarData)
    {
        return EPON_OAM_ERR_MEM;
    }

    snprintf(pVarContainer->pVarData, pVarContainer->varWidth, "%s %s", __DATE__, __TIME__);

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x0003 - Chipset ID */
int ctc_oam_varCb_chipsetId_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    pVarContainer->varDesc = varDesc;
    pVarContainer->varWidth = 0x08; /* Fix value from CTC standard */
    pVarContainer->pVarData = (unsigned char *) malloc(pVarContainer->varWidth * sizeof(unsigned char));
    if(NULL == pVarContainer->pVarData)
    {
        return EPON_OAM_ERR_MEM;
    }

    /* Vender ID - JEDEC ID (dummy ID here) */
    pVarContainer->pVarData[0] = 0x01;
    pVarContainer->pVarData[1] = 0x01;

    /* Chip Model - Dummy model here */
    pVarContainer->pVarData[2] = 0xaa;
    pVarContainer->pVarData[3] = 0xbb;

    /* Revision - Dummy revision here */
    pVarContainer->pVarData[4] = 0xcc;

    /* IC_Version/Date - data */
    pVarContainer->pVarData[5] = 0x12;
    pVarContainer->pVarData[6] = 0x10;
    pVarContainer->pVarData[7] = 0x30;
    
    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x0004 - ONU capabilities-1 */
int ctc_oam_varCb_onuCapabilities1_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    pVarContainer->varDesc = varDesc;
    /* For sample system, pretend it is a 4 ports GE SFU w/o multiple LLID */
    pVarContainer->varWidth = 0x1A; /* Fix value from CTC standard */
    pVarContainer->pVarData = (unsigned char *) malloc(pVarContainer->varWidth * sizeof(unsigned char));
    if(NULL == pVarContainer->pVarData)
    {
        return EPON_OAM_ERR_MEM;
    }

    /* ServiceSupported - Support GE ports */
    pVarContainer->pVarData[0] = 0x01;

    /* Number of GE Ports - 4 GE ports */
    pVarContainer->pVarData[1] = 0x04;

    /* Bitmap of GE Ports - port 1~4 */
    pVarContainer->pVarData[2] = 0x0f;
    pVarContainer->pVarData[3] = 0x00;
    pVarContainer->pVarData[4] = 0x00;
    pVarContainer->pVarData[5] = 0x00;
    pVarContainer->pVarData[6] = 0x00;
    pVarContainer->pVarData[7] = 0x00;
    pVarContainer->pVarData[8] = 0x00;
    pVarContainer->pVarData[9] = 0x00;

    /* Number of FE Ports - No FE ports */
    pVarContainer->pVarData[10] = 0x04;

    /* Bitmap of FE Ports - None */
    pVarContainer->pVarData[11] = 0x00;
    pVarContainer->pVarData[12] = 0x00;
    pVarContainer->pVarData[13] = 0x00;
    pVarContainer->pVarData[14] = 0x00;
    pVarContainer->pVarData[15] = 0x00;
    pVarContainer->pVarData[16] = 0x00;
    pVarContainer->pVarData[17] = 0x00;
    pVarContainer->pVarData[18] = 0x00;

    /* Number of POTS ports - No POTS ports */
    pVarContainer->pVarData[19] = 0x00;

    /* Number of E1 ports - No E1 ports */
    pVarContainer->pVarData[20] = 0x00;

    /* Number of US Queues - No US queue */
    pVarContainer->pVarData[21] = 0x00;

    /* QueueMax per Port US - No queue per port */
    pVarContainer->pVarData[22] = 0x00;

    /* Number of DS Queues - No DS queue */
    pVarContainer->pVarData[23] = 0x00;

    /* QueueMax per Port DS - No queue per port */
    pVarContainer->pVarData[24] = 0x00;

    /* Battery Backup - No battery backup */
    pVarContainer->pVarData[25] = 0x00;

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x0005 - OpticalTransceiverDiagnosis */
int ctc_oam_varCb_opticalTransceiverDiagnosis_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    pVarContainer->varDesc = varDesc;
    /* For sample system, pretend it is a 4 ports GE SFU w/o multiple LLID */
    pVarContainer->varWidth = 0x0A; /* Fix value from CTC standard */
    pVarContainer->pVarData = (unsigned char *) malloc(pVarContainer->varWidth * sizeof(unsigned char));
    if(NULL == pVarContainer->pVarData)
    {
        return EPON_OAM_ERR_MEM;
    }

    /* TransceiverTemperature - dummy temperature */
    pVarContainer->pVarData[0] = 0x11;
    pVarContainer->pVarData[1] = 0x22;

    /* Supply Voltage */
    pVarContainer->pVarData[2] = 0x33;
    pVarContainer->pVarData[3] = 0x44;

    /* Tx bias current */
    pVarContainer->pVarData[4] = 0x55;
    pVarContainer->pVarData[5] = 0x66;

    /* Tx power (output) */
    pVarContainer->pVarData[6] = 0x77;
    pVarContainer->pVarData[7] = 0x88;

    /* Rx power (input) */
    pVarContainer->pVarData[8] = 0x99;
    pVarContainer->pVarData[9] = 0xaa;

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x0006 - Service SLA */
int ctc_oam_varCb_serviceSla_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
}

/* 0xC7/0x0006 - Service SLA */
int ctc_oam_varCb_serviceSla_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    /* Pretend set operation complete */
    pVarContainer->varWidth = 0x80;

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x0007 - ONU capabilities-2 */
int ctc_oam_varCb_onuCapabilities2_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    pVarContainer->varDesc = varDesc;
    /* For sample system, pretend it is a 4 ports GE SFU w/o multiple LLID */
    pVarContainer->varWidth = 10 + 6;
    pVarContainer->pVarData = (unsigned char *) malloc(pVarContainer->varWidth * sizeof(unsigned char));
    if(NULL == pVarContainer->pVarData)
    {
        return EPON_OAM_ERR_MEM;
    }

    /* ONU Type - SFU */
    pVarContainer->pVarData[0] = 0x00;
    pVarContainer->pVarData[1] = 0x00;
    pVarContainer->pVarData[2] = 0x00;
    pVarContainer->pVarData[3] = 0x00;

    /* MultiLLID - single LLID */
    pVarContainer->pVarData[4] = 0x01;

    /* ProtectionType - Unsupport */
    pVarContainer->pVarData[5] = 0x00;

    /* Num of PON IF - 1 PON IF */
    pVarContainer->pVarData[6] = 0x01;

    /* Num of Slot - SFU */
    pVarContainer->pVarData[7] = 0x00;

    /* Num of Interface type - 1 IF Type */
    pVarContainer->pVarData[8] = 0x01;

    /* Interface Type - GE */
    pVarContainer->pVarData[9] = 0x00;
    pVarContainer->pVarData[10] = 0x00;
    pVarContainer->pVarData[11] = 0x00;
    pVarContainer->pVarData[12] = 0x00;

    /* Num of Port - 4 ports */
    pVarContainer->pVarData[13] = 0x00;
    pVarContainer->pVarData[14] = 0x04;

    /* Battery Backup - No battery backup */
    pVarContainer->pVarData[15] = 0x00;

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x0008 - HoldoverConfig */
int ctc_oam_varCb_holdoverConfig_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    int ret = RT_ERR_OK;
    unsigned int getState, getTime;

    pVarContainer->varDesc = varDesc;
    pVarContainer->varWidth = CTC_OAM_VAR_HOLDOVERCONFIG_LEN; /* Fix value from CTC standard */
    pVarContainer->pVarData = (unsigned char *) malloc(pVarContainer->varWidth * sizeof(unsigned char));
    if(NULL == pVarContainer->pVarData)
    {
        return EPON_OAM_ERR_MEM;
    }

    /* MODIFY ME - get from OAM module */

    /* ret = rtk_epon_dsFecState_get(&dsState); */
    if(RT_ERR_OK != ret)
    {
        return EPON_OAM_ERR_UNKNOWN;
    }

    /* Construct return value */
    CTC_BUF_ENCODE32(pVarContainer->pVarData, &getState);
    CTC_BUF_ENCODE32(pVarContainer->pVarData + 4, &getTime);

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x0008 - HoldoverConfig */
int ctc_oam_varCb_holdoverConfig_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    int ret;
    unsigned int setState, setTime;

    if(CTC_OAM_VAR_HOLDOVERCONFIG_LEN != pVarContainer->varWidth)
    {
        return EPON_OAM_ERR_UNKNOWN;
    }
    CTC_BUF_PARSE32(pVarContainer->pVarData, &setState);
    CTC_BUF_PARSE32(pVarContainer->pVarData + 4, &setTime);

    /* MODIFY ME - set to OAM module */

    pVarContainer->varWidth = 0x80;

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x0009 - MxUMngGlobalParameter */
int ctc_oam_varCb_mxUMngGlobalParameter_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
}

/* 0xC7/0x0009 - MxUMngGlobalParameter */
int ctc_oam_varCb_mxUMngGlobalParameter_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    /* Pretend set operation complete */
    pVarContainer->varWidth = 0x80;

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x000A - MxUMngSNMPParameter */
int ctc_oam_varCb_mxUMngSNMPParameter_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
}

/* 0xC7/0x000A - MxUMngSNMPParameter */
int ctc_oam_varCb_mxUMngSNMPParameter_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    /* Pretend set operation complete */
    pVarContainer->varWidth = 0x80;

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x000B - Active PON_IFAdminstate */
int ctc_oam_varCb_activePonIfAdminstate_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    pVarContainer->varDesc = varDesc;
    pVarContainer->varWidth = CTC_OAM_VAR_ACTIVEPONIFADMINSTATE_LEN; /* Fix value from CTC standard */
    pVarContainer->pVarData = (unsigned char *) malloc(pVarContainer->varWidth * sizeof(unsigned char));
    if(NULL == pVarContainer->pVarData)
    {
        return EPON_OAM_ERR_MEM;
    }

    /* Fix value for the PON number */
    pVarContainer->pVarData[0] = CTC_OAM_VAR_ACTIVEPONIFADMINSTATE_PONNUM;

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x000B - Active PON_IFAdminstate */
int ctc_oam_varCb_activePonIfAdminstate_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    /* Not support type C/D protection */
    pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x000C - ONU capabilities-3 */
int ctc_oam_varCb_onuCapabilities3_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    pVarContainer->varDesc = varDesc;
    /* For sample system, pretend it doesn't support IPv6 aware and power control */
    pVarContainer->varWidth = CTC_OAM_VAR_ONUCAPABILITES_LEN; /* Fix value from CTC standard */
    pVarContainer->pVarData = (unsigned char *) malloc(pVarContainer->varWidth * sizeof(unsigned char));
    if(NULL == pVarContainer->pVarData)
    {
        return EPON_OAM_ERR_MEM;
    }

    /* IPv6 Supported - not supported */
    pVarContainer->pVarData[0] = 0x00;

    /* ONUPowerSupplyControl - not supported */
    pVarContainer->pVarData[1] = 0x00;

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x000D - ONU power saving capabilities */
int ctc_oam_varCb_onuPowerSavingCapabilities_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    pVarContainer->varDesc = varDesc;
    pVarContainer->varWidth = CTC_OAM_VAR_ONUPOWERSAVINGCAPABILITES_LEN; /* Fix value from CTC standard */
    pVarContainer->pVarData = (unsigned char *) malloc(pVarContainer->varWidth * sizeof(unsigned char));
    if(NULL == pVarContainer->pVarData)
    {
        return EPON_OAM_ERR_MEM;
    }

    /* Sleep mode capability - not supported */
    pVarContainer->pVarData[0] = 0x00;

    /* Early wake-up capability - not supported */
    pVarContainer->pVarData[0] = 0x00;

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x000E - ONU power saving config */
int ctc_oam_varCb_onuPowerSavingConfig_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    pVarContainer->varDesc = varDesc;
    pVarContainer->varWidth = CTC_OAM_VAR_ONUPOWERSAVINGCONFIG_LEN; /* Fix value from CTC standard */
    pVarContainer->pVarData = (unsigned char *) malloc(pVarContainer->varWidth * sizeof(unsigned char));
    if(NULL == pVarContainer->pVarData)
    {
        return EPON_OAM_ERR_MEM;
    }

    /* Early wake-up - unsupport */
    pVarContainer->pVarData[0] = CTC_OAM_VAR_ONUPOWERSAVINGCONFIG_UNSUPPORT;

    /* Sleep duration max - not support so fill in all zeros */
    memset((pVarContainer->pVarData+1), 0, CTC_OAM_VAR_ONUPOWERSAVINGCONFIG_LEN - 1);

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x000E - ONU power saving config */
int ctc_oam_varCb_onuPowerSavingConfig_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    /* Not support power saving */
    pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x000F - ONU Protection Parameters */
int ctc_oam_varCb_onuProtectionParameters_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    pVarContainer->varDesc = varDesc;
    pVarContainer->varWidth = CTC_OAM_VAR_ONUPROTECTIONPARAMETERS_LEN; /* Fix value from CTC standard */
    pVarContainer->pVarData = (unsigned char *) malloc(pVarContainer->varWidth * sizeof(unsigned char));
    if(NULL == pVarContainer->pVarData)
    {
        return EPON_OAM_ERR_MEM;
    }

    /* Due to the actual LOS time setting is on the tranceiver side
     * And the tranciver control should be different from vender to vender
     * So here always return the default value.
     */

    /* T LOS_Optical - 2 ms */
    CTC_BUF_ENCODE16(pVarContainer->pVarData, CTC_OAM_VAR_ONUPROTECTIONPARAMETERS_LOSOPT);

    /* MODIFY ME - add MPCP timeout settings */
    /* T LOS_MAC - 50 ms */
    CTC_BUF_ENCODE16(pVarContainer->pVarData + 2, CTC_OAM_VAR_ONUPROTECTIONPARAMETERS_LOSMAC);

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x000F - ONU Protection Parameters */
int ctc_oam_varCb_onuProtectionParameters_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    /* Due to the actual LOS time setting is on the tranceiver side
     * And the tranciver control should be different from vender to vender
     * So here always return the default value.
     */

    /* Pretend set operation complete */
    pVarContainer->varWidth = 0x80;

    return EPON_OAM_ERR_OK;
}
/* 0xC7/0x0011 - EthLinkState */
int ctc_oam_varCb_ethLinkState_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    int ret;
    rtk_port_linkStatus_t linkStatus;

    if(NULL == pVarInstant)
    {
        return EPON_OAM_ERR_NOT_FOUND;
    }

    pVarContainer->varDesc = varDesc;
    pVarContainer->varWidth = CTC_OAM_VAR_ETHLINKSTATE_LEN; /* Fix value from CTC standard */
    pVarContainer->pVarData = (unsigned char *) malloc(pVarContainer->varWidth * sizeof(unsigned char));
    if(NULL == pVarContainer->pVarData)
    {
        return EPON_OAM_ERR_MEM;
    }

    if(!CTC_INSTANT_IS_ALLPORTS(pVarInstant) && CTC_INSTANT_IS_ETHERNET(pVarInstant))
    {
        ret = rtk_port_link_get(pVarInstant->parse.uniPort.portNo - 1, &linkStatus);
        if(RT_ERR_OK == ret)
        {
            if(PORT_LINKDOWN == linkStatus)
            {
                *pVarContainer->pVarData = CTC_OAM_VAR_ETHLINKSTATE_DOWN;
            }
            else
            {
                *pVarContainer->pVarData = CTC_OAM_VAR_ETHLINKSTATE_UP;
            }
        }
        else
        {
            return EPON_OAM_ERR_UNKNOWN;
        }
    }

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x0012 - EthPortPause */
int ctc_oam_varCb_ethPortPause_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    int ret;
    rtk_port_phy_ability_t ability;

    if(NULL == pVarInstant)
    {
        return EPON_OAM_ERR_NOT_FOUND;
    }

    pVarContainer->varDesc = varDesc;
    pVarContainer->varWidth = CTC_OAM_VAR_ETHPORTPAUSE_LEN; /* Fix value from CTC standard */
    pVarContainer->pVarData = (unsigned char *) malloc(pVarContainer->varWidth * sizeof(unsigned char));
    if(NULL == pVarContainer->pVarData)
    {
        return EPON_OAM_ERR_MEM;
    }

    if(!CTC_INSTANT_IS_ALLPORTS(pVarInstant) && CTC_INSTANT_IS_ETHERNET(pVarInstant))
    {
        ret = rtk_port_phyAutoNegoAbility_get(pVarInstant->parse.uniPort.portNo - 1, &ability);
        if(RT_ERR_OK == ret)
        {
            if(ability.FC)
            {
                *pVarContainer->pVarData = CTC_OAM_VAR_ETHPORTPAUSE_ENABLE;
            }
            else
            {
                *pVarContainer->pVarData = CTC_OAM_VAR_ETHPORTPAUSE_DISABLE;
            }
        }
        else
        {
            return EPON_OAM_ERR_UNKNOWN;
        }
    }

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x0012 - ethPort Pause */
int ctc_oam_varCb_ethPortPause_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* caller prepare all the resource needed, 
                                          * handler won't need to allocate anything
                                          */
{
    int ret;
    unsigned int port;
    unsigned char setValue;
    rtk_port_phy_ability_t ability;

    if(CTC_OAM_VAR_ETHPORTPAUSE_LEN != pVarContainer->varWidth)
    {
        return EPON_OAM_ERR_UNKNOWN;
    }

    if(NULL == pVarInstant)
    {
        return EPON_OAM_ERR_NOT_FOUND;
    }

    if(CTC_INSTANT_IS_ETHERNET(pVarInstant))
    {
        ret = rtk_port_phyAutoNegoAbility_get(pVarInstant->parse.uniPort.portNo - 1, &ability);
        if(RT_ERR_OK != ret)
        {
            pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
            return EPON_OAM_ERR_OK;
        }

        setValue = *pVarContainer->pVarData;
        if(CTC_OAM_VAR_ETHPORTPAUSE_DISABLE == setValue)
        {
            ability.FC = DISABLED;
        }
        else if(CTC_OAM_VAR_ETHPORTPAUSE_ENABLE == setValue)
        {
            ability.FC = ENABLED;
        }
        else
        {
            pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARBADPARAM;
            return EPON_OAM_ERR_OK;
        }

        if(CTC_INSTANT_IS_ALLPORTS(pVarInstant))
        {
            HAL_SCAN_ALL_PORT(port)
            {
                ret = rtk_port_phyAutoNegoAbility_set(port, &ability);
                if(RT_ERR_OK != ret)
                {
                    pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
                    return EPON_OAM_ERR_OK;
                }
            }
        }
        else
        {
            ret = rtk_port_phyAutoNegoAbility_set(pVarInstant->parse.uniPort.portNo - 1, &ability);
            if(RT_ERR_OK != ret)
            {
                pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
                return EPON_OAM_ERR_OK;
            }
        }
    }
    pVarContainer->varWidth = CTC_OAM_VAR_RESP_SETOK;

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x0013 - ethPortUs Policing */
int ctc_oam_varCb_ethPortUsPolicing_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* caller prepare all the resource needed, 
                                          * handler won't need to allocate anything
                                          */
{
    /* MODIFY ME - Change to actual action */
    /* Pretend set operation complete */
    pVarContainer->varWidth = 0x80;

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x0017 - PortLoopDetect */
int ctc_oam_varCb_portLoopDetect_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* caller prepare all the resource needed, 
                                          * handler won't need to allocate anything
                                          */
{
    int ret;
    unsigned int port;
    unsigned int setValue;
    rtk_enable_t enable;
    rtk_rldp_config_t config;
    rtk_rldp_portConfig_t portConfig;

    if(CTC_OAM_VAR_PORTLOOPDETECT_LEN != pVarContainer->varWidth)
    {
        return EPON_OAM_ERR_UNKNOWN;
    }

    if(NULL == pVarInstant)
    {
        return EPON_OAM_ERR_NOT_FOUND;
    }

    if(CTC_INSTANT_IS_ETHERNET(pVarInstant))
    {
        /* Make sure the global RLDP is enabled */
        ret = rtk_rldp_config_get(&config);
        if(RT_ERR_OK != ret)
        {
            pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
            return EPON_OAM_ERR_OK;
        }
        config.rldp_enable = ENABLED;
        ret = rtk_rldp_config_set(&config);
        if(RT_ERR_OK != ret)
        {
            pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
            return EPON_OAM_ERR_OK;
        }

        CTC_BUF_PARSE32(pVarContainer->pVarData, &setValue);
        if(CTC_OAM_VAR_PORTLOOPDETECT_DEACTIVE == setValue)
        {
            enable = DISABLED;
        }
        else if(CTC_OAM_VAR_PORTLOOPDETECT_ACTIVE == setValue)
        {
            enable = ENABLED;
        }
        else
        {
            pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARBADPARAM;
            return EPON_OAM_ERR_OK;
        }

        if(CTC_INSTANT_IS_ALLPORTS(pVarInstant))
        {
            HAL_SCAN_ALL_PORT(port)
            {
                ret = rtk_rldp_portConfig_get(port, &portConfig);
                if(RT_ERR_OK != ret)
                {
                    pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
                    return EPON_OAM_ERR_OK;
                }
                portConfig.tx_enable = enable;
                ret = rtk_rldp_portConfig_set(port, &portConfig);
                if(RT_ERR_OK != ret)
                {
                    pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
                    return EPON_OAM_ERR_OK;
                }
            }
        }
        else
        {
            ret = rtk_rldp_portConfig_get(pVarInstant->parse.uniPort.portNo - 1, &portConfig);
            if(RT_ERR_OK != ret)
            {
                pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
                return EPON_OAM_ERR_OK;
            }
            portConfig.tx_enable = enable;
            ret = rtk_rldp_portConfig_set(port, &portConfig);
            if(RT_ERR_OK != ret)
            {
                pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
                return EPON_OAM_ERR_OK;
            }
        }
    }
    pVarContainer->varWidth = CTC_OAM_VAR_RESP_SETOK;

    return EPON_OAM_ERR_OK;
}

int ctc_oam_varCb_portMacAgingTime_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    int ret;
    unsigned int getValue;
    rtk_enable_t enable;

    if(NULL == pVarInstant)
    {
        return EPON_OAM_ERR_NOT_FOUND;
    }

    pVarContainer->varDesc = varDesc;
    pVarContainer->varWidth = CTC_OAM_VAR_PORTMACAGINGTIME_LEN; /* Fix value from CTC standard */
    pVarContainer->pVarData = (unsigned char *) malloc(pVarContainer->varWidth * sizeof(unsigned char));
    if(NULL == pVarContainer->pVarData)
    {
        return EPON_OAM_ERR_MEM;
    }

    if(!CTC_INSTANT_IS_ALLPORTS(pVarInstant) && CTC_INSTANT_IS_ETHERNET(pVarInstant))
    {
        /* TODO - Apollo does not support per-port aging time
         * It only support per-system aging time
         */
        ret = rtk_l2_portAgingEnable_get(pVarInstant->parse.uniPort.portNo - 1, &enable);
        if(RT_ERR_OK == ret)
        {
            if(ENABLED == enable)
            {
                ret = rtk_l2_aging_get(&getValue);
                if(RT_ERR_OK != ret)
                {
                    pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
                    return EPON_OAM_ERR_OK;
                }
            }
            else
            {
                getValue = CTC_OAM_VAR_PORTMACAGINGTIME_DEACTIVE;
            }
            CTC_BUF_ENCODE32(pVarContainer->pVarData, &getValue);
        }
        else
        {
            return EPON_OAM_ERR_UNKNOWN;
        }
    }

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x0019 - PortMACAgingTime */
int ctc_oam_varCb_portMacAgingTime_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* caller prepare all the resource needed, 
                                          * handler won't need to allocate anything
                                          */
{
    int ret;
    unsigned int port;
    unsigned int setValue;
    rtk_enable_t enable;

    if(CTC_OAM_VAR_PORTMACAGINGTIME_LEN != pVarContainer->varWidth)
    {
        return EPON_OAM_ERR_UNKNOWN;
    }

    if(NULL == pVarInstant)
    {
        return EPON_OAM_ERR_NOT_FOUND;
    }

    /* TODO - Apollo does not support per-port aging time
     * It only support per-system aging time
     */
    CTC_BUF_PARSE32(pVarContainer->pVarData, &setValue);
    if(CTC_OAM_VAR_PORTMACAGINGTIME_DEACTIVE == setValue)
    {
        setValue = 0;
        enable = DISABLED;
    }
    else
    {
        enable = ENABLED;
    }

    ret = rtk_l2_aging_set(setValue);
    if(RT_ERR_OK != ret)
    {
        pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
        return EPON_OAM_ERR_OK;
    }

    /* Turn on/off each ports' aging out mechanism */
    if(CTC_INSTANT_IS_ETHERNET(pVarInstant))
    {
        if(CTC_INSTANT_IS_ALLPORTS(pVarInstant))
        {
            HAL_SCAN_ALL_PORT(port)
            {
                ret = rtk_l2_portAgingEnable_set(port, &enable);
                if(RT_ERR_OK != ret)
                {
                    pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
                    return EPON_OAM_ERR_OK;
                }
            }
        }
        else
        {
            ret = rtk_l2_portAgingEnable_set(pVarInstant->parse.uniPort.portNo - 1, &enable);
            if(RT_ERR_OK != ret)
            {
                pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
                return EPON_OAM_ERR_OK;
            }
        }
    }
    pVarContainer->varWidth = CTC_OAM_VAR_RESP_SETOK;

    return EPON_OAM_ERR_OK;
}

/* According to CTC Spec, the transparent mode is defined as:
 * Upstream: tag in, tag out, untag in, untag out. No change to tag.
 * Downstream: tag in, tag out, untag in, untag out. No change to tag.
 */
static int ctc_oam_varCb_vlanTransparent_set(
        rtk_port_t port)
{
    int ret;
    rtk_port_t ponPort;
    rtk_portmask_t igrPortMask;
    rtk_vlan_tagKeepType_t keepType;

    /* Set up a transparent tunnel between PON port and UNI port */
    ponPort = HAL_GET_PON_PORT();
    /* Set upstream (PON port) */
    ret = rtk_vlan_portEgrTagKeepType_get(ponPort, &igrPortMask, &keepType);
    if(RT_ERR_OK != ret)
    {
        return EPON_OAM_ERR_UNKNOWN;
    }
    RTK_PORTMASK_PORT_SET(igrPortMask, port);
    ret = rtk_vlan_portEgrTagKeepType_set(ponPort, &igrPortMask, keepType);
    if(RT_ERR_OK != ret)
    {
        return EPON_OAM_ERR_UNKNOWN;
    }

    /* Set downstream (UNI port) */
    ret = rtk_vlan_portEgrTagKeepType_get(port, &igrPortMask, &keepType);
    if(RT_ERR_OK != ret)
    {
        return EPON_OAM_ERR_UNKNOWN;
    }
    RTK_PORTMASK_PORT_SET(igrPortMask, ponPort);
    ret = rtk_vlan_portEgrTagKeepType_set(port, &igrPortMask, keepType);
    if(RT_ERR_OK != ret)
    {
        return EPON_OAM_ERR_UNKNOWN;
    }

    return EPON_OAM_ERR_OK;
}

/* According to CTC Spec, the transparent mode is defined as:
 * Upstream: untag in, tag out, tag in drop.
 * Downstream: tag in, untag out, untag in, drop.
 */
static int ctc_oam_varCb_vlanTag_set(
    rtk_port_t port,
    unsigned int vlan,
    unsigned int priority)
{
    int ret;

    ret = rtk_vlan_portAcceptFrameType_set(port, ACCEPT_FRAME_TYPE_UNTAG_ONLY);
    if(RT_ERR_OK != ret)
    {
        return EPON_OAM_ERR_UNKNOWN;
    }
    ret = rtk_vlan_portPvid_set(port, vlan);
    if(RT_ERR_OK != ret)
    {
        return EPON_OAM_ERR_UNKNOWN;
    }
    ret = rtk_vlan_portPriority_set(port, priority);
    if(RT_ERR_OK != ret)
    {
        return EPON_OAM_ERR_UNKNOWN;
    }

    return EPON_OAM_ERR_OK;
}
    
/* 0xC7/0x0021 - VLAN */
int ctc_oam_varCb_vlan_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* caller prepare all the resource needed, 
                                          * handler won't need to allocate anything
                                          */
{
    int ret = RT_ERR_OK;
    unsigned char vlanMode;
    unsigned int port;
    unsigned int vlanTag;
    unsigned int vlan, priority;

    /* For VLAN variable, the length depends on the VLAN mode */
    if(CTC_OAM_VAR_VLAN_LEN >= pVarContainer->varWidth)
    {
        return EPON_OAM_ERR_UNKNOWN;
    }

    if(NULL == pVarInstant)
    {
        return EPON_OAM_ERR_NOT_FOUND;
    }

    if(CTC_INSTANT_IS_ETHERNET(pVarInstant) && !CTC_INSTANT_IS_ALLPORTS(pVarInstant))
    {
        vlanMode = pVarContainer->pVarData[0];

#if 0
        switch(vlanMode)
        {
        case CTC_OAM_VAR_VLAN_MODE_TRANSPARENT:
            ret = ctc_oam_varCb_vlanTransparent_set(pVarInstant->parse.uniPort.portNo-1);
            break;
        case CTC_OAM_VAR_VLAN_MODE_TAG:
            if(CTC_OAM_VAR_VLAN_TAG_LEN != pVarContainer->varWidth)
            {
                return EPON_OAM_ERR_UNKNOWN;
            }
            CTC_BUF_PARSE32((pVarContainer->pVarData + 1), &vlanTag);
            vlan = vlanTag & 0x0fffUL;
            priority = (vlanTag & 0xe000UL) >> 13;
            ret = ctc_oam_varCb_vlanTag_set(pVarInstant->parse.uniPort.portNo-1, vlan, priority);
            break;
        case CTC_OAM_VAR_VLAN_MODE_TRANSLATION:
            break;
        case CTC_OAM_VAR_VLAN_MODE_AGGREGATION:
            break;
        case CTC_OAM_VAR_VLAN_MODE_TRUNK:
            break;
        default:
            break;
        }
#endif
        if(RT_ERR_OK != ret)
        {
            pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARNORESOURCE;
        }
        else
        {
            pVarContainer->varWidth = CTC_OAM_VAR_RESP_SETOK;
        }
    }
    else
    {
        pVarContainer->varWidth = CTC_OAM_VAR_RESP_VARBADPARAM;
    }

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x0031 - Classification&Marking */
int ctc_oam_varCb_calssificationMarking_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* caller prepare all the resource needed, 
                                          * handler won't need to allocate anything
                                          */
{
    /* MODIFY ME - Change to actual action */
    /* Pretend set operation complete */
    pVarContainer->varWidth = 0x80;

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x0041 - Add/Del Multicast VLAN */
int ctc_oam_varCb_addDelMulticastVlan_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* caller prepare all the resource needed, 
                                          * handler won't need to allocate anything
                                          */
{
    /* MODIFY ME - Change to actual action */
    /* Pretend set operation complete */
    pVarContainer->varWidth = 0x80;

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x0042 - MulticastTagOper */
int ctc_oam_varCb_multicastTagOper_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* caller prepare all the resource needed, 
                                          * handler won't need to allocate anything
                                          */
{
    /* MODIFY ME - Change to actual action */
    /* Pretend set operation complete */
    pVarContainer->varWidth = 0x80;

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x0043 - MulticastSwitch */
int ctc_oam_varCb_multicastSwitch_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* caller prepare all the resource needed, 
                                          * handler won't need to allocate anything
                                          */
{
    /* MODIFY ME - Change to actual action */
    /* Pretend set operation complete */
    pVarContainer->varWidth = 0x80;

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x0045 - Group Num Max */
int ctc_oam_varCb_groupNumMax_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* caller prepare all the resource needed, 
                                          * handler won't need to allocate anything
                                          */
{
    /* MODIFY ME - Change to actual action */
    /* Pretend set operation complete */
    pVarContainer->varWidth = 0x80;

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x0046 - aFastLeaveAbility */
int ctc_oam_varCb_aFastLeaveAbility_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    unsigned int enumCnt = 6, enumValue;

    pVarContainer->varDesc = varDesc;
    /* Length varies according to the enum number
     * Here reply all possible enum values defined in CTC standard
     */
    pVarContainer->varWidth = 
        CTC_OAM_VAR_AFASTLEAVEABILITY_HDR_LEN +
        (CTC_OAM_VAR_AFASTLEAVEABILITY_ENUM_LEN * enumCnt);
    pVarContainer->pVarData = (unsigned char *) malloc(pVarContainer->varWidth * sizeof(unsigned char));
    if(NULL == pVarContainer->pVarData)
    {
        return EPON_OAM_ERR_MEM;
    }

    /* Fill in the header */
    CTC_BUF_ENCODE32(pVarContainer->pVarData, &enumCnt);

    /* 1st enum */
    enumValue = CTC_OAM_VAR_AFASTLEAVEABILITY_SNOOPING_NONFASTLEAVE;
    CTC_BUF_ENCODE32((pVarContainer->pVarData + 4), &enumValue);

    /* 2nd enum */
    enumValue = CTC_OAM_VAR_AFASTLEAVEABILITY_SNOOPING_FASTLEAVE;
    CTC_BUF_ENCODE32((pVarContainer->pVarData + 8), &enumValue);

    /* 3rd enum */
    enumValue = CTC_OAM_VAR_AFASTLEAVEABILITY_IGMP_NONFASTLEAVE;
    CTC_BUF_ENCODE32((pVarContainer->pVarData + 12), &enumValue);

    /* 4th enum */
    enumValue = CTC_OAM_VAR_AFASTLEAVEABILITY_IGMP_FASTLEAVE;
    CTC_BUF_ENCODE32((pVarContainer->pVarData + 16), &enumValue);

    /* 5th enum */
    enumValue = CTC_OAM_VAR_AFASTLEAVEABILITY_MLD_NONFASTLEAVE;
    CTC_BUF_ENCODE32((pVarContainer->pVarData + 20), &enumValue);

    /* 6th enum */
    enumValue = CTC_OAM_VAR_AFASTLEAVEABILITY_MLD_FASTLEAVE;
    CTC_BUF_ENCODE32((pVarContainer->pVarData + 24), &enumValue);

    return EPON_OAM_ERR_OK;
}

/* 0xC7/0x0047 - aFastLeaveAdminState */
int ctc_oam_varCb_aFastLeaveAdminState_get(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* handler should allocate resource, 
                                          * caller will release it
                                          */
{
    unsigned int setValue;
    pVarContainer->varDesc = varDesc;
    pVarContainer->varWidth = CTC_OAM_VAR_AFASTLEAVEADMINSTATE_LEN; /* Fix value from CTC standard */
    pVarContainer->pVarData = (unsigned char *) malloc(pVarContainer->varWidth * sizeof(unsigned char));
    if(NULL == pVarContainer->pVarData)
    {
        return EPON_OAM_ERR_MEM;
    }

    /* MODIFY ME - change to actual get function and fill the value */
    setValue = CTC_OAM_VAR_AFASTLEAVEADMINSTATE_DISABLED;
    CTC_BUF_ENCODE32(pVarContainer->pVarData, &setValue);

    return EPON_OAM_ERR_OK;
}

/* 0xC9/0x0001 - Reset ONU */
int ctc_oam_varCb_resetOnu_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* caller prepare all the resource needed, 
                                          * handler won't need to allocate anything
                                          */
{
    pVarContainer->varWidth = 0x80;

    /* OLT issue reset ONU command */
    printf("%s:%d OLU issue reset ONU command\n", __FILE__, __LINE__);
    /* Sleep 3 seconds to make sure the print complete */
    sleep(3);
    reboot(LINUX_REBOOT_CMD_RESTART);

    return EPON_OAM_ERR_OK;
}

/* 0xC9/0x0048 - acFastLeaveAdminControl */
int ctc_oam_varCb_acFastLeaveAdminControl_set(
    unsigned char llidIdx, /* LLID index of the incoming operation */
    unsigned char op, /* Operation to be taken */
    ctc_varInstant_t *pVarInstant, /* It might be NULL if no instant specified */
    ctc_varDescriptor_t varDesc,
    ctc_varContainer_t *pVarContainer)   /* caller prepare all the resource needed, 
                                          * handler won't need to allocate anything
                                          */
{
    /* MODIFY ME - Change to actual action */
    /* Pretend set operation complete */
    pVarContainer->varWidth = 0x80;

    return EPON_OAM_ERR_OK;
}

int ctc_oam_churningKey_set(
    unsigned char llidIdx,
    unsigned char keyIdx,
    unsigned char key[])
{
    rtk_epon_churningKeyEntry_t entry;

    entry.llidIdx = llidIdx;
    entry.keyIdx = keyIdx;
    memcpy(entry.churningKey, key, RTK_EPON_KEY_SIZE);

    /* Call RTK API to set key to HW */
    rtk_epon_churningKey_set(&entry);

    return EPON_OAM_ERR_OK;
}

int ctc_oam_dbaConfig_get(
    ctc_dbaThreshold_t *pDbaThreshold)
{
    /* MODIFY ME - replace with acturl RTK API
     * Fill in the dummy database
     */
    *pDbaThreshold = dbaThresholdDb;

    return EPON_OAM_ERR_OK;
}

int ctc_oam_dbaConfig_set(
    ctc_dbaThreshold_t *pDbaSetResp)
{
    /* MODIFY ME - replace with acturl RTK API
     * pretend to store the data
     */
    dbaThresholdDb = *pDbaSetResp;

    return EPON_OAM_ERR_OK;
}


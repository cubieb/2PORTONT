/*
 * Copyright (C) 2014 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : Definition of OMCI pm related define
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI pm related define
 */

#ifndef __OMCI_PM_H__
#define __OMCI_PM_H__

#ifdef  __cplusplus
extern "C" {
#endif


#include "gos_type.h"


// define pm properties
#define OMCI_EXT_PM_ACCUM_GLOBAL_CLEAR_BIT		(15)
#define OMCI_EXT_PM_ACCUM_GLOBAL_DISABLE_BIT	(14)
#define OMCI_EXT_PM_TCA_GLOBAL_DISABLE_BIT		(14)
#define OMCI_EXT_PM_CONT_ACCUM_MODE_BIT			(0)
#define OMCI_EXT_PM_DIRECTION_BIT				(1)

// define pm macro
#define m_omci_pm_update_accum_attr(new, old) \
    do { if (new >= old) old = new; else old = ~0; } while (0);

#define m_omci_pm_update_reset_u32_attr(new, old) \
    do { if ((UINT32)(new + old) >= old) old += new; else old = ~0; } while (0);

#define m_omci_pm_update_reset_u64_attr(new, old) \
    do { \
        UINT32 _high = sizeof(new) > 4 ? (new >> 32) : 0; \
        UINT32 _low = (new & 0xFFFFFFFF); \
        UINT8 _carry = 0; \
        if ((_low + old.low) < old.low) \
            _carry = 1; \
        old.low += _low; \
        if ((_high + _carry + old.high) < old.high) \
            { old.high = ~0; old.low = ~0; } \
        else \
            old.high += _high + _carry; \
    } while (0);

#define m_omci_ext_pm_accum_all_clear(x)		((x >> OMCI_EXT_PM_ACCUM_GLOBAL_CLEAR_BIT) & 0x1)
#define m_omci_ext_pm_accum_all_disable(x)		((x >> OMCI_EXT_PM_ACCUM_GLOBAL_DISABLE_BIT) & 0x1)
#define m_omci_ext_pm_accum_disable(x, index)	((x >> (index - 4)) & 0x1)
#define m_omci_ext_pm_tca_all_disable(x)		((x >> OMCI_EXT_PM_TCA_GLOBAL_DISABLE_BIT) & 0x1)
#define m_omci_ext_pm_tca_disable(x, index)		((x >> (index - 4)) & 0x1)

#define m_omci_ext_pm_continous_accum_mode(x)	((x >> OMCI_EXT_PM_CONT_ACCUM_MODE_BIT) & 0x1)
#define m_omci_ext_pm_directionality(x)			((x >> OMCI_EXT_PM_DIRECTION_BIT) & 0x1)

// define pm structure
typedef struct {
	UINT16	ThresholdDataID;
	UINT16	ParentMeClass;
	UINT16	ParentMeInstance;
	UINT16	AccumDisable;
	UINT16	TcaDisable;
	UINT16	CtrlFields;
} omci_me_attr_ext_pm_ctrl_blk_t;


#ifdef  __cplusplus
}
#endif

#endif

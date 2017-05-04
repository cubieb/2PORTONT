/*
 * Copyright (C) 2009 Realtek Semiconductor Corp. 
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
 * Purpose : Definition the basic types in the SDK.
 *
 * Feature : type definition
 *
 */

#ifndef __HAL_COMMON_HAL_ADPT_H__
#define __HAL_COMMON_HAL_ADPT_H__

/*
 * Symbol Definition
 */


#define MAX_PHY_PORT            (7)     /* number of physical ports */
#define MAX_TRK_PORT            (8)     /* number of trunk ports */
#define MAX_CPU_PORT            (1)     /* number of cpu ports */
#define MAX_PHY_N_CPU_PORT      (MAX_PHY_PORT + MAX_CPU_PORT)
#define MAX_LOGIC_PORT          (MAX_PHY_N_CPU_PORT + MAX_TRK_PORT)

#define MAX_PORT                (MAX_PHY_PORT)

/*
 * Data Type Declaration
 */


/*
 * Macro Definition
 */

#endif /* __HAL_COMMON_HAL_ADPT_H__ */


/*
 * Copyright(c) Realtek Semiconductor Corporation, 2011
 * All rights reserved.
 *
 * $Revision: 39101 $
 * $Date: 2013-05-03 17:35:27 +0800 (Fri, 03 May 2013) $
 *
 * Purpose : Definition of HAL API test APIs in the SDK
 *
 * Feature : HAL API test APIs
 *
 */

#ifndef __HAL_REG_TEST_CASE_H__
#define __HAL_REG_TEST_CASE_H__

/*
 * Include Files
 */
#include <common/rt_type.h>


/*
 * Function Declaration
 */

extern int32
hal_reg_def_test(uint32 testcase);

extern int32
hal_reg_rw_test(uint32 testcase);


#endif  /* __HAL_REG_TEST_CASE_H__ */

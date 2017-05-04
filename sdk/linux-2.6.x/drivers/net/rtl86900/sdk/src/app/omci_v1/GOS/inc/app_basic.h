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
 * Purpose : Definition of OMCI generic OS define
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI generic OS define
 */

#ifndef __APP_BASIC_H__
#define __APP_BASIC_H__

#ifdef __cplusplus
extern "C" {
#endif


/* linux defintions */
#include <gos_linux.h>

/* basic type definitions */
#include <gos_type.h>

/* basic funciton definitions */
#include <gos_general.h>

/* GPON OMCI (G.984.4/G.988) definitions */
#include "omci_api.h"

/* MIB exported definitions */
#include <mib_table.h>

// other essential definitions
#include "omci_msgq.h"
#include "omci_util.h"
#include "omci_internal_api.h"
#include "omci_driver.h"
#include "omci_driver_api.h"
#include "omci_timer.h"
#include "omci_alarm.h"
#include "omci_pm.h"


#ifdef __cplusplus
}
#endif

#endif

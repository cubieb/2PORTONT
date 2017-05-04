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
 * Purpose : Definition of feature api layer related define
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1)
 */

#ifndef __FEATURE_MGMT_H__
#define __FEATURE_MGMT_H__

#include "gos_linux.h"
#include "omci_customize.h"
#include "feature_export_api.h"


#define FEATURE_MODULE_MAX_LENGTH	        256
#define FEATURE_MODULE_PATH                 "/lib/features/"
#define FEATURE_MODULE_INTERNAL_USER_PATH   "/lib/features/internal/"
#define FEATURE_MODULE_INTERNAL_KERNEL_PATH "/lib/modules/features/"

typedef struct feature_module_s
{
	unsigned int                moduleID;
	char                        moduleName[FEATURE_MODULE_MAX_LENGTH];
    void                        *p_handle;
    LIST_ENTRY(feature_module_s) entries;
}feature_module_t;

typedef struct feature_api_s
{
	unsigned int regApiId;
	unsigned int (*regCB)(va_list argp);
	unsigned int regModuleId;
}feature_api_t;

LIST_HEAD(featureHead, feature_module_s) featureRegModuleHead;

extern void fMgmtInit(omci_customer_feature_flag_t flag, unsigned int api_cnt_max);
extern void fMgmtInit_internal(unsigned int old_mask, unsigned int new_mask, unsigned int type);

extern void feature_reg_module_show(void);
extern void feature_reg_api_show(void);
extern unsigned int feature_api(unsigned int apiID, ...);
extern unsigned int feature_api_is_registered(unsigned int apiID);

extern int feature_info_get(unsigned int **);
extern char ** feature_prefix_info_get(void);
extern unsigned int feature_max_col_get(void);
#endif

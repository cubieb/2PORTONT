#ifndef _PRMT_IGD_H_
#define _PRMT_IGD_H_

#include <linux/config.h>
#include <config/autoconf.h>

#include <rtk/adsl_drv.h>
#include <rtk/options.h>
#include <rtk/sysconfig.h>
#include <rtk/utility.h>

#include "cwmp_porting.h"
#include "libcwmp.h"
#include "prmt_apply.h"
#ifdef CONFIG_MIDDLEWARE
#include <rtk/midwaredefs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tIGDLeaf[];
extern struct CWMP_NODE tIGDObject[];
extern struct CWMP_NODE tROOT[];
#ifdef CONFIG_MIDDLEWARE
extern struct CWMP_NODE mw_tROOT[];
#endif

int getIGD(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_IGD_H_*/

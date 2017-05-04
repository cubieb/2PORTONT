#ifndef _PRMT_DEVICE2_H_
#define _PRMT_DEVICE2_H_

#include <linux/config.h>
#include <config/autoconf.h>

#include <rtk/adsl_drv.h>
#include <rtk/options.h>
#include <rtk/sysconfig.h>
#include <rtk/utility.h>

#include "cwmp_porting.h"
#include "libcwmp.h"
#include "../prmt_apply.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tDeviceLeaf[];
extern struct CWMP_NODE tDeviceObject[];
extern struct CWMP_NODE device_tROOT[];

int getDevice(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_DEVICE2_H_*/

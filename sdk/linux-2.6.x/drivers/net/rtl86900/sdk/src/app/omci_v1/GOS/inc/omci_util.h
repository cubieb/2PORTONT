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
 * Purpose : Definition of OMCI utilities define
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI utilities define
 */

#ifndef __OMCI_UTIL_H__
#define __OMCI_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "gos_type.h"


extern int gLogLevel;

#define OMCI_LOG(loglevel,fmt, arg...) \
         do { if (gLogLevel >= loglevel) { printf(fmt, ##arg); printf("\n"); } } while (0);

#define OMCI_PRINT(fmt, arg...) \
         do { printf(fmt, ##arg); printf("\n"); } while (0);

#define OMCI_ERR_CHK(loglevel, expr, errCode)\
do {\
    if ((int32)(expr)) {\
		OMCI_LOG(loglevel, "%s()@%d" ,__FUNCTION__, __LINE__); \
        return errCode; \
    }\
} while (0)

#define STR(s) #s

#define IPADDR_PRINT "%d.%d.%d.%d"

#define IPADDR_PRINT_ARG(ip) \
    ((ip) & 0xFF000000) >> 24, ((ip) & 0xFF0000) >> 16, ((ip) & 0xFF00) >> 8, (ip) & 0xFF

#define IPADDRV6_PRINT "%X:%X:%X:%X:%X:%X:%X:%X"

#define IPADDRV6_PRINT_ARG(ipv6) \
    *((uint16 *)&ipv6[0]), *((uint16 *)&ipv6[2]), *((uint16 *)&ipv6[4]), *((uint16 *)&ipv6[6]), \
    *((uint16 *)&ipv6[8]), *((uint16 *)&ipv6[10]), *((uint16 *)&ipv6[12]), *((uint16 *)&ipv6[14])


void OMCI_TaskDelay(UINT32 num);
GOS_ERROR_CODE omci_util_is_digit(char *inputP);
GOS_ERROR_CODE omci_util_is_allZero(const unsigned char *p, unsigned int size);
GOS_ERROR_CODE omci_util_is_all0xFF(const unsigned char *p, unsigned int size);
double omci_util_log10(double z);

#define PON_IF          "pon0"
#define NIC_PROC_PATH   "/proc/rtl8686gmac/dev_port_mapping"
#define VCONFIG_CMD    "/bin/vconfig"
#define IFCONFIG_CMD   "/bin/ifconfig"
#define IPTABLES_CMD   "/sbin/iptables"
#define CMD_LEN        (256)

#ifdef __cplusplus
}
#endif

#endif

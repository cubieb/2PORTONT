#ifndef _CONFIG_RTK_H
#define _CONFIG_RTK_H 1

#include <rtk/options.h>

#ifdef _PRMT_X_CT_COM_DHCP_
/* CT-COM Enterprise ID, 0 is temprary value, but isc-dhcp cannot use 0 */
#define VENDOR_CT_ENT_NUM 0 /*20942*/
#define CTCOM_DHCPV6_OPT17 1
#endif

#endif //_CONFIG_RTK_H


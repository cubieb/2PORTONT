/*
 * $Id: snmp_debug.h,v 1.1.1.1 2003/08/18 05:40:24 kaohj Exp $
 */

#ifndef SNMP_DEBUG_H
#define SNMP_DEBUG_H

#if STDC_HEADERS
extern void snmplib_debug(int, char *,...);
#else
extern void snmplib_debug(va_alist);
#endif

#endif

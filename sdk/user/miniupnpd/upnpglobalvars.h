/* $Id: upnpglobalvars.h,v 1.14 2012/08/01 04:27:13 adsmt Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef __UPNPGLOBALVARS_H__
#define __UPNPGLOBALVARS_H__

#include <time.h>
#include "upnppermissions.h"
#include "miniupnpdtypes.h"
#include "config.h"

// Add by Mason Yu
#include "rtk/sysconfig.h"
#include "rtk/utility.h"

/* name of the network interface used to acces internet */
extern char ext_if_name[IFNAMSIZ];

/* file to store all leases */
extern const char * lease_file;

/* forced ip address to use for this interface
 * when NULL, getifaddr() is used */
extern char use_ext_ip_addr[];

/* parameters to return to upnp client when asked */
extern unsigned long downstream_bitrate;
extern unsigned long upstream_bitrate;

/* statup time */
extern time_t startup_time;

#if 0
/* log packets flag */
extern int logpackets;

/* use system uptime */
extern int sysuptime;

#ifdef ENABLE_NATPMP
extern int enablenatpmp;
#endif
#endif

/* runtime boolean flags */
extern int runtime_flags;
#define LOGPACKETSMASK		0x0001
#define SYSUPTIMEMASK		0x0002
#ifdef ENABLE_NATPMP
#define ENABLENATPMPMASK	0x0004
#endif
#define CHECKCLIENTIPMASK	0x0008
#define SECUREMODEMASK		0x0010

#define ENABLEUPNPMASK		0x0020

#define SETFLAG(mask)	(runtime_flags |= mask)
#define GETFLAG(mask)	(runtime_flags & mask)
#define CLEARFLAG(mask) (runtime_flags &= ~mask)

extern const char * pidfilename;

extern char uuidvalue[];

#define SERIALNUMBER_MAX_LEN (10)
extern char serialnumber[];

#define MODELNUMBER_MAX_LEN (48)
extern char modelnumber[];

#define PRESENTATIONURL_MAX_LEN (64)
extern char presentationurl[];

/* UPnP permission rules : */
extern struct upnpperm * upnppermlist;
extern unsigned int num_upnpperm;

#ifdef ENABLE_NATPMP
/* NAT-PMP */
extern unsigned int nextnatpmptoclean_timestamp;
extern unsigned short nextnatpmptoclean_eport;
extern unsigned short nextnatpmptoclean_proto;
#endif

/* queue */
extern const char * queue;

/* lan addresses */
/* MAX_LAN_ADDR : maximum number of interfaces
 * to listen to SSDP traffic */
#define MAX_LAN_ADDR (4)
extern int n_lan_addr;
extern struct lan_addr_s lan_addr[];
#endif

//cathy
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
#define CLOSE_SSDP
#endif

#ifdef CONFIG_TR_064
#define MANUFACTURER_NAME  "Realtek Semiconductor Corp."
#define MANUFACTURER_OUI  "00E04C"
#define SPEC_VERSION		"TR-064 v1.0"
#define HW_VERSION_STR		"8671x"
#define DESCRIPTION "Realtek ADSL Router"
#define PRODUCT_CLASS "00000000-00000000"

typedef enum {false, true} bool;

//DeviceConfig StateVariables
char A_ARG_TYPE_UUID[40];
char A_ARG_TYPE_Status[15];  // ChangesApplied or RebootRequired.

//Time StateVariables
#define DATETIME_LENGTH 25
bool DaylightSavingsUsed;
char DaylightSavingsStart[DATETIME_LENGTH];  // DateTime format: mm/dd/yyyy hh:mm:ss
char DaylightSavingsEnd[DATETIME_LENGTH];  // DateTime format: mm/dd/yyyy hh:mm:ss

typedef struct TimeZoneTable
{
	char *time_zone_name;
	short time_shift;
	short index;
} TimeZone;

//WANDevice
//WANCommonInterfaceConfig
bool EnabledForInternet;
extern bool IS_TR064_ENABLED;
#endif

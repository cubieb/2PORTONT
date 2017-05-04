/*
 * Copyright (C) 2010 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTEHR THAN AS AUTHORIZED UNDER 
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 * 
 * $Revision: 1.1 $ 
 * $Date: 2011/03/03 08:36:38 $
 *
 * Purpose : Multicast Snooping Function varible type declaration
 *
 * Feature : 
 *
 */

#ifndef __RTL_MULTICAST_TYPES_H__
#define __RTL_MULTICAST_TYPES_H__

#include <rtl_types.h>

#ifndef NULL
  #define NULL 0
#endif

#ifndef TRUE
  #define TRUE 1
#endif

#ifndef FALSE
  #define FALSE 0
#endif

#ifndef SUCCESS
  #define SUCCESS     0
#endif

#ifndef FAILED
  #define FAILED -1
#endif

#ifndef GOOD
  #define GOOD 0
#endif

#ifndef OK
  #define OK 0
#endif

#ifndef RIGHT
  #define RIGHT 0
#endif

#ifndef WRONG
  #define WRONG -1
#endif

#ifndef ERROR
  #define ERROR -1
#endif

#ifndef BAD
  #define BAD -1
#endif

#ifndef ENABLE
  #define ENABLE 1
#endif

#ifndef DISABLE
  #define DISABLE 0
#endif

#ifndef UNTAGGED
  #define UNTAGGED 0
#endif

#ifndef VLANTAGGED
  #define VLANTAGGED 1
#endif

#ifndef PRITAGGED
  #define PRITAGGED 2
#endif

#ifndef _RTL_TYPES_H

typedef unsigned int    uint32;
typedef int         int32;
typedef unsigned short  uint16;
typedef short           int16;
typedef unsigned char   uint8;
typedef char            int8;

#endif

#endif /* __RTL_MULTICAST_TYPES_H__ */


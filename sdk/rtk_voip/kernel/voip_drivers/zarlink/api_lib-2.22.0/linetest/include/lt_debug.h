/** \file lt_debug.h
 * lt_debug.h
 *
 * This file contains the configuration and compile time settings for
 * the Line Test API debug statements.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 10578 $
 * $LastChangedDate: 2012-11-02 15:28:36 -0500 (Fri, 02 Nov 2012) $
 */

#ifndef LT_DEBUG_H
#define LT_DEBUG_H

#include "lt_api.h"

#define LT_API_FORCE_DEBUG_ON 0
//#define LT_DEBUG
#ifdef LT_DEBUG

  #include <linux/kernel.h>
  /* LT-API Debug flags: try to keep flags all same length */
  #define LT_DBG_ERRR    (0x00000001L) /* Any error condition */
  #define LT_DBG_WARN    (0x00000002L) /* Any warning condition */
  #define LT_DBG_INFO    (0x00000004L) /* Un-categorized information */
  #define LT_DBG_FUNC    (0x00000008L) /* Any function */
  #define LT_DBG_ALL     (0x0000000FL) /* Include all debug */

  #define LT_DBG_DFLT   LT_DBG_ALL
//if ((flag) & (gLtDbgFlag | LT_API_FORCE_DEBUG_ON)) 
  #define LT_DOUT(flag, strings)    \
    {                               \
        printk(" [%s]", #flag);     \
        printk strings;            \
        printk("\n");               \
    }
#else
 #define LT_DOUT(flag, strings)
#endif

//EXTERN uint32 gLtDbgFlag; /* Decleration for the default flag variable */

#endif

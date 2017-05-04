/** \file lt_debug.c
 * lt_api.c
 *
 *  This file contains the implementation of LT_DEBUG MACRO.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#include "lt_api.h"
#include "lt_api_internal.h"
#ifdef LT_DEBUG_H
  #include "vp_api.h"
  #ifdef LT_DEBUG
    uint32 gLtDbgFlag = LT_DBG_DFLT;
  #endif
#endif

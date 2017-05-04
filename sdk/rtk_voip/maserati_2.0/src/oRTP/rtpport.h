/*
  The oRTP LinPhone RTP library intends to provide basics for a RTP stack.
  Copyright (C) 2001  Simon MORLAT simon.morlat@linphone.org

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


/* this file is responsible of the portability of the stack */

#ifndef RTPPORT_H
#define RTPPORT_H

#ifdef UGLIB_H
#define HAVE_GLIB
#endif

#define INT_TO_POINTER(truc)	((gpointer)(long)(truc))
#define POINTER_TO_INT(truc)	((int)(long)(truc))

#include <errno.h>
#include <sys/types.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>

#if defined(TIME_WITH_SYS_TIME)
#include <time.h>
#include <sys/time.h>
#elif defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#endif


#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>

#undef MIN
#define MIN(a,b) (((a)>(b)) ? (b) : (a))
#undef MAX
#define MAX(a,b) (((a)>(b)) ? (a) : (b))

#endif /*RTPPORT_H*/

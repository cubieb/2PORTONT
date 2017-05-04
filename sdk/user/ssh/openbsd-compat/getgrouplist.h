/* $Id: getgrouplist.h,v 1.1.1.1 2003/08/18 05:40:23 kaohj Exp $ */

#ifndef _BSD_GETGROUPLIST_H
#define _BSD_GETGROUPLIST_H

#include "config.h"

#ifndef HAVE_GETGROUPLIST

#include <grp.h>

int getgrouplist(const char *, gid_t, gid_t *, int *);

#endif

#endif

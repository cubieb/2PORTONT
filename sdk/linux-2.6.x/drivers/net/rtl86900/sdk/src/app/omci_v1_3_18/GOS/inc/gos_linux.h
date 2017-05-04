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
 * Purpose : Definition of OMCI generic OS linux define
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI generic OS linux define
 */

#ifndef __GOS_LINUX_H__
#define __GOS_LINUX_H__

#ifdef  __cplusplus
extern "C" {
#endif


#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <link.h>
#include <elf.h>
#include <termios.h>
#include <ctype.h>
#include <syslog.h>
#include <signal.h>
#include <dirent.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/sem.h>
#include <sys/mman.h>
#include <sys/syslog.h>
#include <sys/msg.h>
#include <sys/resource.h>
#include <sys/queue.h>
#include <sys/sysinfo.h>

#include <netinet/in.h>

#include <asm/types.h>
#include <asm/ioctls.h>
#include <asm/unistd.h>

#include <arpa/telnet.h>
#include <arpa/inet.h>
#include <sys/prctl.h>

#include <linux/fb.h>

#ifdef __cplusplus
}
#endif

#endif

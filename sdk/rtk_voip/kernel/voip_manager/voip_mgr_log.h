#ifndef __VOIP_MGR_LOG_H__
#define __VOIP_MGR_LOG_H__

//#include <linux/config.h>
#include "rtk_voip.h"

#ifdef CONFIG_RTK_VOIP_LOG_IOCTL
extern void log_ioctl( unsigned short cmd, void *user, unsigned short len, int is_user );
#else
static inline void log_ioctl( unsigned short cmd, void *user, unsigned short len, int is_user )
{
	// dummy 
}
#endif

#endif // __VOIP_MGR_LOG_H__


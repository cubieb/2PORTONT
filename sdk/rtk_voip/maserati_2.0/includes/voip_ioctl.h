#ifndef __VOIP_IOCTL__
#define __VOIP_IOCTL__

#define VOIP_MGR_IOCTL_DEV_NAME		"/dev/voip/mgr"

extern int g_VoIP_Mgr_FD;

#define VOIP_IOCTL(optid, varptr, vartype, qty, ioctl_fn) \
		{	\
			int ret;	\
						\
			if( g_VoIP_Mgr_FD < 0 )	\
				return -1;	\
			if( ( ret = ioctl_fn( g_VoIP_Mgr_FD, 	\
						VOIP_MGR_IOC_CMD( optid, sizeof(vartype)*qty ), 	\
						varptr ) ) < 0 ) 	\
			{							\
				return ret;				\
			}							\
		}

#ifdef __ECOS
extern int linux_wrapper_chrdev_ioctl( int handle, unsigned int cmd, unsigned long arg );
#define SETSOCKOPT(optid, varptr, vartype, qty)	\
			VOIP_IOCTL( optid, ( unsigned long )varptr, vartype, qty, linux_wrapper_chrdev_ioctl )
#else
#define SETSOCKOPT(optid, varptr, vartype, qty)	\
			VOIP_IOCTL( optid, varptr, vartype, qty, ioctl );
#endif

#endif /* __VOIP_IOCTL__ */


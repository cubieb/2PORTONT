#ifndef __SCIOCTL__
#define __SCIOCTL__

#include "pcsc_t0.h"

#define MY_MACIG 'G'
#define SC_IOCTL_GETSW	_IOR(MY_MACIG, 0, int)
#define SC_IOCTL_WRITE	_IOW(MY_MACIG, 1, int)
#define SC_IOCTL_GET_IMEI	_IOR(MY_MACIG, 2, int)
#define SC_IOCTL_GET_SN	_IOR(MY_MACIG, 3, int)
#define SC_IOCTL_SETCMD	_IOW(MY_MACIG, 4, T0cmd)
#define SC_IOCTL_CHECKCARD	_IOR(MY_MACIG, 5, int)
#define SC_IOCTL_GETSYSTEMSTATUS	_IOR(MY_MACIG, 6, int)
#define SC_IOCTL_CHECKATRPPS	_IOR(MY_MACIG, 7, int)
#define SC_IOCTL_DOWARMRESET	_IO(MY_MACIG, 8)

#endif

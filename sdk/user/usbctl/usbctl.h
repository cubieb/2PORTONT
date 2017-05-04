/*--------------------------------
		Includes
---------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <stdint.h>
#include <string.h>

#include <linux/usbdevice_fs.h>
#include <dirent.h>

/*--------------------------------
		Definitions
---------------------------------*/

#define USBCTL "usbctl: "
#define USBPATH "/proc/bus/usb"
#define USBHUBPATH1 "/proc/bus/usb/001/001"
#define USBHUBPATH2 "/proc/bus/usb/002/001"
#define USBHUBPATH3 "/proc/bus/usb/003/001"

/* command for ioctl */
#define USBDEVFS_USB3PHY			_IOR('U', 122, struct usbdevfs_usb3phy)
#define USBDEVFS_CTRL_STAGE			_IOR('U', 123, struct usbdevfs_ctrl_stage)
#define USBDEVFS_USBPHY				_IOR('U', 124, struct usbdevfs_usbphy)	//100421 cathy, support phy register access
#define USBDEVFS_DBG				_IOR('U', 125, unsigned int)	//100421 cathy, print message if error
#define USBDEVFS_HCTESTPKT			_IOR('U', 126, unsigned char)		//100518 cathy, host sends test packet
#define USBDEVFS_MEM				_IOR('U', 127, struct usbdevfs_mem)

/* USB3 PHY register access */
struct usbdevfs_usb3phy {
	unsigned char is_read;	//read or write register
	unsigned char page;	//page 0 or 1
	unsigned char reg;
	unsigned short value;
};

/* USB PHY register access */
struct usbdevfs_usbphy {
	unsigned char is_read;	//read or write register
	unsigned char port;	//usb phy port 0 or port 1
	unsigned char reg;
	unsigned char value;
};

/* memory access */
struct usbdevfs_mem {
	unsigned char is_read;	//read or write register
	unsigned int addr;	//usb phy port 0 or port 1
	int len;
	unsigned int value;
};

/* specific stage of control transfer */
struct usbdevfs_ctrl_stage {
	struct usbdevfs_ctrltransfer ctrl;
	unsigned int stage;
};

/* stage of control transfer */
enum CTRL_STAGE {
	SETUP_STAGE = 1,
	DATA_STAGE,
	STATUS_STAGE
};

/* type of memory or phy register access */
enum access_type {
	IS_WRITE = 0,
	IS_READ,
	IGNORE
};

/* item of command table */
struct cmd_item{
	char  display_name[128];
	int (*fn)(char **var1, unsigned char var2);
	int	argc;
	enum access_type rw_type;
};



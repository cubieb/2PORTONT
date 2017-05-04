#ifndef _AIPC_DEV_H_
#define _AIPC_DEV_H_

#include "aipc_global.h"
#include "soc_type.h"
#include "aipc_ioctl.h"

#define DEVICE_NAME                         "aipc_dev"

#ifndef AIPC_DEV_MAJOR
#define AIPC_DEV_MAJOR 0   /* dynamic major by default */
#endif

#ifndef AIPC_DEV_NR_DEVS
#define AIPC_DEV_NR_DEVS 1    /* aipc_dev0 */
#endif

typedef struct {
	unsigned long size;		/* amount of data stored here 	*/
	struct semaphore sem;	/* mutual exclusion semaphore 	*/
	struct cdev cdev;		/* Char device structure		*/
}aipc_dev_t;

typedef struct {
	int write_file;
	int write_word;
	int bitop_and;
	int bitop_or;
	int bitop_xor;
	int bitop_not;
}aipc_ioc_t;

#endif /* _AIPC_DEV_H_ */


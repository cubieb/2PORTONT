#ifndef __SCIF__
#define __SCIF__

#ifdef __KERNEL__
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
//#include <linux/atomic.h>
#endif

#include "pcsc_atr.h"

#define DEVICE_NAME             "simcard"
#define MODULE_NAME             "simcard"
#define DEV_NAME_LEN 32
#define SIM_IRQ	16
#define IO_MODE_EN				0xBB023018
#define SC_CD_EN				(1<<21)
#define REG32(reg)	(*(volatile unsigned int *)((unsigned int)reg))

struct config_t;


typedef struct dev_node_t {
        char                    dev_name[DEV_NAME_LEN];
        u_char                  major, minor;
        struct dev_node_t       *next;
} dev_node_t;

struct sc_dynids {
        spinlock_t              lock;
        struct list_head        list;
};

struct sc_device {
        /* the hardware "function" device; certain subdevices can
         * share one hardware "function" device. */
        u8                      func;
        struct config_t*        function_config;

        struct list_head        socket_device_list;

        /* deprecated, will be cleaned up soon */
        dev_node_t              *dev_node;
        u_int                   open;

        /* Is the device suspended, or in the process of
         * being removed? */
        u16                     suspended:1;
        u16                     _removed:1;

        /* Flags whether io, irq, win configurations were
         * requested, and whether the configuration is "locked" */
        u16                     _irq:1;
        u16                     _io:1;
        u16                     _win:4;
        u16                     _locked:1;

        /* Flag whether a "fuzzy" func_id based match is
         * allowed. */
        u16                     allow_func_id_match:1;

        /* information about this device */
        u16                     has_manf_id:1;
        u16                     has_card_id:1;
        u16                     has_func_id:1;

        u16                     reserved:3;

        u8                      func_id;
        u16                     manf_id;
        u16                     card_id;

        struct device           dev;

        /* device driver wanted by cardmgr */
        struct sc_driver *  cardmgr;

        /* data private to drivers */
        void                    *priv;
};

struct sc_driver {
        int (*probe)            (struct sc_device *dev);
        void (*remove)          (struct sc_device *dev);

        int (*suspend)          (struct sc_device *dev);
        int (*resume)           (struct sc_device *dev);

        struct module           *owner;
        struct device_driver    drv;
        struct sc_dynids    dynids;
};

struct sc_device_driver {
	const char *name;

	int (*reader_probe) (struct sc_device *udev);
	void (*reader_detach) (struct sc_device *udev);

	//int (*suspend) (struct usb_device *udev, pm_message_t message);
	//int (*resume) (struct usb_device *udev, pm_message_t message);
	//struct usbdrv_wrap drvwrap;
	//	unsigned int supports_autosuspend:1;
};

typedef struct _SCStruct {
	unsigned char isInited;
	unsigned char isInserted;
	unsigned char getATRPPS;	// bit 0 ATR, bit 1 PPS
	unsigned short etu;

	unsigned char open;
	int status1;
	int status2;
	
	struct cdev cdev;
	unsigned char major;
	unsigned char minor;
	
	struct semaphore sem;
	
	ATRStruct *gATR;
}SCStruct;

void addSession(void);
void delSession(void);
char getTrasnferFinish(void);
char checkCardIsPlugged(void);


#endif

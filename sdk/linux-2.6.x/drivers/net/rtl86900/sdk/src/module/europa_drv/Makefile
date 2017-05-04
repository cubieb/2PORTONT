#######################################################3#
#	Build Realtek EUROPA modules			#
#########################################################
# add this line if this module used others compiler flag
include $(APOLLODIR)/Compiler_Flag

MODULE_NAME = europa_drv

# EUROPA_DRIVER_VERSION 0x000208 /* 0.2.8 */
MODULE_VERSION = 0x000208 

VERSION=$(shell printf "%d" $(MODULE_VERSION))
VERSION_MAJOR = ($(VERSION)>>16)&255
VERSION_MINOR = ($(VERSION)>>8)&255
VERSION_PATCH = ($(VERSION))&255

PWD = $(shell pwd)
#KDIR = /lib/modules/`uname -r`/build

# for sdk/src/module is built kernel module not build-in
#EXTRA_CFLAGS += $(FLAGS)

# add this line if this module used below compiler flag 
EXTRA_CFLAGS+=$(PON_CFLAGS)

# define driver version
EXTRA_CFLAGS+=-DEUROPA_DRIVER_VERSION=$(MODULE_VERSION)

#ifneq ($(CONFIG_EUROPA_MODULE),)
#EXTRA_CFLAGS += -DCONFIG_EUROPA_MODULE
#endif
 
obj-$(CONFIG_EUROPA) := $(MODULE_NAME).o

#$(MODULE_NAME)-objs := europa_main.o init.o interrupt.o ddmi.o utility.o reg_acc.o laser_drv.o apc_enable.o tx_enable.o
$(MODULE_NAME)-objs := europa_main.o init.o interrupt.o ddmi.o rtl8290_api.o

#for sdk/src/module is built kernel module not build-in
all: clean
	@echo "   BUILD module $(MODULE_NAME)"
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) ARCH=rlx CROSS_COMPILE=$(CROSS_COMPILE)
	cp -af $(MODULE_NAME).ko ./$(MODULE_NAME).ko_$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)
 
install:
	cp -af $(MODULE_NAME).ko $(ROMFSDIR)/lib/modules

clean:
	rm -rf *.o *~ core .depend .*.cmd *.mod.c .tmp_versions *.symvers *.ko

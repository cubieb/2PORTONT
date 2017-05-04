#########################################################
#	Build Realtek Factory modules			#
#########################################################

include $(APOLLODIR)/Compiler_Flag

MODULE_NAME = ft2
KDIR = 
PWD = $(shell pwd)

SDKDIR=$(APOLLODIR)/sdk
SDK_INCLUDE   = $(SDKDIR)/include
SYS_INCLUDE   = $(SDKDIR)/system/include
DRV_INCLUDE   = $(SDKDIR)/system/linux
PR_INCLUDE    = $(SDKDIR)/src/module/pkt_redirect

EXTRA_CFLAGS+=$(PON_CFLAGS)
FLAGS+=-I$(SDK_INCLUDE) -I$(SYS_INCLUDE) -I$(DRV_INCLUDE) -I$(PR_INCLUDE) -I$(KERNEL_DIR)/drivers/net

EXTRA_CFLAGS+=$(FLAGS)

ifeq ($(CONFIG_MODULE_BUILD_IN),y)
BUILD_MODE=y
else
BUILD_MODE=m
endif

obj-$(BUILD_MODE) := $(MODULE_NAME).o

ifeq ($(CONFIG_SDK_RTL9601B),y)
$(MODULE_NAME)-objs:= operation_life_test.o

obj-$(BUILD_MODE) += qc_test.o
qc-objs:= qc_test.o
else
$(MODULE_NAME)-objs:= factory_test.o
endif

all:
	@echo "   BUILD module $(MODULE_NAME), $(FLAGS)"
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) ARCH=rlx CROSS_COMPILE=$(CROSS_COMPILE)
 

install:
	cp -af $(MODULE_NAME).ko $(ROMFSDIR)/lib/modules
ifeq ($(CONFIG_SDK_RTL9601B),y)
	cp -af SDKDIR/rtk.etc/ol_test.sh $(ROMFSDIR)/etc/ol_test.sh
	cp -af SDKDIR/rtk.etc/9601b_test.sh $(ROMFSDIR)/etc/9601b_test.sh
	cp -af qc_test.ko $(ROMFSDIR)/lib/modules
endif

clean:
	rm -rf *.o *~ core .depend .*.cmd *.mod.c .tmp_versions *.symvers *.ko 

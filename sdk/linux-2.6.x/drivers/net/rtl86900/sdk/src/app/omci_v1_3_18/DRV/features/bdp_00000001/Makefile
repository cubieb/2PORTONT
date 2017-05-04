#########################################################
#	Build Realtek OMCI modules		#
#########################################################
include $(APOLLODIR)/Compiler_Flag

MODULE_NAME=bdp_00000001
PWD=$(shell pwd)
SDKDIR=$(APOLLODIR)/sdk
SDK_INCLUDE   = $(SDKDIR)/include
SYS_INCLUDE   = $(SDKDIR)/system/include
DRV_INCLUDE   = $(SDKDIR)/system/linux
OMCIDIR=$(SDKDIR)/src/app/omci_v1
OMCISRCDIR=$(OMCIDIR)/OMCI/src
OMCIDRVDIR=$(OMCIDIR)/DRV
OMCI_API_INCLUDE = $(OMCIDIR)/API
MODULEDIR=$(SDKDIR)/src/module

KBUILD_EXTRA_SYMBOLS += $(OMCIDRVDIR)/Module.symvers

EXTRA_CFLAGS += -Wall -Wframe-larger-than=2048
EXTRA_CFLAGS += -I$(SDK_INCLUDE) \
		-I$(DRV_INCLUDE) \
		-I$(SYS_INCLUDE) \
		-I$(MODULEDIR) \
		-I$(OMCIDIR) \
		-I$(OMCISRCDIR) \
		-I$(OMCIDRVDIR) \
		-I$(OMCI_API_INCLUDE) \
		-DCONFIG_SDK_KERNEL_LINUX

obj-m := $(MODULE_NAME).o

$(MODULE_NAME)-objs:= omci_tls_4095.o

all: clean
	@echo "   BUILD module $(MODULE_NAME)"
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) ARCH=rlx CROSS_COMPILE=$(CROSS_COMPILE) || exit 1

install:
	@if [ ! -d $(ROMFSDIR)/lib/modules/features ]; \
        then \
                mkdir $(ROMFSDIR)/lib/modules/features ; \
        fi
	cp -af $(MODULE_NAME).ko $(ROMFSDIR)/lib/modules/features

clean:
	rm -rf *.o *~ core .depend .*.cmd *.mod.c .tmp_versions *.symvers *.ko modules.order
	rm -rf $(ROMFSDIR)/lib/modules/features

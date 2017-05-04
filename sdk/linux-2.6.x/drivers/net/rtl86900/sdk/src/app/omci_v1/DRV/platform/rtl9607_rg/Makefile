#########################################################
#	Build Realtek OMCI modules		#
#########################################################
RGDRV_RELEASE_VERSION			= \"1.8.1.0\"
ifeq ($(CONFIG_APOLLO_ROMEDRIVER),y)
include $(APOLLODIR)/Compiler_Flag

MODULE_NAME=pf_rg
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
RG_INCLUDE = $(APOLLODIR)/romeDriver
ROOT_INCLUDE = $(ROOTDIR)/include

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
		-I$(RG_INCLUDE) \
		-I$(NETDIR)  \
		-I$(ROOT_INCLUDE) \
		-DCONFIG_SDK_KERNEL_LINUX

ifdef RGDRV_RELEASE_VERSION
EXTRA_CFLAGS += -DCONFIG_DRV_RELEASE_VERSION="$(RGDRV_RELEASE_VERSION)"
endif

obj-m := $(MODULE_NAME).o

$(MODULE_NAME)-objs:= omci_pf_rtl9607.o

all: clean
	@echo "   BUILD module $(MODULE_NAME)"
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) ARCH=rlx CROSS_COMPILE=$(CROSS_COMPILE) || exit 1

install:
	@if [ ! -d $(ROMFSDIR)/lib/modules ]; \
        then \
                mkdir $(ROMFSDIR)/lib/modules ; \
        fi
	$(STRIP) $(MODULE_NAME).ko
	cp -af $(MODULE_NAME).ko $(ROMFSDIR)/lib/modules/

clean:
	rm -rf *.o *~ core .depend .*.cmd *.mod.c .tmp_versions *.symvers *.ko modules.order
endif

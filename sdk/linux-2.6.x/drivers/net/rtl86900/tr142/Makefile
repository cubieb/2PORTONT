srctree=$(KDIR)

EXTRA_CFLAGS += -Wall -Werror
EXTRA_CFLAGS += $(PON_CFLAGS)
EXTRA_CFLAGS += -I$(APOLLODIR)/romeDriver
EXTRA_CFLAGS += -I$(srctree)/drivers/net
EXTRA_CFLAGS += -DCONFIG_SDK_KERNEL_LINUX

EXTRA_CFLAGS += -I$(APOLLODIR)/tr142/include
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/include
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/system/include
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/src/app/diag_rtk/include
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/src/app/omci_v1/API

MODULE_NAME=rtk_tr142
obj-m := $(MODULE_NAME).o

all:
	$(MAKE) -C $(KDIR) M=$(APOLLODIR)/tr142 ARCH=rlx CROSS_COMPILE=$(CROSS_COMPILE) \
	modules

install:
	cp -af $(MODULE_NAME).ko $(ROMFSDIR)/lib/modules
	cp -af $(MODULE_NAME).sh $(ROMFSDIR)/etc

clean:
	rm -rf *.o *~ core .depend .*.cmd *.mod.c .tmp_versions *.symvers *.ko


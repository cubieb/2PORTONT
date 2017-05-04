# 
# Makefile for the Linux network (ethercard) device drivers.
#

ifeq ($(CONFIG_OPENWRT_RG),y)
obj-y+=sdk/
else
obj-$(CONFIG_MODULE_BUILD_IN)+=sdk/

ifeq ($(CONFIG_MODULE_BUILD_IN),m) 
ifeq ($(CONFIG_RTK_L34_ENABLE),y)
obj-m +=romeDriver/
endif
else
obj-$(CONFIG_RTK_L34_ENABLE)+=romeDriver/
endif
endif

all:
	$(MAKE) -C sdk all;
	$(MAKE) -C romeDriver all;
	$(MAKE) -C tr142 all;

install:
	$(MAKE) -C sdk install;
	$(MAKE) -C romeDriver install; 
	$(MAKE) -C tr142 install;


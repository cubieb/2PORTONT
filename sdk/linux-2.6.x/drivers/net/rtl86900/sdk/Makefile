#
# Makefile for the Linux Wireless network device drivers.
#

include $(APOLLODIR)/Compiler_Flag

SDK_RTCOREDIR = $(SDKDIR)/system/linux/rtcore
SDK_RTKDIR    = $(SDKDIR)/src
SDK_UTDIR     = $(SDKDIR)/unittest
SDK_RTDRVDIR  = $(SDKDIR)/system/linux/rtdrv
SDK_INCLUDE   = $(SDKDIR)/include
SYS_INCLUDE   = $(SDKDIR)/system/include
DRV_INCLUDE   = $(SDKDIR)/system/linux
UT_INCLUDE    = $(SDKDIR)/unittest
SDK_MODULE    = $(SDKDIR)/src/module
SDK_ETCDIR    = $(SDKDIR)/rtk.etc

ifeq ($(CONFIG_DIAGSHELL_MODE),CONFIG_DIAGSHELL_RTK)
DIAGSHELL_DIR := $(SDKDIR)/src/app/diag_rtk
endif

ifeq ($(CONFIG_DIAGSHELL_MODE),CONFIG_DIAGSHELL_RAW)
DIAGSHELL_DIR := $(SDKDIR)/src/app/diag
endif


CONFIG_SDK_KERNEL_LINUX_KERNEL_MODE=y


export SDKDIR SDK_INCLUDE SYS_INCLUDE DRV_INCLUDE UT_INCLUDE
export CONFIG_SDK_KERNEL_LINUX_KERNEL_MODE

ifneq ($(CONFIG_XDSL_RG_DIAGSHELL),y)
obj-y += src/
obj-y += unittest/
endif
obj-y += system/linux/rtcore/
obj-y += system/linux/rtdrv/
ifeq ($(CONFIG_SDK_MODULES),y)
obj-y += src/module/
endif


all:
	$(MAKE) -C $(SDK_MODULE)    SRC_DIR=$(SDK_MODULE)       all;

install:
	$(MAKE) -C $(SDK_MODULE)    SRC_DIR=$(SDK_MODULE)       install;


ifeq ($(CONFIG_RTK_L34_ENABLE),y)
DEF += -D CONFIG_RTK_L34_ENABLE
endif

ifeq ($(CONFIG_XDSL_RG_DIAGSHELL),y)
DEF += -D CONFIG_XDSL_RG_DIAGSHELL
endif

ifeq ($(CONFIG_SDK_RTL9601B),y)
DEF += -D CONFIG_SDK_RTL9601B
endif

ifeq ($(CONFIG_GPON_FEATURE),y)
DEF += -D CONFIG_GPON_FEATURE
endif

ifeq ($(CONFIG_EPON_FEATURE),y)
DEF += -D CONFIG_EPON_FEATURE
endif

ifeq ($(CONFIG_RLDP_FEATURE),y)
DEF += -D CONFIG_RLDP_FEATURE
endif

ifeq ($(CONFIG_REDUCED_DIAG),y)
DEF += -D CONFIG_REDUCED_DIAG
endif

ifeq ($(CONFIG_EUROPA_FEATURE),y)
DEF += -D CONFIG_EUROPA_FEATURE
endif

clean:
	$(MAKE) -C $(SDK_RTKDIR) clean
	$(MAKE) -C $(SDK_RTCOREDIR) clean
	$(MAKE) -C $(SDK_UTDIR) clean
	rm -rf *.o


shell_tree:
	rm -f $(DIAGSHELL_DIR)/../diag_rtk/src/parser/*.o
	cd $(DIAGSHELL_DIR)/config; \
        ./script/mk_parser.py $(DEF) diag_shell.cli -fd ../include/flag/ -f cmd_flag.h; \
        mv ../config/cparser_tree.c ../src/parser/cparser_tree.c; \
        mv ../config/cparser_tree.h ../include/parser/cparser_tree.h; \
        cd - ;


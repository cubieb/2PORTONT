#
# Makefile for the fastpath modules on top of IPv4.
#

obj-y += fastpath.o
obj-$(CONFIG_RTL867X_MCAST_FAST_PATH) += mc_fastpath.o
obj-$(CONFIG_RTL867X_PACKET_PROCESSOR) += fastpath_pp.o

ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
fastpath-objs := fastpath_core.o16 fp_common.o16
else
fastpath-objs := fastpath_core.o fp_common.o
endif
mc_fastpath-objs += mc_fastpath_core.o

#rsdk-1.3.2 uses gcc 3.4.6
ifneq ($(GCC_VERSION),3.4.6)
ifdef CONFIG_RTL8672_MIPS16_IPV4
EXTRA_CFLAGS += -Wall -O2
else
EXTRA_CFLAGS += -Wall -Werror -O2
endif
endif

#include $(TOPDIR)/Rules.make

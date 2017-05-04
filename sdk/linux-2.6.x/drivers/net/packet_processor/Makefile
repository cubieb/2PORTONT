obj-y += packet_processor.o rtl8672pp_extsw.o

packet_processor-objs := rtl8672PacketProcessor.o rtl8672_tblAsicDrv.o icModel.o icTest_ALE.o icModel_ringController.o rtl_glue.o rtl8672_tblDrv.o  pp_debug.o

all: check_obj first_rule

check_obj:
	file $(obj-y) | grep Intel | awk -F: '{print $$1};' | xargs rm -f --

EXTRA_CFLAGS += -Wall -ggdb -Werror
EXTRA_CFLAGS += -DCONFIG_RTL867X -DRTL867X_MODEL_KERNEL -DRTL865X_OVER_KERNEL -DRTL865X_OVER_LINUX
EXTRA_CFLAGS += -I8672 -I. -I../rtl867x/ -I../cle/ -I../


#include $(TOPDIR)/Rules.make

#
# Makefile for DSL IPC
#
ifdef DIR_LINUX
include $(DIR_LINUX)/.config
else
ifdef APP_DIR
include $(APP_DIR)/.config
endif
endif

EXTRA_CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/char/aipc/include

ifeq ($(CONFIG_RTL8686_IPC_PROCESSOR), 0)
EXTRA_CFLAGS += -D_AIPC_CPU_
else ifeq ($(CONFIG_RTL8686_IPC_PROCESSOR), 1)
EXTRA_CFLAGS += -D_AIPC_DSP_
endif

ifeq ($(CONFIG_RTL8686_IPC_DSL_IPC),y)
obj-y += aipc_dsl_ctrl.o
obj-y += aipc_dsl_eoc.o
obj-y += aipc_dsl_event.o
endif

clean:
	rm -fr *.o .*.cmd modules.order *~


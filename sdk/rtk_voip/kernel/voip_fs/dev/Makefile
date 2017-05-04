obj-y = voip_dev_core.o data_dump_dev.o voip_dev_proc.o
obj-$(CONFIG_RTK_VOIP_CIRCULAR_LOG_DEV) += circular_log.o

EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_manager
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_ipc
EXTRA_CFLAGS += -I$(DIR_VOIP)/include

include $(DIR_VOIP)/rules.mak


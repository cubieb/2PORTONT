DIR_RTLASIC = $(DIR_LINUX)/drivers/net/rtl819x/

rtl8192se-objs += 8190n_tx.o \
		8190n_rx.o \
		8190n_osdep.o \
		8190n_util.o \
		8190n_hw.o \
		8190n_file.o \
		8190n_ioctl.o \
		beacon.o \
		8190n_proc.o \
		tx.o \
		rtl_rc.o

obj-y += rtl8192se.o

EXTRA_CFLAGS := -I$(DIR_RTLASIC) -I$(DIR_BOARD) -DRTL865X_OVER_KERNEL -DRTL865X_OVER_LINUX -DCONFIG_RTL8192SE -DNEW_MAC80211_DRV
EXTRA_CFLAGS += -G 0

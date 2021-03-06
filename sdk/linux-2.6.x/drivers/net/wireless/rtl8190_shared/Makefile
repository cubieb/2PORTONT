#
# drivers/net/rtl8190
#
# Makefile for the Linux Wireless network device drivers.
#
#Add mips16 Support
O_TARGET := rtl8190.o

DIR_RTLASIC = $(DIR_LINUX)/drivers/net/rtl819x/
ifdef CONFIG_RTL865X_KERNEL_MIPS16_WLAN
	#CFLAGS_8190n_tx.o	= -mips16
	#CFLAGS_8190n_rx.o	= -mips16
	#CFLAGS_8190n_osdep.o	= -mips16
	#CFLAGS_8190n_sme.o	= -mips16
	#CFLAGS_8190n_util.o	= -mips16
	#CFLAGS_8190n_hw.o	= -mips16
	#CFLAGS_8190n_ioctl.o	= -mips16
	#CFLAGS_8190n_security.o= -mips16
	#CFLAGS_8190n_tkip.o	= -mips16
	#CFLAGS_8190n_aes.o	= -mips16
	#CFLAGS_8190n_cam.o	= -mips16
	#CFLAGS_8190n_proc.o	= -mips16
	#CFLAGS_8190n_br_ext.o	= -mips16
	#CFLAGS_8190n_eeprom.o	= -mips16
	#CFLAGS_8190n_mp.o	= -mips16
	#CFLAGS_8190n_psk.o	= -mips16
	CFLAGS_1x_kmsm_aes.o	= -mips16
	CFLAGS_1x_kmsm_hmac.o	= -mips16
	CFLAGS_1x_md5c.o	= -mips16
	CFLAGS_1x_rc4.o	= -mips16
	CFLAGS_8190n_mib.o	= -mips16
	CFLAGS_8190n_dmem.o	= -mips16
	CFLAGS_romeperf.o	= -mips16
endif
obj-$(CONFIG_RTL8190) += rtl8190.o
obj-$(CONFIG_RTL8192SE) += rtl8190.o

EXTRA_CFLAGS += -DCONFIG_RTL8192SE -DCONFIG_RTL8196B

rtl8190-objs += 819x_pci.o\
		8190n_file.o\
		8190n_hw.o\
		tx.o\
		8190n_rx.o\

ifeq ($(CONFIG_WLAN_80211),y)
EXTRA_CFLAGS += -DNEW_MAC80211_DRV

rtl8190-objs += mac80211/rtl_tx.o \
		mac80211/rtl_osdep.o \
		mac80211/rtl_util.o \
		mac80211/rtl_ioctl.o \
		mac80211/rtl_beacon.o \
		mac80211/rtl_rc.o
else  #ifneq ($(CONFIG_WLAN_80211),y)
#subdir-$(CONFIG_RTL8192SE)      += rtl8192Pci
#add for MESH
ifeq ($(CONFIG_MESH_ENABLE),y)
        EXTRA_CFLAGS += -DCONFIG_RTK_MESH #-DMESH_USE_METRICOP
        ifeq ($(CONFIG_11S_TEST_MODE),y)
        EXTRA_CFLAGS += -D_11s_TEST_MODE_
        endif

        obj-mesh = mesh_ext/mesh_proc.o\
        mesh_ext/mesh_route.o\
        mesh_ext/mesh_rx.o\
        mesh_ext/mesh_sme.o\
        mesh_ext/mesh_security.o\
        mesh_ext/mesh_tx.o\
        mesh_ext/mesh_util.o\
        mesh_ext/mesh_11kv.o\
        mesh_ext/hash_table.o
endif

ifdef CONFIG_RTL_WAPI_SUPPORT
EXTRA_CFLAGS += -O3
endif

obj-rtl8190-$(CONFIG_RTL8190) = rtl_misc.o #8190n_cam.o
#obj-$(CONFIG_RTL_WAPI_SUPPORT) += SMS4.o wapi_wai.o wapiCrypto.o

ifdef CONFIG_RTL_WAPI_MODULE_SUPPORT
obj-m += wapi.o
wapi-objs += wapi_entry.o wapi_wai.o
obj-$(CONFIG_RTL_WAPI_SUPPORT) += wapiCrypto.o
else
obj-$(CONFIG_RTL_WAPI_SUPPORT) += wapi_wai.o wapiCrypto.o
endif

obj-$(CONFIG_RTL_DYNAMIC_IRAM_MAPPING_FOR_WAPI) += switch_iram.o

rtl8190-objs += 8190n_tx.o\
        8190n_osdep.o\
        8190n_sme.o\
        8190n_util.o\
        8190n_ioctl.o\
        8190n_security.o\
        8190n_tkip.o\
        8190n_aes.o\
        8190n_proc.o\
        8190n_br_ext.o\
        8190n_eeprom.o\
        8190n_mp.o\
        8190n_psk.o\
        1x_kmsm_aes.o\
        1x_kmsm_hmac.o\
        1x_md5c.o\
        1x_rc4.o\
        8190n_mib.o\
        $(obj-rtl8190-y)\
        8190n_dmem.o\
        romeperf.o\
        $(obj-mesh)
endif	#ifeq ($(CONFIG_WLAN_80211),y)

EXTRA_CFLAGS += -I$(DIR_RTLASIC)
EXTRA_CFLAGS += -I$(DIR_BOARD)
EXTRA_CFLAGS += -DRTL865X_OVER_KERNEL
EXTRA_CFLAGS += -DRTL865X_OVER_LINUX
EXTRA_CFLAGS += -G 0

EXTRA_AFLAGS += $(EXTRA_CFLAGS)

#
##Build C code for TXT or BIN file
#
.SECONDARY: $(SRCS_TXT) $(SRCS_BIN)
SRCS_TXT = $(addprefix data_, $(notdir $(patsubst %.txt,%.c,$(wildcard data/*.txt))))
SRCS_BIN = $(addprefix data_, $(notdir $(patsubst %.bin,%.c,$(wildcard data/*.bin))))

data_%.c: data/%.txt
	./bin2c.pl $(basename $@) < $< > $@
data_%.c: data/%.bin
	./bin2c.pl $(basename $@) < $< > $@
# create our target
all: $(SRCS_TXT) $(SRCS_BIN) $(O_TARGET)

clean:
	rm *.o mac80211/*.o
#
#include $(TOPDIR)/Rules.make

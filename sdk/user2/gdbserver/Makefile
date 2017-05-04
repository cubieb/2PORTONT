.EXPORT_ALL_VARIABLES:
#
# Realtek Semiconductor Corp.
#
# Viller Hsiao (villerhsiao@realtek.com)
# Mar. 02, 2010
#

DIR_GDB = gdb-6.8
ifeq ($(CONFIG_GDB_SERVER_72),y)
DIR_GDB = gdb-7.2
endif

CFLAGS += -I$(DIR_TMPFS)/include 
#CFLAGS += -DCONFIG_CPU_HAS_FPR
LDFLAGS = -static

CROSS_COMPILE ?= /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-110915/bin/rsdk-linux-
CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
RANLIB=$(CROSS_COMPILE)ranlib
STRIP=$(CROSS_COMPILE)strip

TARGET_CONFIG = CC=$(CROSS_COMPILE)gcc \
		LD=$(CROSS_COMPILE)gcc \
		AR=$(CROSS_COMPILE)ar \
		RANLIB=$(CROSS_COMPILE)ranlib

all:
	mkdir -p build
	mkdir -p build/libtermcap
	cd build/libtermcap; \
	  [ -f Makefile ] || \
		$(TARGET_CONFIG) \
		CFLAGS="-I$(DIR_TMPFS)/include" \
		../../$(DIR_GDB)/libtermcap/configure \
			--host=mips-linux \
			--target=mips-linux \
			--disable-werror; \
	make
	cd build; \
	  [ -f Makefile ] || \
		$(TARGET_CONFIG) \
		CFLAGS="-I$(DIR_TMPFS)/include" \
		LDFLAGS="-static" \
		../$(DIR_GDB)/configure \
			--host=mips-linux \
			--target=mips-linux \
			--disable-werror; \
	make

romfs:
	$(ROMFSINST) build/gdb/gdbserver/gdbserver /bin/gdbserver

clean:
	rm -rf build

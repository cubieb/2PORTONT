PCSC_INSTALL_PREFIX=$(shell pwd)/tmp
PCSC_ARCH=mips
PCSC_CROSS_COMPILER=$(CROSS_COMPILE)

CCID_FLAG=--prefix=$(PCSC_INSTALL_PREFIX) CC=$(PCSC_CROSS_COMPILER)gcc --host=$(PCSC_ARCH)-linux --enable-static --enable-shared CFLAGS=-I$(PCSC_INSTALL_PREFIX)/include/PCSC LDFLAGS=-L$(PCSC_INSTALL_PREFIX)/lib PCSC_LIBS=-lpcsclite LIBUSB_CFLAGS="-I$(PCSC_INSTALL_PREFIX)/include/libusb-1.0" LIBUSB_LIBS="-lusb-1.0"  --enable-usbdropdir=/usr/local/lib/pcsc/drivers
LIBUSB_FLAG=--prefix=$(PCSC_INSTALL_PREFIX) CC=$(PCSC_CROSS_COMPILER)gcc --host=$(PCSC_ARCH)-linux --enable-static --enable-shared --enable-udev=no
PCSCLITE_FLAG=--prefix=$(PCSC_INSTALL_PREFIX) CC=$(PCSC_CROSS_COMPILER)gcc --host=$(PCSC_ARCH)-linux --disable-libudev --enable-static --enable-shared CFLAGS=-I$(PCSC_INSTALL_PREFIX)/include/libusb-1.0 LDFLAGS=-L$(PCSC_INSTALL_PREFIX)/lib --enable-usbdropdir=/usr/local/lib/pcsc/drivers LIBUSB_LIBS="-lusb-1.0" --enable-confdir=/etc/reader.conf.d
PCSCTOOL_FLAG=PCSC_CFLAGS="-I$(PCSC_INSTALL_PREFIX)/include/PCSC" PCSC_LDLIBS="-L$(PCSC_INSTALL_PREFIX)/lib -L$(ROOTDIR)/romfs/lib -lpthread -lpcsclite"

CCID_DIR=ccid-1.4.15
LIBUSB_DIR=libusb-1.0.18
PCSCLITE_DIR=pcsc-lite-1.8.11
PCSCTOOL_DIR=pcsc-tools-1.4.22

export PKG_CONFIG_PATH+=$(PCSC_INSTALL_PREFIX)/lib/pkgconfig

-include $(ROOTDIR)/include/linux/config.h

.PHONY: all libusb pcsclite ccid pcsc-tools

all: libusb pcsclite ccid pcsc-tools

romfs:
	$(ROMFSINST) ./tmp/sbin/pcscd /bin/pcscd
	$(ROMFSINST) ./tmp/sbin/testpcsc /bin/testpcsc
	$(ROMFSINST) ./tmp/lib/libusb-1.0.so.0.1.0 /lib/libusb-1.0.so.0.1.0
	ln -fs libusb-1.0.so.0.1.0 $(ROOTDIR)/romfs/lib/libusb-1.0.so.0
	ln -fs libusb-1.0.so.0.1.0 $(ROOTDIR)/romfs/lib/libusb-1.0.so
	$(ROMFSINST) ./tmp/lib/libpcsclite.so.1.0.0 /lib/libpcsclite.so.1.0.0
	ln -fs libpcsclite.so.1.0.0 $(ROOTDIR)/romfs/lib/libpcsclite.so.1
	ln -fs libpcsclite.so.1.0.0 $(ROOTDIR)/romfs/lib/libpcsclite.so
	$(MAKE) -C $(CCID_DIR) install DESTDIR=$(ROOTDIR)/romfs
	$(ROMFSINST) $(CCID_DIR)/examples/scardcontrol /bin/scardcontrol
	$(MAKE) -C $(PCSCTOOL_DIR) install DESTDIR=$(ROOTDIR)/romfs

ccid:
	cd $(CCID_DIR) && ./configure $(CCID_FLAG)
	$(MAKE) -C $(CCID_DIR)
#	$(MAKE) -C $(CCID_DIR) install DESTDIR=$(ROOTDIR)/romfs

libusb:
	cd $(LIBUSB_DIR) && ./configure $(LIBUSB_FLAG)
	$(MAKE) -C $(LIBUSB_DIR)
	$(MAKE) -C $(LIBUSB_DIR) install

pcsclite:
	cd $(PCSCLITE_DIR) && ./configure $(PCSCLITE_FLAG)
	$(MAKE) -C $(PCSCLITE_DIR)
	$(MAKE) -C $(PCSCLITE_DIR) install

pcsc-tools:
	cd $(PCSCTOOL_DIR)
	$(MAKE) -C $(PCSCTOOL_DIR) $(PCSCTOOL_FLAG)
#	$(MAKE) -C $(PCSCTOOL_DIR) install DESTDIR=$(ROOTDIR)/romfs

clean:
	-$(MAKE) -C $(CCID_DIR) clean
	-$(MAKE) -C $(LIBUSB_DIR) clean
	-$(MAKE) -C $(PCSCLITE_DIR) clean
	-$(MAKE) -C $(PCSCTOOL_DIR) clean
	$(RM) -r $(PCSC_INSTALL_PREFIX)/*

distclean:
	-$(MAKE) -C $(CCID_DIR) distclean
	-$(MAKE) -C $(LIBUSB_DIR) distclean
	-$(MAKE) -C $(PCSCLITE_DIR) distclean
	-$(MAKE) -C $(PCSCTOOL_DIR) distclean
	$(RM) -r $(PCSC_INSTALL_PREFIX)/*

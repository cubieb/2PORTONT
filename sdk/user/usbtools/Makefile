
.EXPORT_ALL_VARIABLES:

USBTOOLS_ROOT=$(shell pwd)
#LIBUSB_DIR=libusb-0.1.12
LIBUSB_DIR=libusb-1.0.9
#LIBUSB_DIR=libusb-1.0.18
#USBMODESWITCH_DIR=usb_modeswitch-1.0.2
#USBMODESWITCH_DIR=usb-modeswitch-1.1.3
USBMODESWITCH_DIR=usb-modeswitch-2.2.1
#USBUTILS_DIR=usbutils-0.84
#USBUTILS_DIR=usbutils-0.91
USBUTILS_DIR=usbutils-003
MNET_DIR=mnet
COMGT_DIR=comgt
HUBCTRL_DIR=hubctrl
PRE_INSTALL_DIR=$(USBTOOLS_ROOT)/preinstall

CFLAGS+= -I$(PRE_INSTALL_DIR)/include
LIBUSB_CFLAGS = -I$(PRE_INSTALL_DIR)/include
LIBUSB_CFLAGS += -I$(PRE_INSTALL_DIR)/include/libusb-1.0
LIBUSB_LIBS = -L$(PRE_INSTALL_DIR)/lib -lusb-1.0 -lpthread

all:
	mkdir -p $(PRE_INSTALL_DIR)
ifeq ($(CONFIG_USER_USBTOOLS_LIBUSB),y)
	#LIBUSB
	if [ -d $(LIBUSB_DIR) ]; then \
		cd $(LIBUSB_DIR); \
		if [ ! -f config.h ]; then \
			./configure --host=mips-linux --prefix=$(PRE_INSTALL_DIR) --exec-prefix=$(PRE_INSTALL_DIR) --enable-static --enable-shared --enable-udev=no; \
		fi; \
		$(MAKE); \
		$(MAKE) install; \
	fi
endif
ifeq ($(CONFIG_USER_3G_USBTOOLS_USBMODESWITCH),y)
	#USBMODESWITCH
	if [ -d $(USBMODESWITCH_DIR) ]; then \
		$(MAKE) -C $(USBMODESWITCH_DIR); \
		$(MAKE) -C $(USBMODESWITCH_DIR) install; \
	fi
endif
ifeq ($(CONFIG_USER_USBTOOLS_LSUSB),y)
	#USBUTILS
	if [ -d $(USBUTILS_DIR) ]; then \
		#cd $(USBUTILS_DIR); \
		#if [ ! -f config.h ]; then \
		#	./configure --host=mips-linux --prefix=$(PRE_INSTALL_DIR) --exec-prefix=$(PRE_INSTALL_DIR) --libdir=$(PRE_INSTALL_DIR)/lib --disable-zlib --disable-udev; \
		#fi; \
		$(MAKE) -C $(USBUTILS_DIR); \
		#$(MAKE) install; \
	fi
endif
ifeq ($(CONFIG_USER_3G_USBTOOLS_MNET),y)
	# MNET
	if [ -d $(MNET_DIR) ]; then \
		$(MAKE) -C $(MNET_DIR); \
	fi	
endif
ifeq ($(CONFIG_USER_3G_USBTOOLS_COMGT),y)
	# COMGT
	if [ -d $(COMGT_DIR) ]; then \
		$(MAKE) -C $(COMGT_DIR); \
	fi	
endif
ifeq ($(CONFIG_USER_3G_USBTOOLS_HUBCTRL),y)
	# Hub ctrl
	if [ -d $(HUBCTRL_DIR) ]; then \
		$(MAKE) -C $(HUBCTRL_DIR); \
	fi	
endif
	
clean:
#ifeq ($(CONFIG_USER_USBTOOLS_LSUSB),y)
	if [ -d $(USBUTILS_DIR) ]; then \
		$(MAKE) -C $(USBUTILS_DIR) clean; \
	fi
#endif
#ifeq ($(CONFIG_USER_3G_USBTOOLS_USBMODESWITCH),y)
	if [ -d $(USBMODESWITCH_DIR) ]; then \
		$(MAKE) -C $(USBMODESWITCH_DIR) clean; \
	fi
#endif
#ifeq ($(CONFIG_USER_USBTOOLS_LIBUSB),y)
	if [ -d $(LIBUSB_DIR) ]; then \
		$(MAKE) -C $(LIBUSB_DIR) clean; \
	fi
#endif
#ifeq ($(CONFIG_USER_3G_USBTOOLS_MNET),y)
	if [ -d $(MNET_DIR) ]; then \
		$(MAKE) -C $(MNET_DIR) clean; \
	fi
#endif
#ifeq ($(CONFIG_USER_3G_USBTOOLS_COMGT),y)
	if [ -d $(COMGT_DIR) ]; then \
		$(MAKE) -C $(COMGT_DIR) clean; \
	fi
#endif
#ifeq ($(CONFIG_USER_3G_USBTOOLS_HUBCTRL),y)
	if [ -d $(HUBCTRL_DIR) ]; then \
		$(MAKE) -C $(HUBCTRL_DIR) clean; \
	fi
#endif
	rm -rf $(PRE_INSTALL_DIR)


romfs:
ifeq ($(CONFIG_USER_3G_USBTOOLS_USBMODESWITCH),y)
	$(MAKE) -C $(USBMODESWITCH_DIR)	romfs
#	cp -f $(PRE_INSTALL_DIR)/etc/usb_modeswitch.conf $(USBTOOLS_ROOT)/../../romfs/etc
#	cp -f $(PRE_INSTALL_DIR)/usr/sbin/usb_modeswitch $(USBTOOLS_ROOT)/../../romfs/bin
endif
ifeq ($(CONFIG_USER_USBTOOLS_LSUSB),y)
	$(MAKE) -C $(USBUTILS_DIR) romfs
	#cp -f $(PRE_INSTALL_DIR)/share/usb.ids $(USBTOOLS_ROOT)/../../romfs/etc
	#cp -f $(PRE_INSTALL_DIR)/sbin/lsusb $(USBTOOLS_ROOT)/../../romfs/bin
	#cp -f $(PRE_INSTALL_DIR)/bin/lsusb $(USBTOOLS_ROOT)/../../romfs/bin
	cp -f /opt/toolchain_mips_nofpu/mips-linux-uclibc/lib/libgcc_s.so.1 $(USBTOOLS_ROOT)/../../romfs/lib/
endif
ifeq ($(CONFIG_USER_USBTOOLS_LIBUSB),y)
	#cp -f $(PRE_INSTALL_DIR)/lib/libusb-0.1.so.4.4.4 $(USBTOOLS_ROOT)/../../romfs/lib/libusb-0.1.so.4
	cp -f $(PRE_INSTALL_DIR)/lib/libusb-1.0.so.0.1.0 $(USBTOOLS_ROOT)/../../romfs/lib/libusb-1.0.so.0
endif
ifeq ($(CONFIG_USER_3G_USBTOOLS_MNET),y)
	$(MAKE) -C $(MNET_DIR)	romfs
	#cp -f $(PRE_INSTALL_DIR)/bin/mnet $(USBTOOLS_ROOT)/../../romfs/bin
endif
ifeq ($(CONFIG_USER_3G_USBTOOLS_COMGT),y)
	$(MAKE) -C $(COMGT_DIR)	romfs
	#cp -f $(PRE_INSTALL_DIR)/bin/comgt $(USBTOOLS_ROOT)/../../romfs/bin
endif
ifeq ($(CONFIG_USER_3G_USBTOOLS_HUBCTRL),y)
	$(MAKE) -C $(HUBCTRL_DIR)	romfs
	#cp -f $(PRE_INSTALL_DIR)/bin/hubctrl $(USBTOOLS_ROOT)/../../romfs/bin
endif

rcX:
ifeq ($(CONFIG_DEFAULTS_REALTEK_LUNA),y)
ifeq ($(CONFIG_USER_3G_USBTOOLS_MNET),y)
	$(MAKE) -C $(MNET_DIR)	rcX
	#cp -f $(PRE_INSTALL_DIR)/bin/mnet $(USBTOOLS_ROOT)/../../romfs/bin
endif
endif


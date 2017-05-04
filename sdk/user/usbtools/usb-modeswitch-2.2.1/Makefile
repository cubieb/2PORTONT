CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
RANLIB = $(CROSS_COMPILE)ranlib
STRIP = $(CROSS_COMPILE)strip
SSTRIP = $(CROSS_COMPILE)sstrip
DESTDIR     = $(shell pwd)/../preinstall
PREFIX      = $(DESTDIR)/usr
ETCDIR      = $(DESTDIR)/etc
SBINDIR     = $(PREFIX)/sbin

PROG        = usb_modeswitch
VERS        = 2.2.1
RM          = /bin/rm -f
OBJS        = usb_modeswitch.c
LIBUSB		= libusb
LIBUSB_V	= 1.0.9
LIB_USB		= ../$(LIBUSB)-$(LIBUSB_V)

CFLAGS += -Os
CFLAGS += $(EXTRA_CFLAGS)
#CFLAGS += -I $(LIB_USB) -I$(LIB_USB)/libusb
CFLAGS += -I $(DESTDIR)/include -I$(LIB_USB)/libusb

all: $(PROG).o
	make -C $(LIB_USB)
	$(CC) $(PROG).o -L$(DESTDIR)/lib $(CFLAGS) -lusb-1.0 -lpthread -o $(PROG)

romfs:
	$(ROMFSINST) /bin/$(PROG)
	$(ROMFSINST)	/etc/usb_modeswitch.conf
	$(ROMFSINST)	-s usb_modeswitch.conf /etc/usb_modeswitch.conf.1
	$(ROMFSINST)	/etc/usb_modeswitch.d


install: all
		mkdir -p $(SBINDIR)
		install ./usb_modeswitch $(SBINDIR)
		mkdir -p $(ETCDIR)
		install ./usb_modeswitch.conf $(ETCDIR)

clean:
	$(RM) -f $(EXEC) *.elf *.o *.a
	$(RM) $(PROG)


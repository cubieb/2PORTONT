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
VERS        = 1.1.3
RM          = /bin/rm -f
OBJS        = usb_modeswitch.c
LIBUSB		= libusb
LIBUSB_V	= 0.1.12
LIB_USB		= ../$(LIBUSB)-$(LIBUSB_V)

CFLAGS += -Os
CFLAGS += $(EXTRA_CFLAGS)
#CFLAGS += -I $(LIB_USB)
CFLAGS += -I $(DESTDIR)/include

all: $(PROG).o
	make -C $(LIB_USB)
	$(CC) $(PROG).o -L$(DESTDIR)/lib -lusb -o $(PROG)

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


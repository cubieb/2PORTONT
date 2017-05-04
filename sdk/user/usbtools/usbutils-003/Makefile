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

PROG        = lsusb
PROGO		= devtree.o lsusb-t.o names.o usbmisc.o lsusb.o 
RM          = /bin/rm -f
OBJS        = devtree.c lsusb-t.c names.c usbmisc.c lsusb.c 
LIBUSB		= libusb
LIBUSB_V	= 1.0.9
LIB_USB		= ../$(LIBUSB)-$(LIBUSB_V)

CFLAGS += -Os
CFLAGS += $(EXTRA_CFLAGS)
CFLAGS += -DDATADIR=\"etc\"
CFLAGS += -I./
#CFLAGS += -I $(LIB_USB) -I$(LIB_USB)/libusb
CFLAGS += -I $(DESTDIR)/include -I$(LIB_USB)/libusb
#CLFAGS +=  

all: $(PROG)

$(PROG):$(PROGO)
	make -C $(LIB_USB)
	$(CC) $(LDFLASGS) -o $@ $(PROGO) -L$(DESTDIR)/lib $(CFLAGS) -lusb-1.0 -lpthread 

romfs:
	$(ROMFSINST) /bin/$(PROG)
	$(ROMFSINST) /etc/usb.ids

clean:
	-rm -f $(PROG) *.elf *.o


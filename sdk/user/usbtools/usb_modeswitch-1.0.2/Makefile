DESTDIR     = $(shell pwd)/../preinstall
PROG        = usb_modeswitch
VERS        = 0.9.7
#STRIP	    = strip
#CC          = gcc
CCFLAGS     = -I $(DESTDIR)/include -L $(DESTDIR)/lib -l usb
RM          = /bin/rm -f
OBJS        = usb_modeswitch.c
PREFIX	    = $(DESTDIR)/usr
ETCDIR	    = $(DESTDIR)/etc
SBINDIR	    = $(PREFIX)/sbin

.PHONY:     clean
all:        $(PROG)
$(PROG):    $(OBJS)
	      $(CC) $(CCFLAGS) -o $(PROG) $(OBJS)
	      $(STRIP) $(PROG)

clean:
	      $(RM) ./usb_modeswitch

install: all
	mkdir -p $(SBINDIR)
	install ./usb_modeswitch $(SBINDIR)
	mkdir -p $(ETCDIR)
	install ./usb_modeswitch.conf $(ETCDIR)

uninstall: clean
	$(RM) /usr/sbin/usb_modeswitch
	$(RM) /etc/usb_modeswitch.conf

.PHONY:     clean install uninstall

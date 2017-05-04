# $Id: Makefile,v 1.10 2012/08/21 11:16:17 krammer Exp $
# MiniUPnP project
# http://miniupnp.free.fr/
# Author : Thomas Bernard
# for use with GNU Make
# To install use :
# $ PREFIX=/dummyinstalldir make -f Makefile.linux install
# or :
# $ INSTALLPREFIX=/usr/local make -f Makefile.linux install
# or :
# $ make -f Makefile.linux install
#
#CFLAGS = -Wall -O -D_GNU_SOURCE -g -DDEBUG
#CFLAGS = -Wall -Os -D_GNU_SOURCE
CFLAGS += -Wall -O -g
#CC = gcc

ifdef CONFIG_USER_IPTABLES_IPTABLES_1_4_18
NETFILTEROBJS = netfilter/iptcrdr-1_4_18.o
CFLAGS += -I$(ROOTDIR)/user/iptables-1.4.18/include
INCLIBS = -L../iptables-1.4.18/libiptc -L../boa/src/LINUX
else
NETFILTEROBJS = netfilter/iptcrdr.o
CFLAGS += -I$(ROOTDIR)/user/iptables/include
INCLIBS = -L../iptables/libiptc -L../boa/src/LINUX
endif

RM = rm -f
INSTALL = install

INSTALLPREFIX ?= $(PREFIX)/usr
SBININSTALLDIR = $(INSTALLPREFIX)/sbin
ETCINSTALLDIR = $(PREFIX)/etc/miniupnpd

ifeq ($(CONFIG_USE_SHARED_LIB), y)
BASEOBJS = miniupnpd.o upnpdescgen.o \
		   upnpredirect.o getifaddr.o daemonize.o upnpglobalvars.o \
		   options.o upnppermissions.o minissdp.o natpmp.o soapmethods.o upnphttpdesc.o

BASEOBJS2 = upnphttp.o upnpsoap.o upnpreplyparse.o minixml.o
else
BASEOBJS = miniupnpd.o upnphttp.o upnpdescgen.o upnpsoap.o \
           upnpreplyparse.o minixml.o \
		   upnpredirect.o getifaddr.o daemonize.o upnpglobalvars.o \
		   options.o upnppermissions.o minissdp.o natpmp.o soapmethods.o upnphttpdesc.o
endif

LNXOBJS = linux/getifstats.o


ALLOBJS = $(BASEOBJS) $(LNXOBJS) $(NETFILTEROBJS)

LIBS = -liptc

TESTUPNPDESCGENOBJS = testupnpdescgen.o upnpdescgen.o

EXECUTABLES = miniupnpd

ifeq ($(CONFIG_USE_SHARED_LIB),y)
UPNPDIR		= ../mini_upnp
UPNPINC		= $(UPNPDIR)
CFLAGS += -I$(UPNPINC)
ifdef CONFIG_USE_RSDK_WRAPPER
UPNPLIB = -lmini_upnp
else #CONFIG_USE_RSDK_WRAPPER
UPNPLIB = $(UPNPDIR)/mini_upnp.so
endif #CONFIG_USE_RSDK_WRAPPER
LIBS += $(UPNPLIB)
##MINIUPNP_LIB = miniupnp.so
endif

.PHONY:	all clean install depend genuuid

all:	$(EXECUTABLES)

clean:
	$(RM) $(BASEOBJS) $(LNXOBJS) $(NETFILTEROBJS) $(EXECUTABLES)
	$(RM) testupnpdescgen.o testgetifstats.o
	$(RM) testupnppermissions.o
	$(RM) miniupnpdctl.o

install:	miniupnpd genuuid
	$(INSTALL) -d $(SBININSTALLDIR)
	$(INSTALL) miniupnpd $(SBININSTALLDIR)
	$(INSTALL) -d $(ETCINSTALLDIR)
	$(INSTALL) netfilter/iptables_init.sh $(ETCINSTALLDIR)
	$(INSTALL) netfilter/iptables_removeall.sh $(ETCINSTALLDIR)
	$(INSTALL) --mode=0644 miniupnpd.conf $(ETCINSTALLDIR)
	$(INSTALL) -d $(PREFIX)/etc/init.d
	$(INSTALL) linux/miniupnpd.init.d.script $(PREFIX)/etc/init.d/miniupnpd

# genuuid is using the uuidgen CLI tool which is part of libuuid
# from the e2fsprogs
genuuid:
	sed -i -e "s/^uuid=[-0-9a-f]*/uuid=`(genuuid||uuidgen) 2>/dev/null`/" miniupnpd.conf

#miniupnpd:	$(BASEOBJS) $(LNXOBJS) $(NETFILTEROBJS) $(LIBS)
ifeq ($(CONFIG_USE_SHARED_LIB),y)
miniupnpd:	$(BASEOBJS) $(LNXOBJS) $(NETFILTEROBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCLIBS) $(LIBMIB) $(LIBVOIP) -o $@ $(LDLIBS$(LDLIBS-$(@))) $(BASEOBJS) $(LNXOBJS) $(NETFILTEROBJS) -liptc $(UPNPLIB)
else
miniupnpd:	$(BASEOBJS) $(LNXOBJS) $(NETFILTEROBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCLIBS) $(LIBMIB) $(LIBVOIP) -o $@ $(LDLIBS$(LDLIBS-$(@))) $(BASEOBJS) $(LNXOBJS) $(NETFILTEROBJS) -liptc
endif

testupnpdescgen:	$(TESTUPNPDESCGENOBJS)

testgetifstats:	testgetifstats.o linux/getifstats.o

testupnppermissions:	testupnppermissions.o upnppermissions.o

miniupnpdctl:	miniupnpdctl.o

config.h:	genconfig.sh
	./genconfig.sh

depend:	config.h
	makedepend -f$(MAKEFILE_LIST) -Y \
	$(ALLOBJS:.o=.c) $(TESTUPNPDESCGENOBJS:.o=.c) \
	testgetifstats.c 2>/dev/null

romfs:
	$(ROMFSINST)  miniupnpd /bin/miniupnpd

# DO NOT DELETE

#miniupnpd.o: config.h upnpglobalvars.h upnppermissions.h miniupnpdtypes.h
#miniupnpd.o: upnphttp.h upnpdescgen.h miniupnpdpath.h getifaddr.h upnpsoap.h
#miniupnpd.o: options.h minissdp.h upnpredirect.h daemonize.h natpmp.h
#upnphttp.o: upnphttp.h config.h upnpdescgen.h miniupnpdpath.h upnpsoap.h
#upnpdescgen.o: upnpdescgen.h miniupnpdpath.h upnpglobalvars.h
#upnpdescgen.o: upnppermissions.h config.h miniupnpdtypes.h upnpdescstrings.h
#upnpsoap.o: upnpglobalvars.h upnppermissions.h config.h miniupnpdtypes.h
#upnpsoap.o: upnphttp.h upnpsoap.h upnpreplyparse.h upnpredirect.h getifaddr.h
#upnpsoap.o: getifstats.h
#upnpreplyparse.o: upnpreplyparse.h minixml.h
#minixml.o: minixml.h
#upnpredirect.o: config.h upnpredirect.h upnpglobalvars.h upnppermissions.h
#upnpredirect.o: miniupnpdtypes.h netfilter/iptcrdr.h commonrdr.h
#getifaddr.o: getifaddr.h
#daemonize.o: daemonize.h config.h
#upnpglobalvars.o: upnpglobalvars.h upnppermissions.h config.h
#upnpglobalvars.o: miniupnpdtypes.h
#options.o: options.h config.h upnppermissions.h upnpglobalvars.h
#options.o: miniupnpdtypes.h
#upnppermissions.o: config.h upnppermissions.h
#minissdp.o: config.h upnpdescstrings.h miniupnpdpath.h upnphttp.h
#minissdp.o: upnpglobalvars.h upnppermissions.h miniupnpdtypes.h minissdp.h
#natpmp.o: config.h natpmp.h upnpglobalvars.h upnppermissions.h
#natpmp.o: miniupnpdtypes.h getifaddr.h upnpredirect.h commonrdr.h
#linux/getifstats.o: getifstats.h config.h
#netfilter/iptcrdr.o: netfilter/iptcrdr.h commonrdr.h config.h
#testupnpdescgen.o: upnpdescgen.h
#upnpdescgen.o: upnpdescgen.h miniupnpdpath.h upnpglobalvars.h
#upnpdescgen.o: upnppermissions.h config.h miniupnpdtypes.h upnpdescstrings.h
#testgetifstats.o: getifstats.h

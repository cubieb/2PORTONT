# udhcp makefile

prefix=/usr
SBINDIR=/sbin
USRSBINDIR=${prefix}/sbin
USRBINDIR=${prefix}/bin
USRSHAREDIR=${prefix}/share

# Uncomment this to get a shared binary. Call as udhcpd for the server,
# and udhcpc for the client
#COMBINED_BINARY=1

# Uncomment this for extra output and to compile with debugging symbols
#DEBUG=1

# Uncomment this to output messages to syslog, otherwise, messages go to stdout
CFLAGS += -DSYSLOG

#CROSS_COMPILE=mips-uclibc-
#CROSS_COMPILE=/usr/mips-linux-uclibc/bin/mips-uclibc-
#CROSS_COMPILE=arm-uclibc-
#CC = $(CROSS_COMPILE)gcc
LD = $(CC)
INSTALL = install

VER := 0.9.8


OBJS_SHARED = options.o socket.o packet.o pidfile.o
ifdef CONFIG_USER_DHCP_SERVER
DHCPD_OBJS = dhcpd.o arpping.o files.o leases.o serverpacket.o dhcrelay2.o
endif
DHCPC_OBJS = dhcpc.o clientpacket.o script.o arpping.o

ifdef COMBINED_BINARY
ifdef CONFIG_USER_DHCP_SERVER
EXEC1 = udhcpd
endif
OBJS1 = $(DHCPD_OBJS) $(DHCPC_OBJS) $(OBJS_SHARED) frontend.o
CFLAGS += -DCOMBINED_BINARY
else
ifdef CONFIG_USER_DHCP_SERVER
EXEC1 = udhcpd
endif
OBJS1 = $(DHCPD_OBJS) $(OBJS_SHARED)

EXEC2 = udhcpc
OBJS2 = $(DHCPC_OBJS) $(OBJS_SHARED)
ifdef CONFIG_LUNA
#ccwei
CFLAGS += -I$(ROOTDIR)/user/boa/src/LINUX/
endif
endif

EXEC3 = dumpleases
OBJS3 = dumpleases.o

BOOT_PROGRAMS = udhcpc
DAEMONS = udhcpd
COMMANDS = dumpleases

ifdef SYSLOG
CFLAGS += -DSYSLOG
endif

CFLAGS += -W -Wall -Wstrict-prototypes -DVERSION='"$(VER)"'

ifdef DEBUG
CFLAGS += -g -DDEBUG
STRIP=true
else
CFLAGS += -Os -fomit-frame-pointer
STRIP=$(CROSS_COMPILE)strip
endif

ifdef CONFIG_USER_CWMP_TR069
CFLAGS += -D_CWMP_TR111_
endif
LDFLAGS+=$(LIBMIB)
ifdef CONFIG_USER_RTK_VOIP
PWD := $(shell pwd)
TOPDIR := $(shell pwd)/../..
VOIP_APP = $(TOPDIR)/user/rtk_voip
CFLAGS += -DVOIP_SUPPORT -I$(VOIP_APP)/includes
LDFLAGS+=-L../rtk_voip/voip_manager -lvoip_manager
VOIPLIB=$(VOIP_APP)/flash/libvoip_flash_client.a
ifdef CONFIG_E8B
LDFLAGS+=$(LIBSSL)
CFLAGS+=$(INCSSL) -I../rtk_voip/includes
endif
endif
CFLAGS += -DLINUX_MAJOR=MAJORVERSION -DLINUX_MINOR=MINORVERSION

all: $(EXEC1) $(EXEC2) $(EXEC3)
#	$(STRIP) --remove-section=.note --remove-section=.comment $(EXEC1) $(EXEC2) $(EXEC3)

$(OBJS1) $(OBJS2) $(OBJS3): *.h Makefile
$(EXEC1) $(EXEC2) $(EXEC3): Makefile

.c.o:
	$(CC) -c $(CFLAGS) $<
	
$(EXEC1): $(OBJS1)
	$(LD) $(LDFLAGS) $(OBJS1) -o $(EXEC1) $(VOIPLIB)

$(EXEC2): $(OBJS2)
	$(LD) $(LDFLAGS) $(OBJS2) -o $(EXEC2) $(VOIPLIB)

$(EXEC3): $(OBJS3)
	$(LD) $(LDFLAGS) $(OBJS3) -o $(EXEC3) $(VOIPLIB)


install: all

	$(INSTALL) $(DAEMONS) $(USRSBINDIR)
	$(INSTALL) $(COMMANDS) $(USRBINDIR)
ifdef COMBINED_BINARY
	ln -sf $(USRSBINDIR)/$(DAEMONS) $(SBINDIR)/$(BOOT_PROGRAMS)
else
	$(INSTALL) $(BOOT_PROGRAMS) $(SBINDIR)
endif
	mkdir -p $(USRSHAREDIR)/udhcpc
	for name in bound deconfig renew script ; do \
		$(INSTALL) samples/sample.$$name \
			$(USRSHAREDIR)/udhcpc/default.$$name ; \
	done
	mkdir -p $(USRSHAREDIR)/man/man1
	$(INSTALL) dumpleases.1 $(USRSHAREDIR)/man/man1
	mkdir -p $(USRSHAREDIR)/man/man5
	$(INSTALL) udhcpd.conf.5 $(USRSHAREDIR)/man/man5
	mkdir -p $(USRSHAREDIR)/man/man8
	$(INSTALL) udhcpc.8 udhcpd.8 $(USRSHAREDIR)/man/man8

clean:
	-rm -f udhcpd udhcpc dumpleases *.o core

romfs:
	$(ROMFSINST) -e CONFIG_USER_DHCP_SERVER udhcpd /bin/udhcpd
	$(ROMFSINST) udhcpc /bin/udhcpc
#	$(ROMFSINST) ../scripts/dhcpc.sh /bin/dhcpc.sh
#	$(ROMFSINST) ../scripts/dhcpd.sh /bin/dhcpd.sh
#	mkdir $(ROMFSDIR)/bin/udhcpc-scripts
#	$(ROMFSINST) ../scripts/eth0.sh /bin/udhcpc-scripts/eth0.sh

ifdef CONFIG_USER_DHCP_SERVER
$(DHCPD_OBJS): $(ROOTDIR)/include/rtk/options.h $(ROOTDIR)/config/autoconf.h
endif


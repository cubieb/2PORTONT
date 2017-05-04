CFLAGS += -Wall -W -O2
LDFLAGS += $(LIBMIB)

# TODO: check dependency, it will cause compile error if not link to voip
ifeq ($(CONFIG_USER_RTK_VOIP),y)
LDFLAGS += $(LIBVOIP)
endif

ifdef CONFIG_USER_FON
COPTS += -DHAVE_WHITELIST
endif

OBJS = cache.o rfc1035.o util.o option.o forward.o isc.o network.o \
       dnsmasq.o dhcp.o lease.o rfc2131.o netlink.o dbus.o bpf.o \
       helper.o tftp.o log.o

.c.o:
	$(CC) $(CFLAGS) $(COPTS) $(DBUS_MINOR) $(I18N) $(DBUS_CFLAGS) $(SUNOS_VER) $(RPM_OPT_FLAGS) -c $<

dnsmasq : $(OBJS)
	$(CC) $(LDFLAGS) -o $@  $(OBJS) $(DBUS_LIBS) $(SUNOS_LIBS) $(LIBS)

dnsmasq.pot : $(OBJS:.o=.c) dnsmasq.h config.h
	xgettext -d dnsmasq --foreign-user --keyword=_ -o dnsmasq.pot -i $(OBJS:.o=.c)

%.mo : ../po/%.po dnsmasq.pot
	msgmerge -o - ../po/$*.po dnsmasq.pot | msgfmt -o $*.mo -

romfs:
	$(ROMFSINST) /bin/dnsmasq
	$(ROMFSINST) -s /var/resolv.conf  /etc/resolv.conf
	$(ROMFSINST) ../dnsmasq.conf /etc/dnsmasq.conf


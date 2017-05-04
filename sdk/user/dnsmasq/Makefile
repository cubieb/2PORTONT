OBJS = cache.o rfc1035.o dhcp.o dnsmasq.o

ifdef CONFIG_IPV6
CFLAGS += -D__USE_BSD=1 -DNO_FORK=1 -DDEFAULT_CACHE_SIZE=20 -DHAVE_IPV6
else
CFLAGS += -D__USE_BSD=1 -DNO_FORK=1 -DDEFAULT_CACHE_SIZE=20 -DNO_IPV6
endif

all : dnsmasq

clean :
	rm -f *~ $(OBJS) dnsmasq dnsmasq.gdb

.c.o: dnsmasq.h config.h
	$(CC) -c $(CFLAGS) $*.c 


dnsmasq : $(OBJS) dnsmasq.h config.h
	$(CC) $(LDFLAGS) -o $@  $(OBJS) $(LDLIBS$(LDLIBS-$(@))) $(LIBMIB) $(LIBVOIP)

install : dnsmasq
	install -d $(BINDIR) -d $(MANDIR)
	install -m 644 dnsmasq.8 $(MANDIR)/man8 
	install -m 755 dnsmasq $(BINDIR)

romfs:
	$(ROMFSINST) /bin/dnsmasq
ifdef CONFIG_RTK_NETCOLOGNE_SETTING
	$(ROMFSINST) -s /var/udhcpc/resolv.conf  /etc/resolv.conf
else
	$(ROMFSINST) -s /var/resolv.conf  /etc/resolv.conf
endif
	




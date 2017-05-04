# Don't edit Makefile! Use conf-* for configuration.
#CC=$(FONGW_CROSS_COMPILER)gcc
CFLAGS=-O2 -Wimplicit -Wunused -Wcomment -Wchar-subscripts -Wuninitialized \
-Wshadow -Wcast-qual -Wcast-align -Wwrite-strings -include /usr/include/errno.h

INSTALL = install
INSTALLPREFIX ?= $(FONGW_INSTALL_PREFIX)

IT=libdjbdns.a libdjbdns.so.1
SYSDEPS=iopause.h select.h uint32.h uint64.h

.PHONY: all clean install

LIBDNS=dns_dfd.o dns_domain.o dns_dtda.o dns_ip.o dns_ipq.o dns_mx.o \
  dns_name.o dns_nd.o dns_packet.o dns_random.o dns_rcip.o dns_rcrw.o \
  dns_resolve.o dns_sortip.o dns_transmit.o dns_txt.o
LIBALLOC=alloc.o alloc_re.o stralloc_catb.o stralloc_cats.o stralloc_copy.o \
  stralloc_eady.o stralloc_opyb.o stralloc_opys.o stralloc_pend.o
LIBBYTE=byte_chr.o byte_copy.o byte_diff.o byte_zero.o case_diffb.o \
  fmt_ulong.o ip4_scan.o scan_ulong.o str_chr.o str_len.o str_start.o \
  uint16_pack.o uint16_unpack.o uint32_unpack.o
LIBUNIX=error.o ndelay_on.o open_read.o openreadclose.o readclose.o \
  socket_bind.o socket_conn.o socket_tcp.o socket_udp.o
LIBTAI=tai_pack.o taia_add.o taia_approx.o taia_frac.o taia_less.o \
  taia_now.o taia_pack.o taia_sub.o taia_uint.o
LIBENV=env.o
LIBMISC=iopause.o

LIBDNS_S=dns_dfd.os dns_domain.os dns_dtda.os dns_ip.os dns_ipq.os \
  dns_mx.os dns_name.os dns_nd.os dns_packet.os dns_random.os dns_rcip.os \
  dns_rcrw.os dns_resolve.os dns_sortip.os dns_transmit.os dns_txt.os
LIBALLOC_S=alloc.os alloc_re.os stralloc_catb.os stralloc_cats.os \
  stralloc_copy.os stralloc_eady.os stralloc_opyb.os stralloc_opys.os \
  stralloc_pend.os
LIBBYTE_S=byte_chr.os byte_copy.os byte_diff.os byte_zero.os case_diffb.os \
  fmt_ulong.os ip4_scan.os scan_ulong.os str_chr.os str_len.os str_start.os \
  uint16_pack.os uint16_unpack.os uint32_unpack.os
LIBUNIX_S=error.os ndelay_on.os open_read.os openreadclose.os readclose.os \
  socket_bind.os socket_conn.os socket_tcp.os socket_udp.os
LIBTAI_S=tai_pack.os taia_add.os taia_approx.os taia_frac.os taia_less.os \
  taia_now.os taia_pack.os taia_sub.os taia_uint.os
LIBENV_S=env.os
LIBMISC_S=iopause.os

it: $(IT) sysdeps

sysdeps: $(SYSDEPS)
	uname -srm >sysdeps
	grep sysdep iopause.h select.h uint32.h uint64.h >>sysdeps

libdjbdns.a: $(LIBDNS) $(LIBALLOC) $(LIBBYTE) $(LIBUNIX) $(LIBENV) \
$(LIBTAI) $(LIBMISC) $(SYSDEPS)
	rm -f libdjbdns.a
	ar cr libdjbdns.a $(LIBDNS) $(LIBALLOC) $(LIBBYTE) $(LIBUNIX) \
	  $(LIBENV) $(LIBMISC) $(LIBTAI)
	# most systems use ranlib
	ranlib libdjbdns.a

libdjbdns.so.1: $(LIBDNS_S) $(LIBALLOC_S) $(LIBBYTE_S) $(LIBUNIX_S) \
$(LIBENV_S) $(LIBTAI_S) $(LIBMISC_S) $(SYSDEPS)
	$(CC) -shared -Wl,-soname,libdjbdns.so.1 -o libdjbdns.so.1 \
	  $(LIBDNS_S) $(LIBALLOC_S) $(LIBBYTE_S) $(LIBUNIX_S) $(LIBENV_S) \
	  $(LIBMISC_S) $(LIBTAI_S)

%.o: %.c $(SYSDEPS)
	$(CC) $(CFLAGS) -c $< -o $@

%.os: %.c $(SYSDEPS)
	$(CC) -fPIC $(CFLAGS) -c $< -o $@

iopause.h: iopause.h1 iopause.h2
	( ( $(CC) -o trypoll trypoll.c && ./trypoll ) >/dev/null 2>&1 \
	&& cat iopause.h2 || cat iopause.h1 ) >iopause.h
	rm -f trypoll

select.h:
	( $(CC) -c tryssysel.c >/dev/null 2>&1 \
	&& cat select.h1 || cat select.h2 ) >select.h
	rm -f trysysel.o

uint32.h:
	( ( $(CC) -o tryulong32 tryulong32.c && ./tryulong32 ) >/dev/null 2>&1 \
	&& cat uint32.h2 || cat uint32.h1 ) >uint32.h
	rm -f tryulong32

uint64.h:
	( ( $(CC) -o tryulong64 tryulong64.c && ./tryulong64 ) >/dev/null 2>&1 \
	&& cat uint64.h2 || cat uint64.h1 ) >uint64.h
	rm -f tryulong64

clean:
	rm -f *.o *.os test.static test.shared libdjbdns.so sysdeps \
	  $(IT) $(SYSDEPS)


test: test.static test.shared
	cat sysdeps
	./test.static
	env LD_LIBRARY_PATH=.:$$LD_LIBRARY_PATH ./test.shared

test.static: test.o libdjbdns.a
	$(CC) -o test.static test.o libdjbdns.a
 
libdjbdns.so: libdjbdns.so.1
	ln -sf libdjbdns.so.1 libdjbdns.so
 
test.shared: test.o libdjbdns.so
	$(CC) -o test.shared test.o -L. -ldjbdns

install:
	$(INSTALL) -d $(INSTALLPREFIX)/include/djbdns
	@echo $(INSTALLPREFIX)
	$(INSTALL) ip4.h $(INSTALLPREFIX)/include/djbdns
	$(INSTALL) dns.h $(INSTALLPREFIX)/include/djbdns
 
all: $(IT) sysdeps

SHELL = /bin/sh

srcdir = .
top_srcdir = ..
top_builddir = $(top_srcdir)
cur_dir = $(shell pwd)
cwmp_root = $(cur_dir)/../..
uClinux_root = $(cwmp_root)/../..

prefix = $(cwmp_root)
exec_prefix = ${prefix}

libdir = ${exec_prefix}/lib
includedir = ${prefix}/include

INSTALL = /usr/bin/install -c
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_HEADER = $(INSTALL_DATA)
NORMAL_INSTALL = :
POST_INSTALL = :

OBJEXT = o
CC = $(RSDK)/bin/rsdk-linux-gcc  -Os
CPP = $(RSDK)/bin/rsdk-linux-gcc  -Os -E
CXX = $(RSDK)/bin/rsdk-linux-g++
LN_S = ln -s
RANLIB = $(RSDK)/bin/rsdk-linux-ranlib
STRIP = $(RSDK)/bin/rsdk-linux-strip
install_sh = $(cwmp_root)/gsoaplib/install-sh
platform = LINUX
CLEANFILES = stdsoap2_cpp.cpp stdsoap2_ck.c stdsoap2_ck_cpp.cpp stdsoap2_ssl.c stdsoap2_ssl_cpp.cpp

lib_LIBRARIES = libgsoap.a libgsoapck.a libgsoapssl.a libgsoapmssl.a
libgsoap_a_SOURCES = stdsoap2.c
libgsoap_a_CFLAGS = $(SOAPCPP2_DEBUG) $(SOAPCPP2_NONAMESPACES) -D$(platform)
libgsoap___a_SOURCES = stdsoap2_cpp.cpp
libgsoap___a_CXXFLAGS = $(SOAPCPP2_DEBUG) $(SOAPCPP2_NONAMESPACES) -D$(platform)
libgsoapck_a_SOURCES = stdsoap2_ck.c
libgsoapck_a_CFLAGS = $(SOAPCPP2_DEBUG) $(SOAPCPP2_NONAMESPACES) -D$(platform) -DWITH_COOKIES
libgsoapck___a_SOURCES = stdsoap2_ck_cpp.cpp
libgsoapck___a_CXXFLAGS = $(SOAPCPP2_DEBUG) $(SOAPCPP2_NONAMESPACES) -D$(platform) -DWITH_COOKIES
libgsoapssl_a_SOURCES = stdsoap2_ssl.c
libgsoapssl_a_CFLAGS = $(SOAPCPP2_DEBUG) $(SOAPCPP2_NONAMESPACES) -D$(platform) -DWITH_COOKIES -DWITH_OPENSSL
libgsoapmssl_a_SOURCES = stdsoap2_ssl.c
libgsoapmssl_a_CFLAGS = $(SOAPCPP2_DEBUG) $(SOAPCPP2_NONAMESPACES) -D$(platform) -DWITH_COOKIES -D_WITH_MATRIXSSL_
libgsoapssl___a_SOURCES = stdsoap2_ssl_cpp.cpp
libgsoapssl___a_CXXFLAGS = $(SOAPCPP2_DEBUG) $(SOAPCPP2_NONAMESPACES) -D$(platform) -DWITH_OPENSSL -DWITH_DOM

BUILT_SOURCES = $(lib_LIBRARIES)

include_HEADERS = stdsoap2.h
subdir = soapcpp2
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
LIBRARIES = $(lib_LIBRARIES)

libgsoap_a_AR = $(AR) cru
libgsoap_a_LIBADD =
am_libgsoap_a_OBJECTS = libgsoap_a-stdsoap2.$(OBJEXT)
libgsoap_a_OBJECTS = $(am_libgsoap_a_OBJECTS)
libgsoapck_a_AR = $(AR) cru
libgsoapck_a_LIBADD =
am_libgsoapck_a_OBJECTS = libgsoapck_a-stdsoap2_ck.$(OBJEXT)
libgsoapck_a_OBJECTS = $(am_libgsoapck_a_OBJECTS)
libgsoapmssl_a_AR = $(AR) cru
libgsoapmssl_a_LIBADD =
am_libgsoapmssl_a_OBJECTS = libgsoapmssl_a-stdsoap2_ssl.$(OBJEXT)
libgsoapmssl_a_OBJECTS = $(am_libgsoapmssl_a_OBJECTS)
libgsoapssl_a_AR = $(AR) cru
libgsoapssl_a_LIBADD =
am_libgsoapssl_a_OBJECTS = libgsoapssl_a-stdsoap2_ssl.$(OBJEXT)
libgsoapssl_a_OBJECTS = $(am_libgsoapssl_a_OBJECTS)

DEFAULT_INCLUDES =  -I. -I$(srcdir) -I$(top_builddir)
ifeq ($(CONFIG_USER_CWMP_WITH_MATRIXSSL_NEW),y)
CPPFLAGS = -DLINUX -I$(uClinux_root)/lib/libssl/include -I$(cwmp_root)/matrixssl-3-6-2-open -I$(cwmp_root)/matrixssl-3-6-2-open/matrixssl
else
CPPFLAGS = -DLINUX -I$(uClinux_root)/lib/libssl/include -I$(cwmp_root)/matrixssl-1-8-3-open
endif
LDFLAGS =  -L$(uClinux_root)/lib/librtk
COMPILE = $(CC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
CCLD = $(CC)
LINK = $(CCLD) $(CFLAGS) $(LDFLAGS) -o $@
CFLAGS = -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DUSE_LIBMD5 -I$(uClinux_root)/include -I$(uClinux_root)/lib/libmd5 -Dlinux -D__linux__ -Dunix -DEMBED -D_USE_RSDK_WRAPPER_ -D_LINUX_2_6_ -D_LINUX_2_6_30_ -include linux/config.h -I$(uClinux_root) -DWITH_IPV6 -DWITH_CASEINSENSITIVETAGS
DIST_SOURCES = $(libgsoap_a_SOURCES) $(libgsoapck_a_SOURCES) $(libgsoapmssl_a_SOURCES) $(libgsoapssl_a_SOURCES)
HEADERS = $(include_HEADERS)

all: $(BUILT_SOURCES)
	$(MAKE) all-recursive

all-recursive:

AR = ar
install-libLIBRARIES: $(lib_LIBRARIES)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(libdir)
	@list='$(lib_LIBRARIES)'; for p in $$list; do \
	  if test -f $$p; then \
	    f="`echo $$p | sed -e 's|^.*/||'`"; \
	    echo " $(INSTALL_DATA) $$p $(libdir)/$$f"; \
	    $(INSTALL_DATA) $$p $(libdir)/$$f; \
	  else :; fi; \
	done
	@$(POST_INSTALL)
	@list='$(lib_LIBRARIES)'; for p in $$list; do \
	  if test -f $$p; then \
	    p="`echo $$p | sed -e 's|^.*/||'`"; \
	    echo " $(RANLIB) $(libdir)/$$p"; \
	    $(RANLIB) $(libdir)/$$p; \
	  else :; fi; \
	done

libgsoap_a-stdsoap2.$(OBJEXT): stdsoap2.c
libgsoap.a: $(libgsoap_a_OBJECTS) $(libgsoap_a_DEPENDENCIES)
	-rm -f libgsoap.a
	$(libgsoap_a_AR) libgsoap.a $(libgsoap_a_OBJECTS) $(libgsoap_a_LIBADD)
	$(RANLIB) libgsoap.a
libgsoapck_a-stdsoap2_ck.$(OBJEXT): stdsoap2_ck.c
libgsoapck.a: $(libgsoapck_a_OBJECTS) $(libgsoapck_a_DEPENDENCIES)
	-rm -f libgsoapck.a
	$(libgsoapck_a_AR) libgsoapck.a $(libgsoapck_a_OBJECTS) $(libgsoapck_a_LIBADD)
	$(RANLIB) libgsoapck.a
libgsoapmssl_a-stdsoap2_ssl.$(OBJEXT): stdsoap2_ssl.c
libgsoapmssl.a: $(libgsoapmssl_a_OBJECTS) $(libgsoapmssl_a_DEPENDENCIES)
	-rm -f libgsoapmssl.a
	$(libgsoapmssl_a_AR) libgsoapmssl.a $(libgsoapmssl_a_OBJECTS) $(libgsoapmssl_a_LIBADD)
	$(RANLIB) libgsoapmssl.a
libgsoapssl_a-stdsoap2_ssl.$(OBJEXT): stdsoap2_ssl.c
libgsoapssl.a: $(libgsoapssl_a_OBJECTS) $(libgsoapssl_a_DEPENDENCIES)
	-rm -f libgsoapssl.a
	$(libgsoapssl_a_AR) libgsoapssl.a $(libgsoapssl_a_OBJECTS) $(libgsoapssl_a_LIBADD)
	$(RANLIB) libgsoapssl.a

.c.o:
	$(COMPILE) -c `test -f '$<' || echo '$(srcdir)/'`$<

.c.obj:
	$(COMPILE) -c `cygpath -w $<`

libgsoap_a-stdsoap2.o: stdsoap2.c
	$(CC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(libgsoap_a_CFLAGS) $(CFLAGS) -c -o libgsoap_a-stdsoap2.o `test -f 'stdsoap2.c' || echo '$(srcdir)/'`stdsoap2.c

libgsoap_a-stdsoap2.obj: stdsoap2.c
	$(CC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(libgsoap_a_CFLAGS) $(CFLAGS) -c -o libgsoap_a-stdsoap2.obj `cygpath -w stdsoap2.c`

libgsoapck_a-stdsoap2_ck.o: stdsoap2_ck.c
	$(CC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(libgsoapck_a_CFLAGS) $(CFLAGS) -c -o libgsoapck_a-stdsoap2_ck.o `test -f 'stdsoap2_ck.c' || echo '$(srcdir)/'`stdsoap2_ck.c

libgsoapck_a-stdsoap2_ck.obj: stdsoap2_ck.c
	$(CC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(libgsoapck_a_CFLAGS) $(CFLAGS) -c -o libgsoapck_a-stdsoap2_ck.obj `cygpath -w stdsoap2_ck.c`

libgsoapmssl_a-stdsoap2_ssl.o: stdsoap2_ssl.c
	$(CC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(libgsoapmssl_a_CFLAGS) $(CFLAGS) -c -o libgsoapmssl_a-stdsoap2_ssl.o `test -f 'stdsoap2_ssl.c' || echo '$(srcdir)/'`stdsoap2_ssl.c

libgsoapmssl_a-stdsoap2_ssl.obj: stdsoap2_ssl.c
	$(CC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(libgsoapmssl_a_CFLAGS) $(CFLAGS) -c -o libgsoapmssl_a-stdsoap2_ssl.obj `cygpath -w stdsoap2_ssl.c`

libgsoapssl_a-stdsoap2_ssl.o: stdsoap2_ssl.c
	$(CC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(libgsoapssl_a_CFLAGS) $(CFLAGS) -c -o libgsoapssl_a-stdsoap2_ssl.o `test -f 'stdsoap2_ssl.c' || echo '$(srcdir)/'`stdsoap2_ssl.c

libgsoapssl_a-stdsoap2_ssl.obj: stdsoap2_ssl.c
	$(CC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(libgsoapssl_a_CFLAGS) $(CFLAGS) -c -o libgsoapssl_a-stdsoap2_ssl.obj `cygpath -w stdsoap2_ssl.c`
CCDEPMODE = depmode=none
install-includeHEADERS: $(include_HEADERS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(includedir)
	@list='$(include_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " $(INSTALL_HEADER) $$d$$p $(includedir)/$$f"; \
	  $(INSTALL_HEADER) $$d$$p $(includedir)/$$f; \
	done

install: install-includeHEADERS install-libLIBRARIES

clean:
	-test -z "$(CLEANFILES)" || rm -f $(CLEANFILES)
	-test -z "$(lib_LIBRARIES)" || rm -f $(lib_LIBRARIES)
	-rm -f *.$(OBJEXT) core *.core

.PHONY: all clean install install-includeHEADERS install-libLIBRARIES

stdsoap2_ck.c: stdsoap2.cpp
	$(LN_S) stdsoap2.cpp stdsoap2_ck.c

stdsoap2_cpp.cpp: stdsoap2.cpp
	$(LN_S) stdsoap2.cpp stdsoap2_cpp.cpp

stdsoap2_ck_cpp.cpp: stdsoap2.cpp
	$(LN_S) stdsoap2.cpp stdsoap2_ck_cpp.cpp

stdsoap2_ssl.c: stdsoap2.cpp
	$(LN_S) stdsoap2.cpp stdsoap2_ssl.c

stdsoap2_ssl_cpp.cpp: stdsoap2.cpp
	$(LN_S) stdsoap2.cpp stdsoap2_ssl_cpp.cpp


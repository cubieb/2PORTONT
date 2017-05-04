# -- updatedd: Makefile.in --
#
# Copyright (C) 2002, 2003, 2004, 2005 Philipp Benner
#
# This file is part of UpdateDD - http://updatedd.philipp-benner.de.
#
# UpdateDD is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# any later version.
#
# UpdateDD is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with UpdateDD; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

SHELL		= /bin/sh -e


prefix		= /usr/local
exec_prefix	= ${prefix}
sbindir		= ${exec_prefix}/sbin
libdir		= ${exec_prefix}/lib
srcdir		= .
top_srcdir	= ../..
builddir	= .
top_builddir	= ../../

INSTALL		= /usr/bin/install -c
#CC		= /opt/toolchain_mips_nofpu/bin/mips-linux-gcc
ifdef CONFIG_USE_RSDK_WRAPPER
CFLAGS		+= -I$(top_builddir) -I$(top_srcdir)/include -Wall -Wwrite-strings -Wstrict-prototypes -Wno-trigraphs -g -O2 -fPIC
else #CONFIG_USE_RSDK_WRAPPER
CFLAGS		= -I$(top_builddir) -I$(top_srcdir)/include -Wall -Wwrite-strings -Wstrict-prototypes -Wno-trigraphs -g -O2 -fPIC
endif #CONFIG_USE_RSDK_WRAPPER
LIBERROR	= $(top_builddir)/libexception_handle/libexception_handle.a

#OBJECTS		:= changeip.o dyndns.o eurodyndns.o hn.o noip.o ods.o ovh.o regfish.o tzo.o
ifeq ($(CONFIG_BOA_WEB_E8B_CH),y)
OBJECTS		:= dyndns.o tzo.o gnudip.o
else
OBJECTS		:= dyndns.o tzo.o oray.o noip.o
endif
PLUGINS_ROOT	:= $(OBJECTS:.o= )

UNAME:=$(shell uname -s)


ifdef CONFIG_USE_RSDK_WRAPPER
export LDFLAGS=-Wl,-soname,$*.so -shared -o $*.so
export PLUGINS= $(OBJECTS:.o=.so)
endif #CONFIG_USE_RSDK_WRAPPER


#ifeq (Darwin,$(UNAME))
#  export LDFLAGS=-dynamic -bundle -o $*.bundle
#  export PLUGINS= $(OBJECTS:.o=.bundle)
#else
#  export LDFLAGS=-Wl,-soname,$*.so -shared -o $*.so
#  export PLUGINS= $(OBJECTS:.o=.so)
#endif

all: plugins

plugins: $(OBJECTS) $(PLUGINS_ROOT:%=%)
%: %.o
	$(CC) $(LDFLAGS) $< \
		$(LIBERROR)

install:
	$(INSTALL) -d $(DESTDIR)$(libdir)/updatedd
	$(INSTALL) -m 755 $(PLUGINS) $(DESTDIR)$(libdir)/updatedd

uninstall:
	$(RM) -r $(libdir)/updatedd

clean:
	$(RM) $(OBJECTS)
	$(RM) $(PLUGINS)

distclean: clean
	$(RM) Makefile

#changeip.o:	changeip.h
dyndns.o:	dyndns.h
#eurodyndns.o:	eurodyndns.h
#hn.o:		hn.h
noip.o:		noip.h
#ods.o:		ods.h
#ovh.o:		ovh.h
#regfish.o:	regfish.h
tzo.o:		tzo.h
gnudip.o:   gnudip.h
oray.o:		oray.h

.PHONY: all plugins install uninstall clean distclean

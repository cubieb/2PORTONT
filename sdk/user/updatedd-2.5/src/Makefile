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


bin		= updatedd

prefix		= /usr/local
exec_prefix	= ${prefix}
bindir		= ${exec_prefix}/bin
libdir		= ${exec_prefix}/lib
mandir		= ${prefix}/man
srcdir		= .
top_srcdir	= ..
builddir	= .
top_builddir	= ../

INSTALL		= /usr/bin/install -c
#CC		= /opt/toolchain_mips_nofpu/bin/mips-linux-gcc
#STRIP		=/opt/toolchain_mips_nofpu/bin/mips-linux-strip
#CFLAGS		= -I$(top_builddir) -I$(top_srcdir)/include -Wall -Wwrite-strings -Wstrict-prototypes -Wno-trigraphs -g -O2 -DLIBPATH=\"$(libdir)/updatedd\"
ifdef CONFIG_USE_RSDK_WRAPPER
CFLAGS		+= -I$(top_builddir) -I$(top_srcdir)/include -Wall -Wwrite-strings -Wstrict-prototypes -Wno-trigraphs -g -O2 -DEMBED -DLIBPATH=\"/lib\"
else #CONFIG_USE_RSDK_WRAPPER
CFLAGS		= -I../../../ -I../../../lib/uClibc/include -I$(top_builddir) -I$(top_srcdir)/include -Wall -Wwrite-strings -Wstrict-prototypes -Wno-trigraphs -g -O2 -DEMBED -DLIBPATH=\"/lib\"
endif #CONFIG_USE_RSDK_WRAPPER
CFLAGS          += -fPIC
LIBS		= -ldl -lnsl -lresolv -L../../boa/src/LINUX $(LIBMIB) $(LIBVOIP)
LIBERROR	= $(top_builddir)/libexception_handle/libexception_handle.a

ifeq ($(CONFIG_BOA_WEB_E8B_CH),y)
OBJECTS		= updatedd.o plugins/dyndns.o plugins/tzo.o plugins/gnudip.o
else
OBJECTS		= updatedd.o plugins/dyndns.o plugins/tzo.o plugins/oray.o plugins/noip.o
endif

UNAME:=$(shell uname -s)

ifeq (Darwin,$(UNAME))
  CFLAGS+=-D__DARWIN__
endif
all: $(bin)

$(bin): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) $(LIBERROR) -o $(bin)
#	$(CROSS)ar rcs $(LIBS) $< $(LIBERROR) -o $(bin)

install:
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL) -m 755 $(bin) $(DESTDIR)$(bindir)

	$(INSTALL) -d $(DESTDIR)$(mandir)/man1
	$(INSTALL) -m 644 $(srcdir)/updatedd.1 $(DESTDIR)$(mandir)/man1

uninstall:
	$(RM) $(bindir)$(bin)
	$(RM) $(mandir)/man1/updatedd.1

clean:
	$(RM) $(OBJECTS)
	$(RM) $(bin)

distclean: clean
	$(RM) Makefile

updatedd.o: updatedd.h

.PHONY: all install uninstall clean distclean

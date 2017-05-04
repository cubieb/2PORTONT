# Copyright (c) 2010-2012 Helsinki Institute for Information Technology
# and University of Helsinki.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

PREFIX = /usr/local
EXEC_PREFIX = $(PREFIX)
BINDIR = $(EXEC_PREFIX)/sbin
ETCDIR = $(EXEC_PREFIX)/etc/y1731
MANDIR = /usr/share/man/man1
SRCDIR = $(EXEC_PREFIX)/src
VARDIR = /var/log
CONFIG_FILE=example-config.xml
MEG_FILE=example-meg.xml


#CC=gcc
#MAKE=make
DIRS= -D CFGFILE="\"$(ETCDIR)/config.xml\"" -D MEGFILE="\"$(ETCDIR)/meg.xml\"" -D LOGFILE="\"$(VARDIR)/oamd.log\""
COMPILETIME= -D COMPILETIME="\"$(shell date --iso=seconds)\""
REVISION= -D REVISION="\"$(shell svn info | grep Revision)\""
#CFLAGS=-std=c99 -O2 -pedantic -Wall -Werror -Wextra -Wvariadic-macros $(DIRS) $(COMPILETIME) $(REVISION)
#-save-temps -g
#Uncomment the following line if test features are needed (remember clean all).
#CFLAGS+= -D TEST_FEATURE 
#INCLUDES= -I. -I../include/ -I/usr/local/include

SRC= src/
#add -static below for the voyage linux as it has older version of glibc.
# if -static is present here the ccontrold might need -ltinfo also see below
LIBS=$(SRC)ezxml/libezxml.a -lm 

SOURCES=$(addprefix $(SRC),oamd.c debug.c init.c pdu.c tools.c output.c input.c configuration.c extendedpsquared.c)
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=oamd

SOURCESTOOL=$(addprefix $(SRC),oamtool.c debug.c init.c pdu.c tools.c output.c input.c configuration.c extendedpsquared.c)
OBJECTSTOOL=$(SOURCESTOOL:.c=.o)
EXECUTABLETOOL=oamtool

SOURCESOAMPING=$(addprefix $(SRC),oamping.c debug.c output.c pdu.c tools.c configuration.c init.c extendedpsquared.c)
OBJECTSOAMPING=$(SOURCESOAMPING:.c=.o)
EXECUTABLEOAMPING=oamping

SOURCESOAMTRACE=$(addprefix $(SRC),oamtrace.c debug.c output.c pdu.c tools.c configuration.c init.c extendedpsquared.c)
OBJECTSOAMTRACE=$(SOURCESOAMTRACE:.c=.o)
EXECUTABLEOAMTRACE=oamtrace

SOURCESCCD=$(addprefix $(SRC),ccontrold.c)
OBJECTSCCD=$(SOURCESCCD:.c=.o)
EXECUTABLECCD=ccontrold

SRCS= $(shell find $(SRC) -name '*.c' -print0 | xargs -0)
HEADERS= $(shell find $(SRC) -name '*.h' -print0 | xargs -0)

VERSION=0.0.1
TARBALL = y1731-$(VERSION).tar.gz
BINBALL = bin-y1731-$(VERSION).tar.gz

ETAGS= etags

.PHONY: all install uninstall
#all: ezxml oamd oamtool oamping oamtrace ccontrold doxygen TAGS
all: ezxml oamd oamtool oamping oamtrace 

install: all
	cp $(EXECUTABLE) $(BINDIR)
	cp $(EXECUTABLETOOL) $(BINDIR)
	cp $(EXECUTABLEOAMPING) $(BINDIR)
	cp $(EXECUTABLECCD) $(BINDIR) 
	chown root:root $(BINDIR)/$(EXECUTABLE)
	chown root:root $(BINDIR)/$(EXECUTABLETOOL)
	chown root:root $(BINDIR)/$(EXECUTABLEOAMPING)
	chown root:root $(BINDIR)/$(EXECUTABLECCD)
	chmod +x $(BINDIR)/$(EXECUTABLE)
	chmod +x $(BINDIR)/$(EXECUTABLETOOL)
	chmod +x $(BINDIR)/$(EXECUTABLEOAMPING)	
	chmod +x $(BINDIR)/$(EXECUTABLEOAMTRACE)
	chmod +x $(BINDIR)/$(EXECUTABLECCD)
	mkdir -p $(ETCDIR)
	cp --no-clobber $(CONFIG_FILE) $(ETCDIR)/config.xml
	cp --no-clobber $(MEG_FILE) $(ETCDIR)/meg.xml
	mkdir -p $(MANDIR)
	cp doc/man/* $(MANDIR)
	chmod a+r $(MANDIR)/oamd.1
	chmod a+r $(MANDIR)/oamping.1
	chmod a+r $(MANDIR)/oamtrace.1
	chmod a+r $(MANDIR)/oamtool.1

uninstall:
	-rm $(BINDIR)/$(EXECUTABLE)
	-rm $(BINDIR)/$(EXECUTABLETOOL)
	-rm $(BINDIR)/$(EXECUTABLEOAMPING) 
	-rm $(BINDIR)/$(EXECUTABLEOAMTRACE)
	-rm $(BINDIR)/$(EXECUTABLECCD)
	-rm $(MANDIR)/oamd.1
	-rm $(MANDIR)/oamping.1
	-rm $(MANDIR)/oamtrace.1
	-rm $(MANDIR)/oamtool.1

purge:
	-rm $(BINDIR)/$(EXECUTABLE)
	-rm $(BINDIR)/$(EXECUTABLETOOL)
	-rm $(BINDIR)/$(EXECUTABLEOAMPING)
	-rm $(BINDIR)/$(EXECUTABLECCD)	
	-rm $(MANDIR)/oamd.1
	-rm $(MANDIR)/oamping.1
	-rm $(MANDIR)/oamtrace.1
	-rm $(MANDIR)/oamtool.1
	-rm -rf $(ETCDIR)/*
	-rmdir $(ETCDIR)

oamd: $(SOURCES)
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(INCLUDES) $(OBJECTS) $(LIBS) -o $@

oamtool: $(SOURCESTOOL) 
$(EXECUTABLETOOL): $(OBJECTSTOOL)
	$(CC) $(INCLUDES) $(OBJECTSTOOL) $(LIBS) -o $@

oamping: $(SOURCESOAMPING) 
$(EXECUTABLEOAMPING): $(OBJECTSOAMPING)
	$(CC) $(INCLUDES) $(OBJECTSOAMPING) $(LIBS) -o $@

oamtrace: $(SOURCESOAMTRACE) 
$(EXECUTABLEOAMTRACE): $(OBJECTSOAMTRACE)
	$(CC) $(INCLUDES) $(OBJECTSOAMTRACE) $(LIBS) -o $@

ccontrold: $(SOURCESCCD) 
$(EXECUTABLECCD): $(OBJECTSCCD)
	$(CC) $(INCLUDES) $(OBJECTSCCD) $(LIBS) -lncurses -o $@
#above line might need -ltinfo on some machines when -static is present

.PHONY: ezxml
ezxml:
	$(MAKE) --directory=$(SRC)ezxml

tarball: $(TARBALL)

binball: $(BINBALL)

TAGS: $(SRCS) $(HEADERS)
	-rm -rf TAGS
	$(ETAGS) $(SRCS) $(HEADERS)

# run doxygen and stop if "Warning" is found from the outputs.
doxygen: doc/Doxyfile
	doxygen $< 2>&1 | ( ! grep Warning )

.PHONY: clean clean-regular clean-tarball clean-docs clean-tags clean-ezxml
clean: clean-regular clean-tarball clean-docs clean-tags clean-ezxml

clean-tags:
	-rm -rf TAGS

clean-ezxml:
	$(MAKE) --directory=$(SRC)ezxml clean

clean-regular:
	-rm -rf $(SRC)*o \
		$(EXECUTABLE) \
		$(EXECUTABLETOOL) \
		$(EXECUTABLECCD) \
		$(EXECUTABLEOAMPING) \
		$(EXECUTABLEOAMTRACE)
clean-docs:
	-rm -rf doc/html/
	-rm -rf doc/latex/

clean-tarball:
	-rm -f $(TARBALL)

clean-binball:
	-rm -f $(BINBALL)

$(TARBALL): clean
	tar -zcf $@ \
		--transform 's,^,y1731-$(VERSION)/,' \
		--exclude=*~ \
		--exclude=.svn \
		--exclude=*.log \
		--exclude=*.tmp \
		--exclude=*.in \
                *

$(BINBALL): all 
	tar -zcf $@ \
		--transform 's,^,y1731-$(VERSION)/,' \
		$(EXECUTABLE) \
		$(EXECUTABLETOOL) \
                $(EXECUTABLECCD) \
                $(EXECUTABLEOAMPING) \
                $(EXECUTABLEOAMTRACE) \
		$(CONFIG_FILE) \
		$(MEG_FILE)

romfs:
	$(ROMFSINST) /bin/oamd
#	$(ROMFSINST) /bin/oamping
#	$(ROMFSINST) /bin/oamtool
#	$(ROMFSINST) /bin/oamtrace
	
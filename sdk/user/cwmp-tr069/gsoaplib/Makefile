SHELL = /bin/sh

srcdir = .
top_srcdir = .
cur_dir = $(shell pwd)

prefix = $(shell pwd)/..
exec_prefix = ${prefix}

CC = $(RSDK)/bin/rsdk-linux-gcc  -Os
CPP = $(RSDK)/bin/rsdk-linux-gcc  -Os -E
CXX = $(RSDK)/bin/rsdk-linux-g++
LN_S = ln -s
PACKAGE = soapcpp2
RANLIB = $(RSDK)/bin/rsdk-linux-ranlib
STRIP = $(RSDK)/bin/rsdk-linux-strip
install_sh = $(cur_dir)/install-sh
platform = LINUX

SUBDIRS = soapcpp2 #libcwmp

subdir = .
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
RECURSIVE_TARGETS = all-recursive install-recursive

all:
	$(MAKE) all-recursive

$(RECURSIVE_TARGETS):
	@set fnord $$MAKEFLAGS; amf=$$2; \
	target=`echo $@ | sed s/-recursive//`; \
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  echo "Making $$target in $$subdir"; \
	    local_target="$$target"; \
	  (cd $$subdir && $(MAKE) $$local_target) || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done; \

install: install-recursive

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

.PHONY: $(RECURSIVE_TARGETS) all all-am clean install


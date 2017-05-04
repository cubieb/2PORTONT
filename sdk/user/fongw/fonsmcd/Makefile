# Makefile
PROFILE="NONE"

INSTALLPREFIX = $(FONGW_INSTALL_PREFIX)
CROSS_COMPILER = $(FONGW_CROSS_COMPILER)
#ARCH = $(FONGW_ARCH)
CFLAGS ="-I$(INSTALLPREFIX)/include -I$(ROOTDIR)/user/fongw/daemontools-0.76/compile -I$(ROOTDIR)/user/fongw/libdaemon-0.14"
LDFLAGS ="-L$(ROOTDIR)/user/fongw/djbdns-1.05 -L$(INSTALLPREFIX)/lib"

.PHONY: all clean install

all: fonsmcd

fonsmcd: build
	mkdir -p deb

build:	
	cd src/ && ./bootstrap
	cd src/build/ && ../configure --prefix=$(ROOTDIR)/user/fongw/fonsmcd/deb/usr CC=$(CROSS_COMPILER)gcc --host=$(FONGW_ARCH)-linux CFLAGS=$(CFLAGS) LDFLAGS=$(LDFLAGS) --enable-static --disable-shared --enable-debug=true
	#cd src/build/ && ../configure CC=$(CROSS_COMPILER)gcc --host=$(ARCH)-linux CFLAGS=$(CFLAGS) LDFLAGS=$(LDFLAGS)
	make -C src/build
	#cp -r files/ deb/
	rsync -av --exclude=".*" files/ deb/
	#make -C src/build install DESTDIR=$(ROOTDIR)/user/fongw/fonsmcd/deb
	make -C src/build install
	$(FONGW_CROSS_COMPILER)strip deb/usr/bin/fonsmcd
	$(FONGW_CROSS_COMPILER)strip deb/usr/bin/fonsmcd_debug

clean:
	rm -rf deb/
	rm -rf src/build
	rm -rf src/install
	rm -rf .configaux
	cd src/ && ./bootstrap --clean


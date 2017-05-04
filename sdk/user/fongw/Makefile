# fongw.deb Makefile
VERSION=0.3
#ARCH=i386
CHILLI=foncoova
DNS=fondnsmasq
DJBDNS=djbdns-1.05
FONSMCD=fonsmcd
PACKAGE=fongw_$(VERSION)_$(ARCH).deb
FONGW_INSTALL_PREFIX=$(shell pwd)/tmp
FONGW_ARCH=mips
FONGW_CROSS_COMPILER=$(CROSS_COMPILE)
LIBDAEMON_FLAG=--prefix=$(FONGW_INSTALL_PREFIX) CC=$(FONGW_CROSS_COMPILER)gcc --host=$(FONGW_ARCH)-linux --enable-static --disable-shared --localstatedir=/var ac_cv_func_setpgrp_void=yes
#GENGETOPT_FLAG=--prefix=$(FONGW_INSTALL_PREFIX) CC=$(FONGW_CROSS_COMPILER)gcc --host=$(FONGW_ARCH)-linux --disable-shared --disable-static
GENGETOPT_FLAG=--prefix=$(FONGW_INSTALL_PREFIX) --with-gnu-ld CC=gcc CXX=g++ --disable-shared --disable-static LDFLAGS='' LD='/usr/bin/ld' RANLIB='ranlib'
GENGETOPT=gengetopt-2.22.5
TMP_DIR=deb
CHILLI_VER=coova-chilli-1.2.9

#AUTOCONFIG_POSTFIX_EXTRAS+=\
                ac_cv_func_getpgrp_void=no \
                ac_cv_func_setpgrp_void=yes \
                ac_cv_func_memcmp_working=yes \
                rb_cv_binary_elf=no \
                rb_cv_negative_time_t=no

export FONGW_INSTALL_PREFIX
export FONGW_ARCH
export FONGW_CROSS_COMPILER
export LDFLAGS
export CFLAGS
export ROOTDIR
export CC=$(FONGW_CROSS_COMPILER)gcc
export PATH := $(shell pwd)/tmp/bin:$(PATH)

all: $(PACKAGE) 

$(PACKAGE):
	mkdir -p $(TMP_DIR)
	#make chilli
	cd $(GENGETOPT) && ./configure $(GENGETOPT_FLAG)
	$(MAKE) -C $(GENGETOPT)
	$(MAKE) -C $(GENGETOPT) install
	cp $(FONGW_INSTALL_PREFIX)/bin/gengetopt $(CHILLI)/$(CHILLI_VER)/src/gengetopt
	$(MAKE) -C $(CHILLI)
	cp -r $(CHILLI)/deb/* $(TMP_DIR)
	#make dns
	#$(MAKE) -C $(DNS)
	#cp -r $(DNS)/deb/* $(TMP_DIR)
	$(MAKE) -C $(DJBDNS)
	$(MAKE) -C $(DJBDNS) install
	cd libdaemon-0.14 && ./configure $(LIBDAEMON_FLAG)
	$(MAKE) -C libdaemon-0.14
	$(MAKE) -C libdaemon-0.14 install
	cd daemontools-0.76 && ./package/compile
	#make  fonsmcd
	$(MAKE) -C $(FONSMCD)
	cp -r $(FONSMCD)/deb/* $(TMP_DIR)

clean:
	-$(MAKE) -C $(CHILLI) clean
	#-$(MAKE) -C $(DNS) clean
	-$(MAKE) -C $(FONSMCD) clean

	$(RM) -rf $(TMP_DIR)
	$(RM) -f $(PACKAGE)

lintian:
	lintian $(PACKAGE)

romfs:
	$(ROMFSINST) ./deb/etc/fon_device /etc/fon_device
	$(ROMFSINST) ./deb/etc/fon_revision /etc/fon_revision
	$(ROMFSINST) ./deb/etc/fonsmcd.conf /etc/fonsmcd.conf
	$(ROMFSINST) ./deb/etc/fon_version /etc/fon_version
	$(ROMFSINST) ./deb/etc/public_sm_rsa_key.pem /etc/public_sm_rsa_key.pem
	$(ROMFSINST) ./deb/usr/bin/fonsmcd /bin/fonsmcd
#	$(ROMFSINST) ./deb/usr/bin/fonsmcd_debug /bin/fonsmcd_debug
	$(ROMFSINST) ./deb/usr/sbin/chilli /bin/chilli
	$(ROMFSINST) ./deb/usr/sbin/chilli_radconfig /bin/chilli_radconfig
	$(ROMFSINST) ./deb/usr/sbin/chilli_opt /bin/chilli_opt
	$(ROMFSINST) ./chilli_radconfig.sh /etc/chilli_radconfig.sh

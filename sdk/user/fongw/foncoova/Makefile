# foncoova.deb Makefile
VERSION=0.1
CHILLI_VERSION=coova-chilli-1.2.9
CHILLI_URL=http://ap.coova.org/chilli
PATCHES= patches/001-endian_fix.patch patches/300-radius_retrytimeout.patch \
patches/205-dev-tun.patch patches/207-chilli_whitelist.patch \
patches/208-redir_logout.patch patches/206-fix_localusers_pap.patch
BINARIES=deb/usr/sbin/chilli deb/usr/sbin/chilli_radconfig
SCRIPTS=deb/etc/chilli.conf deb/etc/cron.daily/chilli deb/etc/init.d/chilli \
deb/etc/default/chilli

# use this to create a single package with other fon daemons
all: $(BINARIES) $(SCRIPTS)
	touch .done

#deb/DEBIAN/control:  $(BINARIES) $(SCRIPTS) DEBIAN/control
#	cp -R DEBIAN deb/

deb/usr/sbin/chilli: 
	#build/patched	
	cd $(CHILLI_VERSION) && ./configure --prefix=$$(pwd) --host=$(FONGW_ARCH)-linux --enable-static --disable-shared --enable-chilliscript=no --localstatedir=/var --enable-statusfile --enable-ipwhitelist
	make -C $(CHILLI_VERSION) -f Makefile
	make -C $(CHILLI_VERSION) install
	mkdir -p deb/usr/sbin
	cp $(CHILLI_VERSION)/sbin/chilli deb/usr/sbin/
	cp $(CHILLI_VERSION)/sbin/chilli_radconfig deb/usr/sbin/
	cp $(CHILLI_VERSION)/sbin/chilli_opt deb/usr/sbin/
	$(FONGW_CROSS_COMPILER)strip deb/usr/sbin/chilli_radconfig
	$(FONGW_CROSS_COMPILER)strip deb/usr/sbin/chilli
	$(FONGW_CROSS_COMPILER)strip deb/usr/sbin/chilli_opt

deb/usr/sbin/chilli_radconfig: deb/usr/sbin/chilli

#build/patched: build/chillispot-$(CHILLI_VERSION).tar.gz $(PATCHES)
#	tar zxpvf build/chillispot-$(CHILLI_VERSION).tar.gz -C build/
#	for i in $(PATCHES); do patch -d build/chillispot-1.0-coova.4 -p1 < $$i; done
#	touch build/patched

#build/chillispot-$(CHILLI_VERSION).tar.gz:
#	mkdir -p build/
#	wget $(CHILLI_URL)/chillispot-$(CHILLI_VERSION).tar.gz
#	mv chillispot-$(CHILLI_VERSION).tar.gz build/

deb/etc/chilli.conf: scripts/chilli.conf.in
	./replace.sh deb/etc/chilli.conf scripts/chilli.conf.in
deb/etc/cron.daily/chilli: scripts/chilli.cron.in
	./replace.sh deb/etc/cron.daily/chilli scripts/chilli.cron.in
deb/etc/init.d/chilli: scripts/chilli.init.in
	./replace.sh deb/etc/init.d/chilli scripts/chilli.init.in
deb/etc/default/chilli: scripts/chilli.default.in
	./replace.sh deb/etc/default/chilli scripts/chilli.default.in

#romfs:
#	$(ROMFSINST) build/coova-chilli-1.2.9/lib/libbstring.so.0.0.0 /lib/libbstring.so.0.0.0
#	$(ROMFSINST) -s /lib/libbstring.so.0.0.0 /lib/libbstring.so.0
#	$(ROMFSINST) -s /lib/libbstring.so.0.0.0 /lib/libbstring.so

clean:
	#rm -rf deb/ build/
	-$(MAKE) -C $(CHILLI_VERSION)  clean
	rm -rf deb/
	rm -f .done

.PHONY: clean


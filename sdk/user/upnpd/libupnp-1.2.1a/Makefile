# Generated automatically from Makefile.in by configure.

ROMFS_DIRS = etc/linuxigd

all:	common
	make -C upnp TARGET=$(CROSS)
#	make -C upnp/sample/pseudoicsd TARGET=$(CROSS)
	make -C upnp/sample/linux-igd TARGET=$(CROSS)

clean:	common-clean
	make -C upnp clean
#	make -C upnp/sample/pseudoicsd clean
	make -C upnp/sample/linux-igd clean
	
mrproper:	common-clean 
	make -C upnp clean
#	make -C upnp/sample/pseudoicsd clean
	make -C upnp/sample/linux-igd clean

romfs:	common
#	$(ROMFSINST) upnp/sample/pseudoicsd/pseudoicsd /bin/upnp
#	$(ROMFSINST) upnp/sample/pseudoicsd/web /etc/pseudoicsd
#	$(ROMFSINST) -s /var/run/picsdecs.xml /etc/pseudoicsd/picsdesc.xml
	$(ROMFSINST) upnp/sample/linux-igd/upnpd /bin/upnpd
	for i in $(ROMFS_DIRS); do \
		[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)/$$i; \
	done
	$(ROMFSINST) upnp/sample/linux-igd/etc/gatedesc.xml.org /etc/linuxigd
	$(ROMFSINST) upnp/sample/linux-igd/etc/gateconnSCPD.xml  /etc/linuxigd
	$(ROMFSINST) upnp/sample/linux-igd/etc/gateicfgSCPD.xml /etc/linuxigd
	$(ROMFSINST) upnp/sample/linux-igd/etc/dummy.xml /etc/linuxigd
	$(ROMFSINST) upnp/sample/linux-igd/etc/ligd.gif /etc/linuxigd
	$(ROMFSINST) upnp/sample/linux-igd/etc/upnpd.conf /etc
	$(ROMFSINST) upnp/sample/linux-igd/etc/libgcc_s.so.1 /lib
	$(ROMFSINST) -s /var/gatedesc.xml  /etc/linuxigd/gatedesc.xml

common:
#	cp -f upnp/src/inc/common.h include/upnp/common.h

common-clean:
#	rm -f include/upnp/common.h

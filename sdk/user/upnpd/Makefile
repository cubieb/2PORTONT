-include ../config.arch
#-include ../../config.path
.EXPORT_ALL_VARIABLES:

all:
	echo "***** Make libupnp-1.2.1a"
	make -C libupnp-1.2.1a	
#	cp libupnp-1.2.1/upnp/bin/libupnp.a linux-igd/.
#	cp libupnp-1.2.1/ixml/src/obj/libixml.a linux-igd/.
#	cp libupnp-1.3.1/ixml/.libs/libixml.so.2 ../../romfs/lib
#	ln -s libixml.so.2.0.0 ../../romfs/lib/libixml.so
#	ln -s libixml.so.2.0.0 ../../romfs/lib/libixml.so.2
#	cp libupnp-1.3.1/threadutil/.libs/libthreadutil.so.2 ../../romfs/lib
#	ln -s libixml.so.2.0.0 ../../romfs/lib/libthreadutil.so
#	ln -s libixml.so.2.0.0 ../../romfs/lib/libthreadutil.so.2
#	cp libupnp-1.3.1/upnp/.libs/libupnp.so.2 ../../romfs/lib
#	ln -s libupnp.so.2.0.1 ../../romfs/lib/libupnp.so
#	ln -s libupnp.so.2.0.1 ../../romfs/lib/libupnp.so.2
#	make -C linuxigd-0.95
romfs:
#	cp libupnp-1.3.1/ixml/.libs/libixml.so.2 ../../romfs/lib
#	cp libupnp-1.3.1/threadutil/.libs/libthreadutil.so.2 ../../romfs/lib
#	cp libupnp-1.3.1/upnp/.libs/libupnp.so.2 ../../romfs/lib
	for i in libupnp-1.2.1a ; do make -C $$i $@ || exit $?; done

install:
	@echo -e "\033[32mInstalling upnpd_v2 ...\033[0m"
	make -C linux-igd install

clean:
	make clean -C libupnp-1.2.1a
#	make clean -C linux-igd
#	rm -f linux-igd/*.a
	rm -f *.elf *.gdb
	
.PHONY: all clean
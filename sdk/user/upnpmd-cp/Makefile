LIBUPNP = libupnp-1.6.19
UPNPMD_CP = ctrlpt
LIBUPNP_PREFIX=$(shell pwd)/tmp

CONF_OPTS = --host=mips-linux --target=mips-linux --enable-shared=no --disable-device --prefix=$(LIBUPNP_PREFIX)
ifdef CONFIG_IPV6
CONF_OPTS += --enable-ipv6
endif

all: libupnp upnpmdcp

libupnp:
	cd $(LIBUPNP) && ./configure $(CONF_OPTS)
	$(MAKE) -C $(LIBUPNP)
	$(MAKE) -C $(LIBUPNP) install

upnpmdcp:
	$(MAKE) -C $(UPNPMD_CP)

clean:
	$(MAKE) -C $(LIBUPNP) clean
	$(MAKE) -C $(UPNPMD_CP) clean
	rm -rf $(LIBUPNP_PREFIX)

romfs:
	$(ROMFSINST)  ./$(UPNPMD_CP)/upnpmd_cp /bin/upnpmd_cp

.PHONY: all libupnp upnpmdcp clean romfs

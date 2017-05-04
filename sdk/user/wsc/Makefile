CRYPTLIB = portingssl
WSCSRC = src

all:
	make -C $(CRYPTLIB)
	make -C $(WSCSRC)
	
clean:
	make -C $(CRYPTLIB) clean
	make -C $(WSCSRC) clean

romfs:
ifeq ($(CONFIG_ONE_WPS_LED), y)
	$(ROMFSINST) wscd.conf_one_wps_led /etc/wscd.conf
else
	$(ROMFSINST) wscd.conf_e8b /etc/wscd.conf
endif
##	$(ROMFSINST) /etc/wscd.conf
	$(ROMFSINST) src/wscd /bin/wscd
	

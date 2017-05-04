.EXPORT_ALL_VARIABLES:

CUPS_ROOT=$(shell pwd)
CUPS_DIR=cups-1.0.5

#CC += -Os

all: startcupsd
	echo "CC=" $(CC)
	echo "CFLAGS=" $(CFLAGS) -DDEBUG
	echo "LDFLAGS=" $(LDFLAGS)
	@if [ -d $(CUPS_DIR) ]; then \
		$(MAKE) $(CUPS_DIR)/config.h; \
		$(MAKE) -C $(CUPS_DIR); \
	fi	

$(CUPS_DIR)/config.h: $(CUPS_DIR)/configure 
	cd $(CUPS_DIR); \
	./configure --host=mips-linux;

romfs:
	if [ ! -d $(ROMFSDIR)/etc/cups ]; then \
		mkdir $(ROMFSDIR)/etc/cups ; \
		mkdir $(ROMFSDIR)/etc/cups/conf ; \
	fi
	make -C $(CUPS_DIR) romfs
	$(ROMFSINST) startcupsd /bin/startcupsd

clean:
	make -C $(CUPS_DIR) clean
	rm -f startcupsd

startcupsd: LDFLAGS += $(LIBMIB) $(LIBVOIP)

RC_LEVEL=/etc/init.d/rc32
.PHONY: rcX
rcX:
	$(ROMFSINST) -a "mkdir -p /var/cups/" $(RC_LEVEL)
	$(ROMFSINST) -a "mkdir -p /var/cups/conf/" $(RC_LEVEL)
	$(ROMFSINST) -a "mkdir -p /var/cups/requests/" $(RC_LEVEL)
	$(ROMFSINST) -a "mkdir -p /var/cups/filter/" $(RC_LEVEL)
	$(ROMFSINST) -a "mkdir -p /var/cups/jobs/" $(RC_LEVEL)
	$(ROMFSINST) -a "cp /etc/cups/conf/* /var/cups/conf/" $(RC_LEVEL)

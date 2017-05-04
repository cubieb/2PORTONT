
.EXPORT_ALL_VARIABLES:

include ../.linux_config

DIRS = build
TARGET = solar

all:
	for i in $(DIRS) ; do make -C $$i || exit $? ; done
	make -C tools

romfs:
ifneq ($(CONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP), y)
	$(ROMFSINST)	build/all	/bin/solar
	$(ROMFSINST)	tools/solar_monitor /bin/solar_monitor
	$(ROMFSINST)	dns/dns_task /bin/dns_task
	$(ROMFSINST)	ivr/ivrserver /bin/ivrserver
endif
ifneq ($(CONFIG_RTK_VOIP_PACKAGE_867X), y)
	$(ROMFSINST)	fwupdate/fwupdate /bin/fwupdate
	$(ROMFSINST)    fwupdate/fwdownload.sh /bin/fwdownload.sh
	$(ROMFSINST)	netlink/netlink /bin/netlink
	$(ROMFSINST)	netlink/wanlink.sh /bin/wanlink.sh
endif
ifeq ($(CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT), y)
	$(ROMFSINST)	dspg_cmbs_app/cmbs_tcx/LA_DB /usr/LA_DB
endif

clean:
	-for i in $(DIRS) ; do make -C $$i clean ; done
	make -C tools clean

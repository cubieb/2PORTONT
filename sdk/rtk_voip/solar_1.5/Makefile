include .linux_config

ifndef DIR_TOOLS
DIR_TOOLS=$(DIR_LINUX)/../AP
endif

all:
	make -C flash
ifdef CONFIG_RTK_VOIP_IP_PHONE
	make -C ui
endif
ifdef CONFIG_RTK_VOIP_DECT_SITEL_SUPPORT
	make -C dect_sitel/sc1445x_src sacvm.a 
endif
ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT
	make -C src/dspg_cmbs_app
endif
ifdef CONFIG_VOIP_SDK_USER
	make -C voip_manager
  ifeq ($(CONFIG_RTK_VOIP_IVR),y)
	make -C src/ivr
  endif
  ifneq ($(CONFIG_RTK_VOIP_PACKAGE_867X), y)
	make -C src/netlink
	make -C src/fwupdate
  endif
else
	make -C src
endif
	make -C test

romfs:
	$(ROMFSINST)	solar.conf /etc/solar.conf
ifdef CONFIG_RTK_VOIP_IP_PHONE
	$(ROMFSINST)	ui/tmp/ui	/bin/ui
endif
	make -C voip_manager romfs
ifdef CONFIG_VOIP_SDK_USER
 ifdef CONFIG_RTK_VOIP_IP_PHONE
	$(ROMFSINST)    src/solar_ipphone   /bin/solar
  else
	$(ROMFSINST)	src/solar	/bin/solar
  endif
	$(ROMFSINST)	src/solar_monitor	/bin/solar_monitor
	$(ROMFSINST)	src/dns_task /bin/dns_task
	$(ROMFSINST)	src/ivr/ivrserver /bin/ivrserver
  ifneq ($(CONFIG_RTK_VOIP_PACKAGE_867X), y)
	$(ROMFSINST)	src/fwupdate/fwupdate /bin/fwupdate
	$(ROMFSINST)	src/fwupdate/fwdownload.sh /bin/fwdownload.sh
	$(ROMFSINST)	src/netlink/netlink /bin/netlink
	$(ROMFSINST)	src/netlink/wanlink.sh /bin/wanlink.sh
  endif
else
	make -C src romfs
	make -C voip_manager romfs
endif
	make -C test romfs
ifneq ($(CONFIG_RTK_VOIP_PACKAGE_867X), y)
	$(ROMFSINST)	script/autocfg.sh	/bin/autocfg.sh
endif
ifneq ($(RCSCONFIG),)
	make rcsconf
endif

clean:
ifdef CONFIG_RTK_VOIP_IP_PHONE
	make -C ui clean
endif
ifdef CONFIG_RTK_VOIP_DECT_SITEL_SUPPORT
	make -C dect_sitel/sc1445x_src clean
endif
ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT
	make -C src/dspg_cmbs_app clean
endif
ifdef CONFIG_VOIP_SDK_USER
	make -C voip_manager clean
	make -C src/ivr clean
ifneq ($(CONFIG_RTK_VOIP_PACKAGE_867X), y)
	make -C src/netlink clean
	make -C src/fwupdate clean
endif
else
	make -C src clean
endif
	make -C test clean
	make -C flash clean

rcsconf:
	echo "CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST=$(CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST)" >> $(RCSCONFIG)
	echo "CONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP=$(CONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP)" >> $(RCSCONFIG)
	echo "CONFIG_RTK_VOIP_DSP_DEVICE_NR=$(CONFIG_RTK_VOIP_DSP_DEVICE_NR)" >> $(RCSCONFIG)
	echo "CONFIG_RTK_VOIP_IP_PHONE=$(CONFIG_RTK_VOIP_IP_PHONE)" >> $(RCSCONFIG)
	echo "CONFIG_RTK_VOIP_DECT_UART_SUPPORT=$(CONFIG_RTK_VOIP_DECT_UART_SUPPORT)" >> $(RCSCONFIG)
	echo "CONFIG_APP_RTK_VOIP=$(CONFIG_RTK_VOIP)" >> $(RCSCONFIG)

_prepare:
	ln -fs $(DIR_LINUX)/include/linux/autoconf.h includes/kernel_config.h
	ln -fs $(DIR_LINUX)/rtk_voip/include/rtk_voip.h includes/rtk_voip.h
	ln -fs $(DIR_LINUX)/rtk_voip/include/voip_version.h includes/voip_version.h
	ln -fs $(DIR_LINUX)/rtk_voip/include/voip_control.h includes/voip_control.h
	ln -fs $(DIR_LINUX)/rtk_voip/include/voip_params.h includes/voip_params.h
	ln -fs $(DIR_LINUX)/rtk_voip/include/voip_feature.h includes/voip_feature.h
	ln -fs $(DIR_LINUX)/rtk_voip/include/voip_types.h includes/voip_types.h
	ln -fs $(DIR_LINUX)/rtk_voip/include/voip_debug.h includes/voip_debug.h
	ln -fs $(DIR_LINUX)/rtk_voip/include/voip_errno.h includes/voip_errno.h
	ln -fs $(DIR_LINUX)/acmw_lx/include/userdef.h includes/acmw_userdef.h
	ln -fs $(DIR_LINUX)/rtk_voip/srtp/crypto/include/crypto_types.h includes/crypto_types.h
	ln -fs $(DIR_LINUX)/include/linux/rtnetlink.h includes/rtnetlink.h
	ln -fs $(DIR_LINUX)/include/linux/netlink.h includes/netlink.h
	ln -fs $(DIR_LINUX)/include/linux/if_link.h includes/if_link.h
	#tar -zxvf var.tgz

prepare:
	@if [ ! -e includes/kernel_config.h ]; then	\
		make _prepare;	\
	fi

#REMOVE BEGIN
	@if [ ! -f $(DIR_LINUX)/rtk_voip/include/svn_version.h ]; then \
		touch $(DIR_LINUX)/rtk_voip/include/svn_version.h; \
	fi

	@if [ ! -f includes/svn_version.h ]; then \
		ln -s $(DIR_LINUX)/rtk_voip/include/svn_version.h includes/svn_version.h; \
	fi

ifeq ($(CONFIG_RTK_VOIP_PACKAGE_867X), y)
		$(DIR_USERS)/rtk_voip/mksvnver_iad.sh $(DIR_ROOT)
else
		$(DIR_USERS)/rtk_voip/mksvnver.sh $(DIR_ROOT)
endif
#REMOVE END

ifdef CONFIG_RTK_VOIP_IP_PHONE
	@if [ ! -e include/ui_flash_layout.h ]; then	\
		make -C ui dep;	\
	fi
endif

	


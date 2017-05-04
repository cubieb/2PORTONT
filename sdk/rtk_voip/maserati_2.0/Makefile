include .linux_config
include Makefile.rcm

ifndef DIR_TOOLS
DIR_TOOLS=$(DIR_LINUX)/../AP
endif

RC_LEVEL=/etc/init.d/rc34
RC_DEVICE_LEVEL=/etc/init.d/rc3



all:
ifeq ($(OPENWRT_BUILD), 1)
	make -C flash
	make -C src
	make -C voip_manager
	make -C misc
	make -C gwdt
else
	make -C flash
#	make -C rtk_tr104
ifdef CONFIG_RTK_VOIP_DECT_SITEL_SUPPORT
	make -C dect_sitel/sc1445x_src sacvm.a 
endif
ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT
	make -C src/dspg_cmbs_app
endif
ifdef RCM_SIMULATION
	make -C cmsim
endif
ifdef CONFIG_VOIP_SDK_TEMP
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
ifdef CONFIG_RTK_VOIP_IP_PHONE
	make -C ui
endif
	make -C misc
	make -C test
	make -C gwdt
endif # $(OPENWRT_BUILD)

ecos-all:
	make -C voip_manager ecos-all
	make -C test ecos-all

romfs:
	# rock: let solar.conf can be modified in read-only fs
	$(ROMFSINST)	solar.conf /etc/solar.conf.in
	$(ROMFSINST)	-s /var/solar.conf  /etc/solar.conf
ifdef CONFIG_RTK_VOIP_IP_PHONE
	$(ROMFSINST)	ui/tmp/ui	/bin/ui
endif
	make -C voip_manager romfs
	make -C src/ivr romfs
	make -C gwdt romfs
ifdef CONFIG_VOIP_SDK_TEMP
 ifdef CONFIG_RTK_VOIP_IP_PHONE
	$(ROMFSINST)    src/solar_ipphone   /bin/solar
  else
	$(ROMFSINST)	src/solar	/bin/solar
  endif
	$(ROMFSINST)	src/solar_monitor	/bin/solar_monitor
#	$(ROMFSINST)	src/dns_task /bin/dns_task
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
	make -C misc romfs 
	make -C test romfs
ifneq ($(CONFIG_RTK_VOIP_PACKAGE_867X), y)
	$(ROMFSINST)	script/autocfg.sh	/bin/autocfg.sh
endif
ifneq ($(RCSCONFIG),)
	make rcsconf
endif

.PHONY: rcX
rcX:
	$(ROMFSINST) rc_dev_voip               /etc/scripts/rc_dev_voip
	$(ROMFSINST) -a "/etc/scripts/rc_dev_voip"     $(RC_DEVICE_LEVEL)
ifndef CONFIG_LUNA_DUAL_LINUX
ifndef CONFIG_RTK_VOIP_STANDALONE_ARCH
	$(ROMFSINST) -a "/etc/rc_boot_dsp" $(RC_DEVICE_LEVEL)
endif
endif
	$(ROMFSINST) rc_voip.aipc              /etc/scripts/rcm_voip
	$(ROMFSINST) -a "/etc/scripts/rcm_voip&"        $(RC_LEVEL)
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
ifdef CONFIG_VOIP_SDK
	make -C voip_manager clean
endif
ifdef CONFIG_VOIP_SDK_TEMP
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
	make -C gwdt clean
	make -C misc clean
	make unprepare
ifdef RCM_SIMULATION
	make -C cmsim clean
	-find $(ROMFSDIR)/etc -name solar.conf          | xargs -r rm -rf
	-find $(ROMFSDIR)/lib -name libvoip_manager.so  | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name fxo                 | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name rtp                 | xargs -r rm -rf
#	-find $(ROMFSDIR)/bin -name solar               | xargs -r rm -rf
#	-find $(ROMFSDIR)/bin -name solar_monitor       | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name dns_task            | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name ivrserver           | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name voipbox             | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name dbg                 | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name fskgen              | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name pulse_dial          | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name cp3_measure         | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name reg                 | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name ram                 | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name slic_reset          | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name switchmii           | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name vmwigen             | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name rtcp_statistic      | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name clone_mac           | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name bandwidth_mgr       | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name gpio_init           | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name gpio_read           | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name gpio_write          | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name send_2833           | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name voipcli             | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name tone                | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name playtone            | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name rtptalk             | xargs -r rm -rf
	-find $(ROMFSDIR)/bin -name autocfg.sh          | xargs -r rm -rf
endif

rcsconf:
	echo "CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST=$(CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST)" >> $(RCSCONFIG)
	echo "CONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP=$(CONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP)" >> $(RCSCONFIG)
	echo "CONFIG_RTK_VOIP_DSP_DEVICE_NR=$(CONFIG_RTK_VOIP_DSP_DEVICE_NR)" >> $(RCSCONFIG)
	echo "CONFIG_RTK_VOIP_IP_PHONE=$(CONFIG_RTK_VOIP_IP_PHONE)" >> $(RCSCONFIG)
	echo "CONFIG_RTK_VOIP_DECT_UART_SUPPORT=$(CONFIG_RTK_VOIP_DECT_UART_SUPPORT)" >> $(RCSCONFIG)
	echo "CONFIG_APP_RTK_VOIP=$(CONFIG_RTK_VOIP)" >> $(RCSCONFIG)

ifneq ($(OPENWRT_BUILD), 1)
_prepare:
ifeq ($(CONFIG_DEFAULTS_KERNEL_2_6),y)
	ln -nfs $(DIR_LINUX)/include/linux/autoconf.h includes/kernel_config.h
else
ifeq ($(CONFIG_DEFAULTS_KERNEL_3_4),y)
	ln -nfs $(DIR_LINUX)/include/generated/autoconf.h includes/kernel_config.h
endif
ifeq ($(CONFIG_DEFAULTS_KERNEL_3_18),y)
	ln -nfs $(DIR_LINUX)/include/generated/autoconf.h includes/kernel_config.h
endif
endif
	ln -nfs $(DIR_LINUX)/rtk_voip/include/rtk_voip.h includes/rtk_voip.h
	#ln -nfs $(DIR_LINUX)/rtk_voip/include/voip_version.h includes/voip_version.h
	ln -nfs $(DIR_LINUX)/rtk_voip/include/voip_control.h includes/voip_control.h
	ln -nfs $(DIR_LINUX)/rtk_voip/include/voip_params.h includes/voip_params.h
	ln -nfs $(DIR_LINUX)/rtk_voip/include/voip_feature.h includes/voip_feature.h
	ln -nfs $(DIR_LINUX)/rtk_voip/include/voip_types.h includes/voip_types.h
	ln -nfs $(DIR_LINUX)/rtk_voip/include/voip_debug.h includes/voip_debug.h
	ln -nfs $(DIR_LINUX)/rtk_voip/include/voip_errno.h includes/voip_errno.h
	#ln -nfs $(DIR_LINUX)/acmw_lx/include/userdef.h includes/acmw_userdef.h
	ln -nfs $(DIR_LINUX)/rtk_voip/srtp/crypto/include/crypto_types.h includes/crypto_types.h
	ln -nfs $(DIR_LINUX)/include/linux/rtnetlink.h includes/rtnetlink.h
	ln -nfs $(DIR_LINUX)/include/linux/netlink.h includes/netlink.h
	ln -nfs $(DIR_LINUX)/include/linux/if_link.h includes/if_link.h
#common include
	ln -nfs common/rcm_common_tapi.h includes/rcm_tapi.h
	ln -nfs common/rcm_flash_client.h includes/voip_flash_client.h
	ln -nfs common/rcm_flash.h includes/voip_flash.h
	ln -nfs common/rcm_common_defs.h includes/rcm_sim_defs.h
	#tar -zxvf var.tgz
	ln -nfs $(ROOTDIR)/rtk_voip/voip_manager_1.6 voip_manager
	ln -nfs $(ROOTDIR)/rtk_voip/voip_manager_1.6/includes/voip_manager.h includes/voip_manager.h
	ln -nfs $(ROOTDIR)/user/boa/src/LINUX/form_src/multilang.h includes/multilang.h

prepare:
	make _prepare


ifdef CONFIG_RTK_VOIP_IP_PHONE
	@if [ ! -e ui/include/ui_flash_layout.h ]; then	\
		make -C ui dep;	\
	fi
endif
	
prepare:
	make _prepare

_unprepare:
ifeq ($(CONFIG_DEFAULTS_KERNEL_2_6),y)
	[ -L includes/kernel_config.h ] && unlink includes/kernel_config.h
else
ifeq ($(CONFIG_DEFAULTS_KERNEL_3_4),y)
	[ -L includes/kernel_config.h ] && unlink includes/kernel_config.h
endif
ifeq ($(CONFIG_DEFAULTS_KERNEL_3_18),y)
	[ -L includes/kernel_config.h ] && unlink includes/kernel_config.h
endif
endif
	[ -L includes/rtk_voip.h      ] && unlink includes/rtk_voip.h     
	[ -L includes/voip_control.h  ] && unlink includes/voip_control.h 
	[ -L includes/voip_params.h   ] && unlink includes/voip_params.h  
	[ -L includes/voip_feature.h  ] && unlink includes/voip_feature.h 
	[ -L includes/voip_types.h    ] && unlink includes/voip_types.h   
	[ -L includes/voip_debug.h    ] && unlink includes/voip_debug.h   
	[ -L includes/voip_errno.h    ] && unlink includes/voip_errno.h   
	[ -L includes/crypto_types.h  ] && unlink includes/crypto_types.h 
	[ -L includes/rtnetlink.h     ] && unlink includes/rtnetlink.h    
	[ -L includes/netlink.h       ] && unlink includes/netlink.h      
	[ -L includes/if_link.h       ] && unlink includes/if_link.h      
	[ -L includes/voip_manager.h  ] && unlink includes/voip_manager.h
	[ -L includes/rcm_tapi.h          ] && unlink includes/rcm_tapi.h         
	[ -L includes/voip_flash_client.h ] && unlink includes/voip_flash_client.h
	[ -L includes/voip_flash.h        ] && unlink includes/voip_flash.h       
	[ -L includes/rcm_sim_defs.h      ] && unlink includes/rcm_sim_defs.h     
#	[ -L .linux_config            ] && unlink .linux_config           
	[ -L voip_manager             ] && unlink voip_manager            

unprepare:
	make _unprepare
endif #$(OPENWRT_BUILD)

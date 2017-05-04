include Makeparm.cfg
include Makeparm.def

all: 
	make -C $(MANAGER_PATH)
	make -C $(SIP_PATH)
	make -C $(RTP_PATH)
	make -C $(EXOSIP_PATH)
	make -C $(LINPHONE_PATH)
#	make -C $(STUN_PATH)

# ifdef CONFIG_RTK_VOIP_IVR
#	make -C $(IVR_PATH)
#  endif

  ifneq ($(BUILD_DIALPLAN), 0)
	make -C $(DIALPLAN_PATH)
  endif 
#  ifeq ($(RTK_DNS), YES)
#	make -C $(DNS_PATH)
#  endif
	make -C $(CSV_PARSER_PATH)
	make -C $(ATA_SYSTEM_PATH)
ifdef CONFIG_USER_TR104
	$(XCC) -c -o $(CWMPEVT) $(CFLAGS) $(EXTRACFLAGS) ../../rtk_tr104/cwmpevt.c
  endif

  ifdef RCM_SIMULATION
	$(XCC) -g $(LDFLAGS) -Wl,-Map,solar.map -o $@ $(LIBS) ../../flash/libvoip_flash_client.a ../../cmsim/librcm_sim.a $(CWMPEVT)
  else
	$(XCC) -g $(LDFLAGS) -o $@ $(LIBS) ../../flash/libvoip_flash_client.a $(CWMPEVT)
  endif
	#-cp all /cvs/nfsdir/solar
	#-cp all /mnt/rock_nfs/solar
	cp all solar
ifneq ($(OPENWRT_BUILD), 1)
  ifdef RCM_SIMULATION
    :
  else
	$(STRIP) all
  endif
endif
  ifdef CONFIG_RTK_VOIP_IP_PHONE
	cp all solar_ipphone
  endif

  ifdef RCM_SIMULATION
    :
  else
	make -C $(NETLINK_PATH)
  endif

clean:
	make -C $(MANAGER_PATH) clean
	make -C $(SIP_PATH) clean
	make -C $(RTP_PATH) clean
	make -C $(EXOSIP_PATH) clean
	make -C $(LINPHONE_PATH) clean
#	make -C $(STUN_PATH) clean
  ifdef CONFIG_RTK_VOIP_IVR
	make -C $(IVR_PATH) clean
  endif
  ifneq ($(BUILD_DIALPLAN), 0)
	make -C $(DIALPLAN_PATH) clean
  endif 
#	make -C $(DNS_PATH) clean
	make -C $(ATA_SYSTEM_PATH) clean
	make -C $(NETLINK_PATH) clean
	make -C $(CSV_PARSER_PATH) clean
	-rm all
	-rm solar
	-rm voip_flash_client.o
  ifdef CONFIG_APP_TR104
	-rm $(CWMPEVT)
  endif
  ifdef RCM_SIMULATION
	find ../ -name *.lst | xargs rm -rf
	find ../ -name *.map | xargs rm -rf
  endif
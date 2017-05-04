include Makeparm.cfg
include Makeparm.def

all: 
	make -C $(MANAGER_PATH)
	make -C $(SIP_PATH)
	make -C $(RTP_PATH)
	make -C $(EXOSIP_PATH)
	make -C $(LINPHONE_PATH)
	make -C $(STUN_PATH)
  ifdef CONFIG_RTK_VOIP_IVR
	make -C $(IVR_PATH)
  endif
  ifneq ($(BUILD_DIALPLAN), 0)
	make -C $(DIALPLAN_PATH)
  endif 
  ifeq ($(RTK_DNS), YES)
	make -C $(DNS_PATH)
  endif
	make -C $(ATA_SYSTEM_PATH)
  ifdef CONFIG_APP_TR104
	$(XCC) -c -o $(CWMPEVT) $(CFLAGS) $(EXTRACFLAGS) ../../tr104/cwmpevt.c
  endif
	$(XCC) -g $(LDFLAGS) -o $@ $(LIBS) ../../flash/libvoip_flash_client.a $(CWMPEVT)
	#-cp all /cvs/nfsdir/solar
	#-cp all /mnt/rock_nfs/solar
	cp all solar
	$(STRIP) all
  ifdef CONFIG_RTK_VOIP_IP_PHONE
	cp all solar_ipphone
  endif
	make -C $(NETLINK_PATH)
	make -C $(FWUPDATE_PATH)
clean:
	make -C $(MANAGER_PATH) clean
	make -C $(SIP_PATH) clean
	make -C $(RTP_PATH) clean
	make -C $(EXOSIP_PATH) clean
	make -C $(LINPHONE_PATH) clean
	make -C $(STUN_PATH) clean
  ifdef CONFIG_RTK_VOIP_IVR
	make -C $(IVR_PATH) clean
  endif
  ifneq ($(BUILD_DIALPLAN), 0)
	make -C $(DIALPLAN_PATH) clean
  endif 
	make -C $(DNS_PATH) clean
	make -C $(ATA_SYSTEM_PATH) clean
	make -C $(NETLINK_PATH) clean
	make -C $(FWUPDATE_PATH) clean
	-rm all
	-rm solar
	-rm voip_flash_client.o
  ifdef CONFIG_APP_TR104
	-rm $(CWMPEVT)
  endif

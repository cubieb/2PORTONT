ifeq ($(OPENWRT_BUILD), 1)
include $(TOPDIR)/rtk_voip/maserati_2.0/.linux_config
CFLAGS += -I$(TOPDIR)/rtk_voip/maserati_2.0/includes
LDFLAGS += -L$(TOPDIR)/rtk_voip/maserati_2.0/voip_manager -lvoip_manager

CFLAGS += -Werror

all:
	$(CC) ring.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o ring
	$(CC) phone.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o phone
	$(CC) rtp.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o rtp
	$(CC) rtp2.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o rtp2
	$(CC) rtp_red.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o rtp_red
#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
	$(CC) rtp_v6.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o rtp_v6
	$(CC) t38_v6.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o t38_v6
#endif
	$(CC) daa.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o daa
	$(CC) caller_id.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o caller_id
	$(CC) fxo.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o fxo
	$(CC) fxo_int.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o fxo_int
	$(CC) fxo_dual.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o fxo_dual
	$(CC) conference.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o conference
	$(CC) custom_tone.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o custom_tone	
	$(CC) country_tone_update.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o country_tone_update
	$(CC) fxs.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o fxs
	$(CC) fxs_int.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o fxs_int
	$(CC) pulse_dial.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o pulse_dial
	$(CC) pcm_dump.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o pcm_dump
	$(CC) gain_test.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o gain_test
	$(CC) sip_reg_led.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o sip_reg_led
	$(CC) fxo_callerid_det.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o fxo_callerid_det
	$(CC) dis_tone.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o dis_tone
	$(CC) dtmfdet_dump.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o dtmfdet_dump
	$(CC) send_2833.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o send_2833
	$(CC) proslic_set.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o proslic_set
	$(CC) fskgen.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o fskgen
	$(CC) fsk_date.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o fsk_date
	$(CC) vmwigen.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o vmwigen
	$(CC) led_ctrl.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o led_ctrl
	$(CC) dtmf_det_cfg.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o dtmf_det_cfg
	$(CC) rfc2833.c	$(CFLAGS) $(LIBS) $(LDFLAGS) -o rfc2833
	$(CC) dtmf_det_test.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o dtmf_det_test
	$(CC) detbuf2_dump.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o detbuf2_dump
	$(CC) genMeter.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o genMeter
	$(CC) portdetect.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o portdetect	
	$(CC) rtp_rtcp_stats.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o rtp_rtcp_stats
	$(CC) fax_ex.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o fax_ex
	$(CC) t38_get.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o t38_get	
	$(CC) phoneoff.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o phoneoff
	$(CC) calibrate.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o calibrate
	$(CC) offhook_polarity.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o  offhook_polarity
	$(CC) loopback.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o pcmloopback
else
include $(ROOTDIR)/user/rtk_voip/.linux_config

CFLAGS += -g -I$(DIR_USERS)/rtk_voip/includes
#LDFLAGS += --static
#LIBS = ../../../voip_manager/libvoip_manager.a
#LDFLAGS += -L../../../voip_manager -lvoip_manager
LDFLAGS += -L$(DIR_USERS)/rtk_voip/voip_manager -lvoip_manager

ifdef CONFIG_RTK_VOIP_IP_PHONE
CFLAGS += -I../../../ui/include
endif

ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT
LDFLAGS += -lpthread -ldl
endif

CFLAGS += $(ECOS_SHELL_CFLAGS)

CFLAGS += -Werror

#----------------------------------------------------------------------
all:
	$(CC) ring.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o ring
	$(CC) phone.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o phone
	$(CC) rtp.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o rtp
	$(CC) rtp2.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o rtp2
	$(CC) rtp_red.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o rtp_red
#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
	$(CC) rtp_v6.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o rtp_v6
	$(CC) t38_v6.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o t38_v6
#endif
	$(CC) daa.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o daa
	$(CC) caller_id.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o caller_id
	$(CC) fxo.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o fxo
	$(CC) fxo_int.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o fxo_int
	$(CC) fxo_dual.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o fxo_dual
	$(CC) conference.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o conference
	$(CC) custom_tone.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o custom_tone	
	$(CC) country_tone_update.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o country_tone_update
	$(CC) fxs.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o fxs
	$(CC) fxs_int.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o fxs_int
	$(CC) pulse_dial.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o pulse_dial
	$(CC) pcm_dump.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o pcm_dump
	$(CC) gain_test.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o gain_test
	$(CC) sip_reg_led.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o sip_reg_led
	$(CC) fxo_callerid_det.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o fxo_callerid_det
	$(CC) dis_tone.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o dis_tone
	$(CC) dtmfdet_dump.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o dtmfdet_dump
	$(CC) send_2833.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o send_2833
	$(CC) proslic_set.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o proslic_set
	$(CC) fskgen.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o fskgen
	$(CC) fsk_date.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o fsk_date
	$(CC) vmwigen.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o vmwigen
	$(CC) led_ctrl.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o led_ctrl
	$(CC) dtmf_det_cfg.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o dtmf_det_cfg
	$(CC) rfc2833.c	$(CFLAGS) $(LIBS) $(LDFLAGS) -o rfc2833
	$(CC) dtmf_det_test.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o dtmf_det_test
	$(CC) detbuf2_dump.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o detbuf2_dump
	$(CC) genMeter.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o genMeter
	$(CC) portdetect.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o portdetect	
	$(CC) rtp_rtcp_stats.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o rtp_rtcp_stats
	$(CC) fax_ex.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o fax_ex
	$(CC) t38_get.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o t38_get	
	$(CC) phoneoff.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o phoneoff
	$(CC) calibrate.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o calibrate
	$(CC) offhook_polarity.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o  offhook_polarity
	$(CC) loopback.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o pcmloopback

ecos-shell:
	$(CC) rtp.c $(CFLAGS) -c -o rtp.o 

romfs:
	$(ROMFSINST) fxs /bin/fxs
	$(ROMFSINST) fxs_int /bin/fxs_int
	$(ROMFSINST) fxo /bin/fxo
	$(ROMFSINST) fxo_int /bin/fxo_int
	$(ROMFSINST) fxo_dual /bin/fxo_dual
	$(ROMFSINST) rtp /bin/rtp
	$(ROMFSINST) rtp2 /bin/rtp2
	$(ROMFSINST) rtp_red /bin/rtp_red
ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
	$(ROMFSINST) rtp_v6 /bin/rtp_v6
	$(ROMFSINST) t38_v6 /bin/t38_v6
endif
	$(ROMFSINST) t38_get /bin/t38_get
	$(ROMFSINST) pcm_dump /bin/pcm_dump
	$(ROMFSINST) caller_id /bin/caller_id
	$(ROMFSINST) dtmfdet_dump /bin/dtmfdet_dump
	$(ROMFSINST) send_2833 /bin/send_2833
	$(ROMFSINST) proslic_set /bin/proslic_set
	$(ROMFSINST) fskgen /bin/fskgen
	$(ROMFSINST) fsk_date /bin/fsk_date
	$(ROMFSINST) vmwigen /bin/vmwigen
	$(ROMFSINST) led_ctrl /bin/led_ctrl
	$(ROMFSINST) dtmf_det_cfg /bin/dtmf_det_cfg
	$(ROMFSINST) conference /bin/conference
	$(ROMFSINST) rfc2833 /bin/rfc2833
	$(ROMFSINST) genMeter /bin/genMeter	
	$(ROMFSINST) portdetect /bin/portdetect	
	$(ROMFSINST) custom_tone /bin/custom_tone
	$(ROMFSINST) country_tone_update /bin/country_tone_update
	$(ROMFSINST) rtp_rtcp_stats /bin/rtp_rtcp_stats
	$(ROMFSINST) fax_ex /bin/fax_ex
	$(ROMFSINST) phoneoff /bin/phoneoff
	$(ROMFSINST) calibrate /bin/calibrate
	$(ROMFSINST) offhook_polarity /bin/offhook_polarity		
	$(ROMFSINST) pcmloopback /bin/pcmloopback	

clean:
	rm -f *.o ring phone rtp rtp2 rtp_red rtp_v6 t38_v6 daa caller_id vmwi fxo fxo_int conference fxs fxs_int custom_tone country_tone_update pulse_dial pcm_dump gain_test sip_reg_led fxo_callerid_det dis_tone dtmfdet_dump send_2833 proslic_set fskgen vmwigen led_ctrl dtmf_det_cfg rtp_rtcp_stats rfc2833 genMeter portdetect fxo_dual fsk_date detbuf2_dump dtmf_det_test fax_ex phoneoff calibrate offhook_polarity pcmloopback t38_get

endif # $(OPENWRT_BUILD)

# Naming our phony targets
.PHONY: clean install

#CROSSCOMPILE_FLAGS = -mcr16cplus -mdata-model=far -mint32 -fPIC

PWD = $(shell pwd)

EXOSIP_DIR = $(PWD)/exosip2
SYSLOG_DIR =$(PWD)/../syslog
PRINT_API_DIR = $(PWD)/../si_print_api
MCU_DIR = $(PWD)/../mcu
OSIP_DIR = $(PWD)/../../lib/libosip2-3.3.0/release
ORTP_DIR = $(PWD)/../../lib/ortp-0.13.1/release
ALSA_DIR = $(PWD)/../../lib/alsa-lib-1.0.13/release
FXS_DIR = $(PWD)/SiATA
COMMON_DIR = $(PWD)/common/
RSS_APPLICATION_DIR = $(PWD)/../RSSFeed
RADIO_APP_DIR = $(PWD)/../radio
RSSFEED_DIR = $(PWD)/../../lib/RSSFeedLib/release
SRTP_DIR = $(PWD)/../../lib/srtp-1.4.2/release
#OPENSSL_DIR = $(PWD)/../../lib/openssl-0.9.8j/release

#TFT SPECIFIC LIBS
JPEG_LIB = $(PWD)/../../lib/libjpeg/release
PNG_LIB = $(PWD)/../../lib/libpng-1.2.40/release
TRUETYPE_LIB = $(PWD)/../../lib/libfreetype-2.3.9/release
UNGIF_LIB = $(PWD)/../../lib/libungif-4.1.4/release

#NET RADIO LIBS
LIBICONV_DIR = $(PWD)/../../lib/libiconv-1.12/release
LIBID3TAG_DIR = $(PWD)/../../lib/libid3tag-0.15.1b/release
LIBZ_DIR = $(PWD)/../../lib/libz-1.1.4/release
LIBFLAC_DIR = $(PWD)/../../lib/codecs/flac-1.2.1/release
LIBCR16MAD_DIR = $(PWD)/../../lib/codecs/libcr16mad/release
LIBOGG_DIR = $(PWD)/../../lib/codecs/libogg-1.1.3/release
LIBVORBIS_DIR = $(PWD)/../../lib/codecs/libvorbis-1.2.0/release

#ATA DEFINITIONS
ATA_DIR = $(PWD)/SiATA
DISTR_DIR = $(PWD)/../../../../..
LEG_CODE_DIR = $(DISTR_DIR)/linux-2.6.x_removed/drivers/char/sc1445x_ata

#T38 DEFINITIONS
T38_DIR  = $(PWD)/../../lib/T38Lib/release

#DECT DEFINITIONS
DECT_DIR = $(PWD)/SiDECT

CFLAGS		=	$(CROSSCOMPILE_FLAGS) -c -Wall -O2 -g2	\
				-I $(EXOSIP_DIR)/include				\
				-I $(OSIP_DIR)/include					\
				-I $(EXOSIP_DIR)/include				\
 				-I $(ORTP_DIR)/include					\
				-I $(ALSA_DIR)/include					\
				-I $(RSSFEED_DIR)/include				\
				-I $(SRTP_DIR)/include					\
				-I $(LIBOGG_DIR)/include				\
				-I $(LIBICONV_DIR)/include				\
				-I $(LIBID3TAG_DIR)/include				\
				-I $(LIBZ_DIR)/include					\
				-I $(LIBFLAC_DIR)/include				\
				-I $(LIBCR16MAD_DIR)/include			\
				-I $(LIBVORBIS_DIR)/include				\
				-I $(PRINT_API_DIR)/					\
				-DCC_CONSOLE							\
				-DMCU_USE_INTERNAL_CONFIG				\
 				-DUSE_FLASH			
 				
CFLAGS_TFT	=	-I $(JPEG_LIB)/include \
				-I $(PNG_LIB)/include \
				-I $(TRUETYPE_LIB)/include \
				-I $(TRUETYPE_LIB)/include/freetype2 \
				-I $(UNGIF_LIB)/include
				 					
CFLAGS_ATA	=	$(CROSSCOMPILE_FLAGS) -c -Wall -O2 -g2	\
				-I $(EXOSIP_DIR)/include				\
				-I $(OSIP_DIR)/include					\
				-I $(EXOSIP_DIR)/include				\
 				-I $(ORTP_DIR)/include					\
				-I $(ALSA_DIR)/include					\
				-I $(SRTP_DIR)/include					\
				-I $(LEG_CODE_DIR)/api_lib/includes/	\
				-I $(LEG_CODE_DIR)/arch/cr16/			\
				-I $(LEG_CODE_DIR)/api_lib/vp890_api	\
				-I $(LEG_CODE_DIR)/vpmod				\
				-I $(FXS_DIR)							\
				-I $(T38_DIR)/include					\
				-I $(PRINT_API_DIR)/					\
				-DCC_CONSOLE							\
				-DVM_DEBUG								\
				-DVP_DEBUG								\
				-DVM_HAL_DEBUG_							\
 				-DUSE_FLASH								\
 				-DMCU_USE_INTERNAL_CONFIG				\
 				-DUSE_PCM								
																																
CFLAGS_DECT_ONLY	=	-I $(DECT_DIR)/							\
						-I $(DECT_DIR)/natalie/					\
						-D__USE_POSIX199309 
				
CFLAGS_DECT	=	$(CROSSCOMPILE_FLAGS) -c -Wall -O2 -g2	\
				-I $(EXOSIP_DIR)/include				\
				-I $(OSIP_DIR)/include					\
				-I $(EXOSIP_DIR)/include				\
 				-I $(ORTP_DIR)/include					\
				-I $(ALSA_DIR)/include					\
				-I $(SRTP_DIR)/include					\
				-I $(DECT_DIR)/							\
				-I $(DECT_DIR)/natalie/					\
				-I $(COMMON_DIR)						\
				-D__USE_POSIX199309						\
				-I $(PRINT_API_DIR)/					\
 				-DCC_CONSOLE							\
 				-DUSE_PCM								\
 				-DMCU_USE_INTERNAL_CONFIG				\
 				-DUSE_FLASH	

CFLAGS_SA_CVM =	$(CROSSCOMPILE_FLAGS) -c -Wall -O2 -g	\
				-I $(COMMON_DIR)						\
				-DUSE_PCM								\
				-DMCU_USE_INTERNAL_CONFIG				\
				-DSA_CVM

CFLAGS_SA_CVM_A = 	$(CFLAGS_SA_CVM) -fPIC				\
					-DSA_CVM_NO_MAIN					\
					-DCONFIG_RTK_VOIP
			
LDFLAGS		= $(CROSSCOMPILE_FLAGS) -fno-rtti -Wl,--traditional-format,-static -fPIC -L $(EXOSIP_DIR)/lib -L $(OSIP_DIR)/lib						\
				-L $(ORTP_DIR)/lib  -L $(ALSA_DIR)/lib -L /usr/local/lib -L $(RSSFEED_DIR)/lib -L $(SRTP_DIR)/lib -L $(LIBICONV_DIR)/lib	\
				-L $(LIBID3TAG_DIR)/lib	-L $(LIBZ_DIR)/lib -L $(LIBFLAC_DIR)/lib -L $(LIBCR16MAD_DIR)/lib -L $(LIBOGG_DIR)/lib -L $(LIBVORBIS_DIR)/lib			\
				-losipparser2 -losip2  -lortp -lasound -lcurl -lmxml -lsrtp						\
				-liconv -lFLAC -lvorbisenc -lvorbisfile -lvorbis -logg -lcr16mad -lrt -lid3tag -lm -lz -lpthread
				
LDFLAGS_NO_MMI = $(CROSSCOMPILE_FLAGS) -fno-rtti -Wl,--traditional-format,-static -fPIC	\
				-L $(EXOSIP_DIR)/lib -L $(OSIP_DIR)/lib				\
				-L $(ORTP_DIR)/lib  -L $(ALSA_DIR)/lib -L /usr/local/lib -L $(SRTP_DIR)/lib 	\
				-losipparser2 -losip2 -lortp -lasound -lpthread -lsrtp						

LDFLAGS_SACVM = $(CROSSCOMPILE_FLAGS) -fno-rtti -Wl,--traditional-format,-static -fPIC	\
				-lpthread
				
LDFLAGS_TFT	 = -L $(JPEG_LIB)/lib -L $(PNG_LIB)/lib -L $(TRUETYPE_LIB)/lib -L $(UNGIF_LIB)/lib -ljpeg -lfreetype -lungif -lpng
				
LDFLAGS_DECT = -lrt 

LDFLAGS_ATA = -L $(T38_DIR)/lib -L $(T38_DIR)/lib -lall_t38 -lfxr_t38 

PROJECT_DIR     = .

TUI_SOURCES			=	TUI/phone_commands.c TUI/sc1445x_gui_main.c TUI/scr_main.c TUI/scr_main_state_machine.c \
						TUI/scr_mainmenu.c TUI/scr_optionbox.c TUI/scr_inputbox.c TUI/scr_double_inputbox.c \
						TUI/scr_barbox.c TUI/scr_phbook.c TUI/scr_calls_log.c TUI/obj_buttons.c TUI/obj_info.c \
						TUI/obj_list.c TUI/obj_option.c TUI/obj_input.c TUI/scr_confirmationbox.c \
						TUI/obj_bar.c TUI/menu_functions.c TUI/lcd_driver_api.c TUI/kbd_driver_api.c

DOTMATRIX_SOURCES	=	GUI/phone_commands.c GUI/sc1445x_gui_main.c GUI/scr_main.c GUI/scr_main_state_machine.c \
						GUI/scr_mainmenu.c GUI/scr_optionbox.c GUI/scr_inputbox.c GUI/scr_double_inputbox.c \
						GUI/scr_barbox.c GUI/scr_phbook.c GUI/scr_calls_log.c GUI/obj_buttons.c GUI/obj_info.c \
						GUI/obj_list.c GUI/obj_option.c GUI/obj_input.c GUI/scr_confirmationbox.c \
						GUI/obj_bar.c GUI/menu_functions.c GUI/lcd_driver_api.c GUI/kbd_driver_api.c 

TFT_SOURCES			=	TFT_GUI/phone_commands.c TFT_GUI/sc1445x_gui_main.c TFT_GUI/scr_main.c TFT_GUI/scr_main_state_machine.c \
						TFT_GUI/scr_mainmenu.c TFT_GUI/scr_optionbox.c TFT_GUI/scr_inputbox.c TFT_GUI/scr_double_inputbox.c \
						TFT_GUI/scr_barbox.c TFT_GUI/scr_phbook.c TFT_GUI/scr_calls_log.c TFT_GUI/obj_buttons.c TFT_GUI/obj_info.c \
						TFT_GUI/obj_list.c TFT_GUI/obj_option.c TFT_GUI/obj_input.c TFT_GUI/scr_confirmationbox.c \
						TFT_GUI/obj_bar.c TFT_GUI/menu_functions.c TFT_GUI/kbd_driver_api.c \
						TFT_GUI/tft_fbv_api.c TFT_GUI/jpeg.c TFT_GUI/bmp.c TFT_GUI/gif.c TFT_GUI/png.c \
						TFT_GUI/tft_vfonts.c TFT_GUI/tft_bfonts.c TFT_GUI/scr_rss.c TFT_GUI/scr_radio.c TFT_GUI/scr_ims.c TFT_GUI/scr_dect_info.c\
						$(COMMON_DIR)/si_report_weather.c
						
PHONE_SOURCES		=	SiPhoneUA/si_ua_instantmessage.c SiPhoneUA/si_user_agent.c SiPhoneUA/si_ua_server_api.c SiPhoneUA/si_ua_init.c \
						SiPhoneUA/si_ua_registrar.c SiPhoneUA/si_ua_callcontrol.c SiPhoneUA/si_ua_hold.c SiPhoneUA/si_ua_dtmf.c	\
						SiPhoneUA/si_ua_notify.c SiPhoneUA/si_ua_events.c SiPhoneUA/si_ua_parser.c \
						SiPhoneUA/si_ua_transfer.c SiPhoneUA/si_sdp_negotiation.c \
  						SiPhoneUA/si_ua_api.c SiPhoneUA/si_ua_dns.c \
						SiPhoneUA/si_sc_manager.c SiPhoneUA/si_sc_tone.c \
						SiPhoneUA/si_ua_stun.c SiPhoneUA/siUAFunctions.c \
						SiPhoneUA/si_ua_redirect.c SiPhoneUA/si_ua_sessiontimer.c SiPhoneUA/si_ua_reliable.c \
						SiPhoneUA/si_ua_fax.c SiPhoneUA/si_ua_srtp_layer.c SiPhoneUA/si_ua_base64.c \
						CCFSM/ccfsm_init.c CCFSM/sc1445x_phone_api.c \
						ConsoleIFACE/si_cc_server_api.c \
						exosip2/src/eXosip.c \
						exosip2/src/eXconf.c exosip2/src/eXregister_api.c \
						exosip2/src/eXcall_api.c exosip2/src/eXmessage_api.c exosip2/src/eXtransport.c \
						exosip2/src/jrequest.c exosip2/src/jresponse.c exosip2/src/jcallback.c \
						exosip2/src/jdialog.c exosip2/src/udp.c exosip2/src/jcall.c exosip2/src/jreg.c \
						exosip2/src/eXutils.c exosip2/src/jevents.c exosip2/src/misc.c exosip2/src/jauth.c exosip2/src/eXtl.c \
						exosip2/src/eXtl_udp.c exosip2/src/eXtl_tcp.c exosip2/src/eXtl_dtls.c exosip2/src/eXtl_tls.c \
						exosip2/src/eXsubscription_api.c \
						exosip2/src/eXoptions_api.c exosip2/src/eXinsubscription_api.c exosip2/src/eXpublish_api.c \
						exosip2/src/jnotify.c exosip2/src/jsubscribe.c \
						exosip2/src/inet_ntop.c exosip2/src/milenage.c exosip2/src/rijndael.c \
						exosip2/src/jpipe.c exosip2/src/eXrefer_api.c exosip2/src/jpublish.c exosip2/src/sdp_offans.c \
						$(MCU_DIR)/sc1445x_amb_api.c $(MCU_DIR)/sc1445x_mcu_api.c $(MCU_DIR)/sc1445x_mcu_block.c $(MCU_DIR)/sc1445x_mcu_scheduler.c \
						$(MCU_DIR)/sc1445x_alsa.c $(MCU_DIR)/sc1445x_ortp.c $(MCU_DIR)/sc1445x_media.c $(MCU_DIR)/sc1445x_ringplay.c \
						$(MCU_DIR)/sc1445x_srtp.c \
 						$(COMMON_DIR)/si_html_conv.c $(COMMON_DIR)/si_phbook_api.c $(COMMON_DIR)/si_configfiles_api.c \
						$(COMMON_DIR)/si_call_log_api.c $(COMMON_DIR)/si_timers_api.c $(COMMON_DIR)/app_presence.c\
						$(PRINT_API_DIR)/si_print_api.c \
						$(SYSLOG_DIR)/syslog.c \
  						$(RSS_APPLICATION_DIR)/RSSFeed.c $(RSS_APPLICATION_DIR)/error.c $(RSS_APPLICATION_DIR)/RSSReader.c \
  						$(RSS_APPLICATION_DIR)/HTTPApi.c $(RSS_APPLICATION_DIR)/connectionHandler.c \
						$(RADIO_APP_DIR)/SimpleAudio.c $(RADIO_APP_DIR)/alsa.c $(RADIO_APP_DIR)/audio.c $(RADIO_APP_DIR)/audio_conversion.c \
						$(RADIO_APP_DIR)/common.c $(RADIO_APP_DIR)/decoder.c $(RADIO_APP_DIR)/fifo_buf.c $(RADIO_APP_DIR)/files.c \
						$(RADIO_APP_DIR)/io.c $(RADIO_APP_DIR)/io_curl.c $(RADIO_APP_DIR)/log.c $(RADIO_APP_DIR)/mp3.c \
						$(RADIO_APP_DIR)/out_buf.c $(RADIO_APP_DIR)/player.c  $(RADIO_APP_DIR)/playlist.c $(RADIO_APP_DIR)/playlist_file.c \
						$(RADIO_APP_DIR)/rbtree.c  $(RADIO_APP_DIR)/tags_cache.c  $(RADIO_APP_DIR)/vorbis.c $(RADIO_APP_DIR)/xing.c $(RADIO_APP_DIR)/flac.c 
					
NO_MMI_SOURCES =		SiPhoneUA/si_ua_instantmessage.c SiPhoneUA/si_user_agent.c SiPhoneUA/si_ua_server_api.c SiPhoneUA/si_ua_init.c \
						SiPhoneUA/si_ua_registrar.c SiPhoneUA/si_ua_callcontrol.c SiPhoneUA/si_ua_hold.c SiPhoneUA/si_ua_dtmf.c \
						SiPhoneUA/si_ua_notify.c SiPhoneUA/si_ua_events.c SiPhoneUA/si_ua_parser.c \
						SiPhoneUA/si_ua_transfer.c SiPhoneUA/si_sdp_negotiation.c \
  						SiPhoneUA/si_ua_api.c SiPhoneUA/si_ua_dns.c \
						SiPhoneUA/si_sc_manager.c SiPhoneUA/si_sc_tone.c  \
						SiPhoneUA/si_ua_stun.c SiPhoneUA/siUAFunctions.c  \
						SiPhoneUA/si_ua_redirect.c SiPhoneUA/si_ua_sessiontimer.c SiPhoneUA/si_ua_reliable.c  \
						SiPhoneUA/si_ua_fax.c SiPhoneUA/si_ua_srtp_layer.c SiPhoneUA/si_ua_base64.c \
						CCFSM/ccfsm_init.c CCFSM/sc1445x_phone_api.c \
 						exosip2/src/eXosip.c \
						exosip2/src/eXconf.c exosip2/src/eXregister_api.c \
						exosip2/src/eXcall_api.c exosip2/src/eXmessage_api.c exosip2/src/eXtransport.c \
						exosip2/src/jrequest.c exosip2/src/jresponse.c exosip2/src/jcallback.c \
						exosip2/src/jdialog.c exosip2/src/udp.c exosip2/src/jcall.c exosip2/src/jreg.c \
						exosip2/src/eXutils.c exosip2/src/jevents.c exosip2/src/misc.c exosip2/src/jauth.c exosip2/src/eXtl.c \
						exosip2/src/eXtl_udp.c exosip2/src/eXtl_tcp.c exosip2/src/eXtl_dtls.c exosip2/src/eXtl_tls.c \
						exosip2/src/eXsubscription_api.c \
						exosip2/src/eXoptions_api.c exosip2/src/eXinsubscription_api.c exosip2/src/eXpublish_api.c \
						exosip2/src/jnotify.c exosip2/src/jsubscribe.c \
						exosip2/src/inet_ntop.c exosip2/src/milenage.c exosip2/src/rijndael.c\
						exosip2/src/jpipe.c exosip2/src/eXrefer_api.c exosip2/src/jpublish.c exosip2/src/sdp_offans.c \
						$(MCU_DIR)/sc1445x_amb_api.c $(MCU_DIR)/sc1445x_mcu_api.c $(MCU_DIR)/sc1445x_mcu_block.c $(MCU_DIR)/sc1445x_mcu_scheduler.c \
						$(MCU_DIR)/sc1445x_alsa.c $(MCU_DIR)/sc1445x_ortp.c $(MCU_DIR)/sc1445x_media.c $(MCU_DIR)/sc1445x_ringplay.c \
						$(MCU_DIR)/sc1445x_srtp.c   \
 						$(COMMON_DIR)/si_configfiles_api.c $(COMMON_DIR)/si_call_log_api.c $(COMMON_DIR)/si_timers_api.c \
 						$(PRINT_API_DIR)/si_print_api.c \
						$(SYSLOG_DIR)/syslog.c \

SA_CVM_ADDINS_SOURCES =	$(DECT_DIR)/si_dect_api.c $(DECT_DIR)/si_dect_manager.c 

CVM_ONLY_SOURCES =	$(DECT_DIR)/si_dect_callcontrol.c \
					$(DECT_DIR)/cvmcon.c \
					$(DECT_DIR)/bus.c \
					$(DECT_DIR)/hdlc.c	
				
NATALIE_ONLY_SOURCES =	$(DECT_DIR)/cvmcon.c \
						$(DECT_DIR)/si_dect_callcontrol.c \
						$(DECT_DIR)/natalie/MailLog.c \
						$(DECT_DIR)/natalie/natalie_app.c				
					
ATA_OBJS = $(PWD)/leg_vp_dvr.o

all:
	@echo "Usage:"
	@echo "	'make sacvm' to make standalone exe."
	@echo "	'make sacvm.a' to make archive."

$(PWD)/leg_vp_dvr.o: $(PWD)/leg_vp_dvr.a
	cp $(PWD)/leg_vp_dvr.a $(PWD)/leg_vp_dvr.o 

T38_OBJS = 	$(MCU_DIR)/sc1445x_t38layer.o

TUI_PHONE_OBJECTS = $(PHONE_SOURCES:.c=.o) 
TUI_PHONE_OBJECTS += $(TUI_SOURCES:.c=.o) 

DOTMATRIX_PHONE_OBJECTS = $(PHONE_SOURCES:.c=.o)
DOTMATRIX_PHONE_OBJECTS += $(DOTMATRIX_SOURCES:.c=.o)

TFT_PHONE_OBJECTS = $(PHONE_SOURCES:.c=.o) 
TFT_PHONE_OBJECTS += $(TFT_SOURCES:.c=.o) 

ATA_OBJECTS = $(NO_MMI_SOURCES:.c=.o) 
ATA_OBJECTS += $(ATA_OBJS)
ATA_OBJECTS += $(T38_OBJS)

CVM_OBJECTS = $(NO_MMI_SOURCES:.c=.o)
CVM_OBJECTS += $(CVM_ONLY_SOURCES:.c=.o) 

SA_CVM_OBJECTS = $(CVM_ONLY_SOURCES:.c=.o)
SA_CVM_OBJECTS += $(SA_CVM_ADDINS_SOURCES:.c=.o)

SA_CVM_A_OBJECTS = $(SA_CVM_OBJECTS)

NATALIE_OBJECTS = $(NO_MMI_SOURCES:.c=.o)
NATALIE_OBJECTS += $(NATALIE_ONLY_SOURCES:.c=.o)

ATA_CVM_OBJECTS = $(CVM_OBJECTS)
ATA_CVM_OBJECTS += $(ATA_OBJS)
ATA_CVM_OBJECTS += $(T38_OBJS)

ATA_NATALIE_OBJECTS = $(NATALIE_OBJECTS)
ATA_NATALIE_OBJECTS += $(ATA_OBJS)
ATA_NATALIE_OBJECTS += $(T38_OBJS)

D_CVM_HEAD_OBJECTS = $(DOTMATRIX_PHONE_OBJECTS)
D_CVM_HEAD_OBJECTS += $(CVM_ONLY_SOURCES:.c=.o) 

G_CVM_HEAD_OBJECTS = $(TFT_PHONE_OBJECTS)
G_CVM_HEAD_OBJECTS += $(CVM_ONLY_SOURCES:.c=.o) 

T_CVM_HEAD_OBJECTS = $(TUI_PHONE_OBJECTS)
T_CVM_HEAD_OBJECTS += $(CVM_ONLY_SOURCES:.c=.o) 

D_NATALIE_HEAD_OBJECTS = $(DOTMATRIX_PHONE_OBJECTS)
D_NATALIE_HEAD_OBJECTS += $(NATALIE_ONLY_SOURCES:.c=.o)  

G_NATALIE_HEAD_OBJECTS = $(TFT_PHONE_OBJECTS)
G_NATALIE_HEAD_OBJECTS += $(NATALIE_ONLY_SOURCES:.c=.o)  

T_NATALIE_HEAD_OBJECTS = $(TUI_PHONE_OBJECTS)
T_NATALIE_HEAD_OBJECTS += $(NATALIE_ONLY_SOURCES:.c=.o)  

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $(CFLAGS_ATA) $(CFLAGS_DECT) $(CFLAGS_DECT_ONLY)$< > $@.$$$$; \
	sed 's,\($(*F)\)\.o[ :]*,\$*.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

objs = $(TUI_PHONE_OBJECTS) $(TFT_PHONE_OBJECTS) $(DOTMATRIX_PHONE_OBJECTS) $(ATA_OBJECTS) $(CVM_OBJECTS) $(NATALIE_OBJECTS) $(ATA_CVM_OBJECTS) $(ATA_NATALIE_OBJECTS) $(G_CVM_HEAD_OBJECTS) \
		$(T_CVM_HEAD_OBJECTS) $(D_CVM_HEAD_OBJECTS) $(G_NATALIE_HEAD_OBJECTS) $(D_NATALIE_HEAD_OBJECTS) $(T_NATALIE_HEAD_OBJECTS)
objs = $(SA_CVM_OBJECTS) $(SA_CVM_A_OBJECTS)
-include $(objs:.o=.d)

sidphone: version_build $(DOTMATRIX_PHONE_OBJECTS)
	$(CC) $(DOTMATRIX_PHONE_OBJECTS) $(LDFLAGS) -o $@

siphone: version_build $(TUI_PHONE_OBJECTS)
	$(CC) $(TUI_PHONE_OBJECTS) $(LDFLAGS) -o $@

sigphone: conf_check version_build ./common/si_linux_autoconf.h 
	$(MAKE) CFLAGS="$(CFLAGS) $(CFLAGS_TFT)" _sigphone

_sigphone: $(TFT_PHONE_OBJECTS)	
	$(CC) $(TFT_PHONE_OBJECTS) $(LDFLAGS_TFT) $(LDFLAGS) -o sigphone	

siata:	conf_check version_build ./common/si_linux_autoconf.h
	$(MAKE) CFLAGS="$(CFLAGS_ATA)" _siata

_siata: $(ATA_OBJECTS) 
	$(CC)  $(ATA_OBJECTS) $(LDFLAGS_NO_MMI) $(LDFLAGS_ATA) -o siata	
	$(AR) cru leg_vp_dvr.a $(ATA_OBJS)
	
sicvm: conf_check version_build ./common/si_linux_autoconf.h
	$(MAKE) CFLAGS="$(CFLAGS_DECT)" _sicvm

_sicvm: $(CVM_OBJECTS)
	$(CC) $(CVM_OBJECTS) $(LDFLAGS_NO_MMI) $(LDFLAGS_DECT) voip_manager.o -o sicvm

sacvm: version_build
	$(MAKE) CFLAGS="$(CFLAGS_SA_CVM)" _sacvm

_sacvm: $(SA_CVM_OBJECTS)
	$(CC) $(SA_CVM_OBJECTS) $(LDFLAGS_SACVM) ../../voip_manager/voip_manager.o -o sacvm

sacvm.a: version_build
	$(MAKE) CFLAGS="$(CFLAGS_SA_CVM_A)" _sacvm.a

_sacvm.a: $(SA_CVM_A_OBJECTS)
	$(AR) cru sacvm.a $(SA_CVM_A_OBJECTS)

si_ata_cvm: conf_check version_build ./common/si_linux_autoconf.h
	$(MAKE) CFLAGS="$(CFLAGS) $(CFLAGS_ATA) $(CFLAGS_DECT_ONLY)" _si_ata_cvm

_si_ata_cvm: $(ATA_CVM_OBJECTS)
	$(CC) $(ATA_CVM_OBJECTS) $(LDFLAGS_NO_MMI) $(LDFLAGS_ATA) $(LDFLAGS_DECT) -o si_ata_cvm
	$(AR) cru leg_vp_dvr.a $(ATA_OBJS)
	
si_ata_natalie: conf_check version_build ./common/si_linux_autoconf.h
	$(MAKE) CFLAGS="$(CFLAGS) $(CFLAGS_ATA) $(CFLAGS_DECT_ONLY)" _si_ata_natalie

_si_ata_natalie: $(ATA_NATALIE_OBJECTS)
	$(CC) $(ATA_NATALIE_OBJECTS) $(LDFLAGS_NO_MMI) $(LDFLAGS_ATA) $(LDFLAGS_DECT) -o si_ata_natalie
	$(AR) cru leg_vp_dvr.a $(ATA_OBJS)
	
si_d_cvm_head: conf_check version_build ./common/si_linux_autoconf.h
	$(MAKE) CFLAGS="$(CFLAGS) $(CFLAGS_DECT_ONLY)" _si_g_cvm_head

_si_d_cvm_head: $(D_CVM_HEAD_OBJECTS)
	$(CC) $(D_CVM_HEAD_OBJECTS) $(LDFLAGS_DECT) $(LDFLAGS) -o si_g_cvm_head	

si_g_cvm_head: conf_check version_build ./common/si_linux_autoconf.h
	$(MAKE) CFLAGS="$(CFLAGS) $(CFLAGS_TFT) $(CFLAGS_DECT_ONLY)" _si_g_cvm_head

_si_g_cvm_head: $(G_CVM_HEAD_OBJECTS)
	$(CC) $(G_CVM_HEAD_OBJECTS) $(LDFLAGS_DECT) $(LDFLAGS_TFT) $(LDFLAGS) -o si_g_cvm_head
	
si_t_cvm_head: conf_check version_build ./common/si_linux_autoconf.h
	$(MAKE) CFLAGS="$(CFLAGS) $(CFLAGS_DECT_ONLY)" _si_t_cvm_head

_si_t_cvm_head: $(T_CVM_HEAD_OBJECTS)
	$(CC) $(T_CVM_HEAD_OBJECTS) $(LDFLAGS_DECT) $(LDFLAGS) -o si_t_cvm_head
		
sinatalie: conf_check version_build ./common/si_linux_autoconf.h
	$(MAKE) CFLAGS="$(CFLAGS_DECT)" _sinatalie

_sinatalie: $(NATALIE_OBJECTS)
	$(CC) $(NATALIE_OBJECTS) $(LDFLAGS_NO_MMI) $(LDFLAGS_DECT) -o sinatalie

si_d_natalie_head: conf_check version_build ./common/si_linux_autoconf.h
	$(MAKE) CFLAGS="$(CFLAGS) $(CFLAGS_DECT_ONLY)" _si_g_natalie_head

_si_d_natalie_head: $(D_NATALIE_HEAD_OBJECTS)
	$(CC) $(D_NATALIE_HEAD_OBJECTS) $(LDFLAGS_DECT) $(LDFLAGS) -o si_g_natalie_head	
	
si_g_natalie_head: conf_check version_build ./common/si_linux_autoconf.h
	$(MAKE) CFLAGS="$(CFLAGS) $(CFLAGS_TFT) $(CFLAGS_DECT_ONLY)" _si_g_natalie_head

_si_g_natalie_head: $(G_NATALIE_HEAD_OBJECTS)
	$(CC) $(G_NATALIE_HEAD_OBJECTS) $(LDFLAGS_DECT) $(LDFLAGS_TFT) $(LDFLAGS) -o si_g_natalie_head
	
si_t_natalie_head: conf_check version_build ./common/si_linux_autoconf.h
	$(MAKE) CFLAGS="$(CFLAGS) $(CFLAGS_DECT_ONLY)" _si_t_natalie_head

_si_t_natalie_head: $(T_NATALIE_HEAD_OBJECTS)
	$(CC) $(T_NATALIE_HEAD_OBJECTS) $(LDFLAGS_DECT) $(LDFLAGS) -o si_t_natalie_head	
	
conf_check: 
	./ata_dect_config_check.sh

version_build: 
	./version_build.sh
	
#ATA OBJS GENERATION
$(ATA_DIR)/sys_service.o : $(LEG_CODE_DIR)/arch/cr16/sys_service.c
	$(CC) $(CFLAGS) -c $(LEG_CODE_DIR)/arch/cr16/sys_service.c -o $@ 
$(ATA_DIR)/vp_kernel.o : $(LEG_CODE_DIR)/api_lib/vp_kernel/vp_kernel.c 
	$(CC) $(CFLAGS) -c $(LEG_CODE_DIR)/api_lib/vp_kernel/vp_kernel.c -o $@
$(ATA_DIR)/vp_api_common.o : $(LEG_CODE_DIR)/api_lib/common/vp_api_common.c
	$(CC) $(CFLAGS) -c $(LEG_CODE_DIR)/api_lib/common/vp_api_common.c -o $@ 
$(ATA_DIR)/boot_common.o : $(LEG_CODE_DIR)/api_lib/common/boot_common.c
	$(CC) $(CFLAGS) -c $(LEG_CODE_DIR)/api_lib/common/boot_common.c -o $@ 
$(ATA_DIR)/vp_api.o : $(LEG_CODE_DIR)/api_lib/common/vp_api.c
	$(CC) $(CFLAGS) -c $(LEG_CODE_DIR)/api_lib/common/vp_api.c -o $@ 
$(ATA_DIR)/vp_debug.o : $(LEG_CODE_DIR)/api_lib/common/vp_debug.c
	$(CC) $(CFLAGS) -c $(LEG_CODE_DIR)/api_lib/common/vp_debug.c -o $@ 
$(ATA_DIR)/vp_pcm_compute.o : $(LEG_CODE_DIR)/api_lib/common/vp_pcm_compute.c
	$(CC) $(CFLAGS) -c $(LEG_CODE_DIR)/api_lib/common/vp_pcm_compute.c -o $@
$(ATA_DIR)/profiles8.o : $(LEG_CODE_DIR)/vpmod/profiles8.c
	$(CC) $(CFLAGS) -c $(LEG_CODE_DIR)/vpmod/profiles8.c -o $@	  
$(ATA_DIR)/si_fxs_util.o : $(ATA_DIR)/si_ata_util.c
	$(CC) $(CFLAGS) -c $(ATA_DIR)/si_fxs_ata.c -o $@
$(ATA_DIR)/si_fxs_hdlr.o : $(ATA_DIR)/si_ata_hdlr.c
	 $(CC) $(CFLAGS) -c $(ATA_DIR)/si_fxs_ata.c -o $@ 

.c.o:
	$(CC) $(CFLAGS) $< -o $@

# Removing the executable and the object files
clean:
	rm -rf $(TUI_PHONE_OBJECTS) $(TFT_PHONE_OBJECTS) $(DOTMATRIX_PHONE_OBJECTS) $(ATA_OBJECTS) $(CVM_OBJECTS) $(NATALIE_OBJECTS) $(ATA_CVM_OBJECTS) $(ATA_NATALIE_OBJECTS) $(G_CVM_HEAD_OBJECTS) $(T_CVM_HEAD_OBJECTS) $(G_NATALIE_HEAD_OBJECTS) $(T_NATALIE_HEAD_OBJECTS) *.gdb \
	siata siphone sigphone sidphone sicvm si_ata_cvm si_d_cvm_head si_t_cvm_head si_g_cvm_head sinatalie si_ata_natalie si_d_natalie_head si_t_natalie_head si_g_natalie_head  *.elf *.elf2flt \
	$(SA_CVM_A_OBJECTS) $(SA_CVM_OBJECTS) sacvm sacvm.a
	rm -rf $(objs:.o=.d*)
	#echo clean: make complete

nfs:
	cp sacvm /work/nfs/dect/ -f


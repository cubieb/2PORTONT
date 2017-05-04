
obj-y	:=	lt_api_vp886_909_all.o	\
			lt_api_vp886_cap.o	\
			lt_api_vp886_dc_feed_st.o	\
			lt_api_vp886_dc_voltage.o	\
			lt_api_vp886_line_v.o	\
			lt_api_vp886_loopback.o	\
			lt_api_vp886_msocket.o	\
			lt_api_vp886_on_off_hook_st.o	\
			lt_api_vp886_pre_line_v.o	\
			lt_api_vp886_rd_bat_cond.o	\
			lt_api_vp886_rd_loop_cond.o	\
			lt_api_vp886_res_flt.o	\
			lt_api_vp886_ringers.o	\
			lt_api_vp886_ringing_st.o	\
			lt_api_vp886_roh.o	\
			lt_api_vp886_xconnect.o	\
			lt_api_vp886.o


#EXTRA_CFLAGS += -Werror
EXTRA_CFLAGS += -DREALTEK_PATCH_FOR_MICROSEMI

#EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/
#EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/common/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/common/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp886_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/linetest/vp886_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/linetest/include/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/linetest/vp886_lt/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/$(ZARLINK_PROFILE)/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/arch/rtl89xxb/

EXTRA_CFLAGS += -I$(DIR_VOIP)/include/




include $(DIR_VOIP)/rules.mak

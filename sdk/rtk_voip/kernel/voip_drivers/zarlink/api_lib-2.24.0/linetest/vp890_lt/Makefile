
obj-y	:=	lt_api_vp890_all_gr_909.o	\
			lt_api_vp890_cap.o	\
			lt_api_vp890_cap_ringers.o	\
			lt_api_vp890_cross_connect.o	\
			lt_api_vp890_dc_feed.o	\
			lt_api_vp890_dc_voltage.o	\
			lt_api_vp890_flt_dscrm.o	\
			lt_api_vp890_line_v.o	\
			lt_api_vp890_loopback.o	\
			lt_api_vp890_master_socket.o	\
			lt_api_vp890_on_off_hook_st.o	\
			lt_api_vp890_pre_linev.o	\
			lt_api_vp890_rd_bat_cond.o	\
			lt_api_vp890_rd_loop_cond.o	\
			lt_api_vp890_res_flt.o	\
			lt_api_vp890_ringers.o	\
			lt_api_vp890_ringing_st.o	\
			lt_api_vp890_roh.o	\
			lt_api_vp890_vvp_calculate_results.o	\
			lt_api_vp890.o	\


#EXTRA_CFLAGS += -Werror
EXTRA_CFLAGS += -DREALTEK_PATCH_FOR_MICROSEMI

#EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/
#EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/common/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/common/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp890_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/linetest/vp890_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/linetest/include/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/linetest/vp890_lt/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/$(ZARLINK_PROFILE)/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/arch/rtl89xxb/

EXTRA_CFLAGS += -I$(DIR_VOIP)/include/




include $(DIR_VOIP)/rules.mak

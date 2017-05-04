
obj-y	:= 	lt_api_vp880.o					\
			lt_api_vp880_all_gr_909.o		\
			lt_api_vp880_cap.o				\
			lt_api_vp880_cap_ringers.o		\
			lt_api_vp880_cross_connect.o	\
			lt_api_vp880_dc_feed.o			\
			lt_api_vp880_dc_voltage.o		\
			lt_api_vp880_flt_dscrm.o		\
			lt_api_vp880_line_v.o			\
			lt_api_vp880_loopback.o			\
			lt_api_vp880_master_socket.o	\
			lt_api_vp880_on_off_hook_st.o	\
			lt_api_vp880_pre_linev.o		\
			lt_api_vp880_rd_bat_cond.o		\
			lt_api_vp880_rd_loop_cond.o		\
			lt_api_vp880_res_flt.o			\
			lt_api_vp880_ringers.o			\
			lt_api_vp880_ringing_st.o		\
			lt_api_vp880_roh.o				\
			lt_api_vp880_vvp_calculate_results.o

EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/microsemi/microsemi_verivoice_api/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/microsemi/microsemi_verivoice_api/vp880_lt/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/microsemi/microsemi_verivoice_api/vp886_lt/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/microsemi/microsemi_verivoice_api/vp890_lt/
EXTRA_CFLAGS += -I$(DIR_VOIP)/include/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/common/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/microsemi/microsemi_voicepath_api/arch/rtl89xxb/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/vp890_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/vp880_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/vp886_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/includes/linetest/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/vp886_api/linetest/

include $(DIR_VOIP)/rules.mak


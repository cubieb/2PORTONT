
obj-y	:= 	lt_api_vp886.o					\
			lt_api_vp886_909_all.o			\
			lt_api_vp886_cap.o				\
			lt_api_vp886_dc_feed_st.o		\
			lt_api_vp886_dc_voltage.o		\
			lt_api_vp886_line_v.o			\
			lt_api_vp886_loopback.o			\
			lt_api_vp886_msocket.o			\
			lt_api_vp886_on_off_hook_st.o	\
			lt_api_vp886_pre_line_v.o		\
			lt_api_vp886_rd_bat_cond.o 		\
			lt_api_vp886_rd_loop_cond.o		\
			lt_api_vp886_res_flt.o			\
			lt_api_vp886_ringers.o			\
			lt_api_vp886_ringing_st.o		\
			lt_api_vp886_roh.o				\
			lt_api_vp886_xconnect.o

EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/microsemi/microsemi_verivoice_api/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/microsemi/microsemi_verivoice_api/vp880_lt/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/microsemi/microsemi_verivoice_api/vp886_lt/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/microsemi/microsemi_verivoice_api/vp890_lt/
EXTRA_CFLAGS += -I$(DIR_VOIP)/include/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/common/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/microsemi/microsemi_voicepath_api/arch/rtl89xxb/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/vp886_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/includes/linetest/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/vp886_api/linetest/

include $(DIR_VOIP)/rules.mak


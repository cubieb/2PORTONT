
obj-y	:= 	vp890_apiCal.o  	\
			vp890_common.o  	\
			vp890_config.o  	\
			vp890_control.o  	\
			vp890_fxo_control.o	\
			vp890_fxs_control.o	\
			vp890_init.o	\
			vp890_lp_control.o	\
			vp890_query.o		\
			vp890_seq.o	

EXTRA_CFLAGS += -I$(DIR_VOIP)/include/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/vp890_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/vp886_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/common/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/microsemi/microsemi_voicepath_api/arch/rtl89xxb/

ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LINETEST)" "y"
obj-y	+=	linetest/vp890_3ele_cap.o \
			linetest/vp890_3ele_res_hg.o \
			linetest/vp890_3ele_res_lg.o \
			linetest/vp890_ac_rloop.o \
			linetest/vp890_calibrate.o \
			linetest/vp890_conclude.o \
			linetest/vp890_cross_connect.o \
			linetest/vp890_dc_rloop.o \
			linetest/vp890_flt_dscrm.o \
			linetest/vp890_hybrid_loss.o \
			linetest/vp890_loopback.o \
			linetest/vp890_loop_conditions.o \
			linetest/vp890_met_ramp.o \
			linetest/vp890_met_ramp_int.o \
			linetest/vp890_msocket.o \
			linetest/vp890_open_vxc.o \
			linetest/vp890_testline.o \
			linetest/vp890_test_timer.o 
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/includes/linetest/
endif

include $(DIR_VOIP)/rules.mak

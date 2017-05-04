
obj-y	:= 	vp880_abs_calibration.o 	\
			vp880_calibration_common.o 	\
			vp880_control_common.o 	\
			vp880_fxo_control.o	\
			vp880_fxs_control.o	\
			vp880_init.o	\
			vp880_linestate_control.o	\
			vp880_lp_control.o	\
			vp880_query.o	\
			vp880_seq.o	\
			vp880_tracker_calibration.o

ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LINETEST)" "y"
obj-y	+=	linetest/vp_3ele_cap.o \
			linetest/vp_3ele_res_hg.o \
			linetest/vp_3ele_res_lg.o \
			linetest/vp_ac_rloop.o \
			linetest/vp_api_testline.o \
			linetest/vp_calibrate.o \
			linetest/vp_conclude.o \
			linetest/vp_cross_connect.o \
			linetest/vp_dc_rloop.o \
			linetest/vp_flt_dscrm.o \
			linetest/vp_loopback.o \
			linetest/vp_loop_conditions.o \
			linetest/vp_met_ramp.o \
			linetest/vp_met_ramp_int.o \
			linetest/vp_msocket.o \
			linetest/vp_open_vxc.o \
			linetest/vp_test_timer.o
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/includes/linetest/
endif

EXTRA_CFLAGS += -I$(DIR_VOIP)/include/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/vp880_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/vp890_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/vp886_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/microsemi/microsemi_voicepath_api/arch/rtl89xxb/

include $(DIR_VOIP)/rules.mak


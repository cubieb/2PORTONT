
obj-y	:= 	vp886_adaptive_ringing.o 	\
			vp886_calibration_common.o 	\
			vp886_common.o 	\
			vp886_control.o	\
			vp886_events.o	\
			vp886_init.o	\
			vp886_query.o	\
			vp886_seq.o	\
			vp886_slac.o	\
			vp886_timers.o

EXTRA_CFLAGS += -I$(DIR_VOIP)/include/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/vp886_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/common/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/vp890_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/microsemi/microsemi_voicepath_api/arch/rtl89xxb/

ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LINETEST)" "y"
obj-y	+= 	linetest/vp886_3ele_cap.o \
			linetest/vp886_ac_rloop.o \
			linetest/vp886_conclude.o \
			linetest/vp886_dc_rloop.o \
			linetest/vp886_gain_toolbox.o \
			linetest/vp886_loopback.o \
			linetest/vp886_loop_conditions.o \
			linetest/vp886_met_ramp.o \
			linetest/vp886_met_ramp_init.o \
			linetest/vp886_mpicmd.o \
			linetest/vp886_msocket.o \
			linetest/vp886_open_vxc.o \
			linetest/vp886_prepare.o \
			linetest/vp886_res_flt.o \
			linetest/vp886_testline.o \
			linetest/vp886_timer.o \
			linetest/vp886_xconnect.o 
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/includes/linetest/
endif

include $(DIR_VOIP)/rules.mak


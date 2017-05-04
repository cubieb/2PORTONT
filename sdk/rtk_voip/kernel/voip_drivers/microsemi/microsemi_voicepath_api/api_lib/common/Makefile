
obj-y := vp_api_config.o \
	vp_api_common.o	\
	vp_api_control.o \
	vp_api_cslac_seq.o \
	vp_api_init.o \
	vp_api_query.o \
	vp_api_test.o \
	vp_pulse_decode.o \
	vp_timer_queue.o \
	vp_debug.o \
	vp_dtmf_detect.o

EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/microsemi/microsemi_voicepath_api/arch/rtl89xxb/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/vp890_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/vp880_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/vp886_api/

ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LINETEST)" "y"
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/includes/linetest
endif

include $(DIR_VOIP)/rules.mak

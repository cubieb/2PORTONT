
obj-y	:= 	vp_api_config.o		\
			vp_api_common.o \
			vp_api_control.o	\
			vp_api_cslac_seq.o	\
			vp_api_init.o	\
			vp_api_query.o	\
			vp_api_test.o	\
			vp_dtmf_detect.o	\
			vp_pulse_decode.o	\
			vp_timer_queue.o	\
			vp_debug.o 

EXTRA_CFLAGS += -DREALTEK_PATCH_FOR_MICROSEMI
EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/arch/rtl89xxb/

ifeq ($(CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES),y) 
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp890_api/
ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_LINETEST)" "y"
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/linetest/include/
endif
endif

ifeq ($(CONFIG_RTK_VOIP_SLIC_ZARLINK_880_SERIES),y) 
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp880_api/
endif

ifeq ($(CONFIG_RTK_VOIP_SLIC_ZARLINK_886_SERIES),y) 
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp886_api/
ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_LINETEST)" "y"
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/linetest/include/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/linetest/vp886_api/
endif
endif

include $(DIR_VOIP)/rules.mak

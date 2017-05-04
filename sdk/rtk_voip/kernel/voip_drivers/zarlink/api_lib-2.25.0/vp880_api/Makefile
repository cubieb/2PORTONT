
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

EXTRA_CFLAGS += -DREALTEK_PATCH_FOR_MICROSEMI
EXTRA_CFLAGS += -I$(DIR_VOIP)/include/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp880_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/arch/rtl89xxb/

ifeq "$(CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES)" "y"
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp890_api/
endif

include $(DIR_VOIP)/rules.mak



obj-y	:= 	vp886_3ele_cap.o  	\
			vp886_ac_rloop.o  	\
			vp886_conclude.o  	\
			vp886_dc_rloop.o  	\
			vp886_gain_toolbox.o	\
			vp886_loop_conditions.o	\
			vp886_loopback.o	\
			vp886_met_ramp.o	\
			vp886_met_ramp_init.o  		\
			vp886_mpicmd.o	\
			vp886_msocket.o	\
			vp886_open_vxc.o	\
			vp886_prepare.o	\
			vp886_res_flt.o	\
			vp886_testline.o	\
			vp886_timer.o	\
			vp886_xconnect.o
			

EXTRA_CFLAGS += -DREALTEK_PATCH_FOR_MICROSEMI
			
EXTRA_CFLAGS += -I$(DIR_VOIP)/include/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/linetest/vp886_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/linetest/include/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp886_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/arch/rtl89xxb/

include $(DIR_VOIP)/rules.mak

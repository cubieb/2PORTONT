
obj-y	:= 	vp890_3ele_cap.o  	\
			vp890_3ele_res_hg.o  	\
			vp890_3ele_res_lg.o  	\
			vp890_ac_rloop.o  	\
			vp890_calibrate.o	\
			vp890_conclude.o	\
			vp890_cross_connect.o	\
			vp890_dc_rloop.o	\
			vp890_flt_dscrm.o  		\
			vp890_hybrid_loss.o	\
			vp890_loop_conditions.o	\
			vp890_loopback.o	\
			vp890_met_ramp.o	\
			vp890_met_ramp_int.o	\
			vp890_msocket.o	\
			vp890_open_vxc.o	\
			vp890_test_timer.o	\
			vp890_testline.o	\
			

EXTRA_CFLAGS += -DREALTEK_PATCH_FOR_MICROSEMI

EXTRA_CFLAGS += -I$(DIR_VOIP)/include/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/linetest/vp890_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/linetest/include/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp890_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/arch/rtl89xxb/

include $(DIR_VOIP)/rules.mak

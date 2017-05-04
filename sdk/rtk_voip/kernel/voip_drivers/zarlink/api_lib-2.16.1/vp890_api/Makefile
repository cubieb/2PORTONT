
obj-y	:= 	vp890_apiCal.o  	\
			vp890_common.o  	\
			vp890_config.o  	\
			vp890_control.o  	\
			vp890_init.o  		\
			vp890_query.o 

# vp890_seq.c requires Zarlink NDA
ifeq ($(CONFIG_VOIP_SDK),y)
obj-$(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD) += vp890_seq_rtl89xxD.obj
obj-$(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) += vp890_seq.obj
obj-$(CONFIG_RTK_VOIP_DRIVERS_PCM8676) += vp890_seq_rtl8676.obj
obj-$(CONFIG_RTK_VOIP_SOC_8686_CPU0) += vp890_seq_rtl8686_cpu0.obj
obj-$(CONFIG_RTK_VOIP_SOC_8686_CPU1) += vp890_seq_rtl8686_cpu1.obj
else
obj-y += vp890_seq.o
endif

EXTRA_CFLAGS += -I$(DIR_VOIP)/include/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp890_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/arch/rtl89xxb/

include $(DIR_VOIP)/rules.mak

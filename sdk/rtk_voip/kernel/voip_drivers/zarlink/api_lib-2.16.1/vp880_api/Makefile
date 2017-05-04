
obj-y	:= 	apiCal.o 	\
			apicnt.o 	\
			apiInit.o 	\
			apiquery.o 	

# apiseq.c requires Zarlink NDA
ifeq ($(CONFIG_VOIP_SDK),y)
obj-$(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD) += apiseq_rtl89xxD.obj
obj-$(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) += apiseq.obj
obj-$(CONFIG_RTK_VOIP_DRIVERS_PCM8676) += apiseq_rtl8676.obj
obj-$(CONFIG_RTK_VOIP_SOC_8686_CPU0) += apiseq_rtl8686_cpu0.obj
obj-$(CONFIG_RTK_VOIP_SOC_8686_CPU1) += apiseq_rtl8686_cpu1.obj
else
obj-y += apiseq.o
endif

EXTRA_CFLAGS += -I$(DIR_VOIP)/include/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp880_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/arch/rtl89xxb/

ifeq "$(CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES)" "y"
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp890_api/
endif

include $(DIR_VOIP)/rules.mak


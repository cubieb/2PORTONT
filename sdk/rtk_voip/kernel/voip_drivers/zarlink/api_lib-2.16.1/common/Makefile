
obj-y	:= 	vp_api.o		\
			vp_api_common.o \
			vp_debug.o 

# vp_api_cslac_seq.c requires Zarlink NDA
ifeq ($(CONFIG_VOIP_SDK),y)
ifeq ($(CONFIG_RTK_VOIP_SLIC_ZARLINK_880_SERIES)$(CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES),yy)
	obj-$(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) += vp_api_cslac_seq_rtl89xxC.obj
	obj-$(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD) += vp_api_cslac_seq_rtl89xxD.obj
	obj-$(CONFIG_RTK_VOIP_DRIVERS_PCM8676) += vp_api_cslac_seq_rtl8676.obj
	obj-$(CONFIG_RTK_VOIP_DRIVERS_PCM8672) += vp_api_cslac_seq_rtl867x.obj
	obj-$(CONFIG_RTK_VOIP_SOC_8686_CPU0)   += vp_api_cslac_seq_rtl8686_cpu0.obj
	obj-$(CONFIG_RTK_VOIP_SOC_8686_CPU1)   += vp_api_cslac_seq_rtl8686_cpu1.obj
else
ifeq ($(CONFIG_RTK_VOIP_SLIC_ZARLINK_880_SERIES),y)
	obj-$(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) += vp_api_cslac_seq_vp880_rtl89xxC.obj
	obj-$(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD) += vp_api_cslac_seq_vp880_rtl89xxD.obj
	obj-$(CONFIG_RTK_VOIP_DRIVERS_PCM8676) += vp_api_cslac_seq_vp880_rtl8676.obj
	obj-$(CONFIG_RTK_VOIP_DRIVERS_PCM8672) += vp_api_cslac_seq_vp880_rtl867x.obj
	obj-$(CONFIG_RTK_VOIP_SOC_8686_CPU0)   += vp_api_cslac_seq_vp880_rtl8686_cpu0.obj
	obj-$(CONFIG_RTK_VOIP_SOC_8686_CPU1)   += vp_api_cslac_seq_vp880_rtl8686_cpu1.obj
endif
ifeq ($(CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES),y)
	obj-$(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) += vp_api_cslac_seq_vp890_rtl89xxC.obj
	obj-$(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD) += vp_api_cslac_seq_vp890_rtl89xxD.obj
	obj-$(CONFIG_RTK_VOIP_DRIVERS_PCM8676) += vp_api_cslac_seq_vp890_rtl8676.obj
	obj-$(CONFIG_RTK_VOIP_DRIVERS_PCM8672) += vp_api_cslac_seq_vp890_rtl867x.obj
	obj-$(CONFIG_RTK_VOIP_SOC_8686_CPU0)   += vp_api_cslac_seq_vp890_rtl8686_cpu0.obj
	obj-$(CONFIG_RTK_VOIP_SOC_8686_CPU1)   += vp_api_cslac_seq_vp890_rtl8686_cpu1.obj
endif
endif
else
	obj-y += vp_api_cslac_seq.o
endif

EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/arch/rtl89xxb/

ifeq ($(CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES),y) 
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp890_api/
endif

ifeq ($(CONFIG_RTK_VOIP_SLIC_ZARLINK_880_SERIES),y) 
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp880_api/
endif

include $(DIR_VOIP)/rules.mak

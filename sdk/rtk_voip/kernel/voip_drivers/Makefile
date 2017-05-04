# pcm-mod-objs-yy will not be built 

pcm-mod-objs-y    :=

obj-$(CONFIG_RTK_VOIP_DRIVERS)  += voip_drivers.o

#CONFIG_VOIP_SDK=y

# con
pcm-mod-objs-y += con_main.o con_feature.o 
pcm-mod-objs-y += con_register.o con_bind.o
pcm-mod-objs-y += con_mux.o 
pcm-mod-objs-y += con_event.o 
pcm-mod-objs-y += con_pluse_dial.o 
pcm-mod-objs-y += con_ring.o 
pcm-mod-objs-$(CONFIG_RTK_VOIP_QOS) += con_sup_qos.o 
pcm-mod-objs-$(CONFIG_RTK_VOIP_CON_BUSFIFO_LECBUF) += con_bus_fifo.o con_bus_handler.o con_lec_buf.o 

pcm-mod-objs-y += con_polling.o  
pcm-mod-objs-y$(CONFIG_RTK_VOIP_CON_POLLING_PROCESS) += con_polling_snd.o
pcm-mod-objs-$(CONFIG_RTK_VOIP_CON_POLLING_PROCESS) += con_polling_process.o

pcm-mod-objs-y += con_entry.o

pcm-mod-objs-$(CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP)  += con_ipc_mirror_dsp.o  con_ipc_rpc_dsp.o
pcm-mod-objs-$(CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST) += con_ipc_mirror_host.o con_ipc_rpc_host.o con_ipc_help_host.o 

pcm-mod-objs-$(CONFIG_RTK_VOIP_DEFER_SNDDEV_INIT) += con_defer_init.o

#pcm-mod-objs-y += con_16k_sample.o

# bus
pcm-mod-objs-$(CONFIG_RTK_VOIP_DRIVERS_IIS) += bus_iis_interface.o
pcm-mod-objs-$(CONFIG_RTK_VOIP_DRIVERS_PCM) += bus_pcm_interface.o bus_pcm_law.o
pcm-mod-objs-$(CONFIG_RTK_VOIP_DRIVERS_PCM_LAW) += bus_pcm_law.o

# snd
#pcm-mod-objs-$(CONFIG_RTK_VOIP_SLIC_RELAY) += snd_slic_relay.o 
pcm-mod-objs-$(CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA)   += snd_virtual_daa.o

pcm-mod-objs-$(CONFIG_RTK_VOIP_DRIVERS_ATA_SLIC)    += snd_mux_slic.o snd_help.o 
#pcm-mod-objs-$(CONFIG_RTK_VOIP_DRIVERS_FXO)   += snd_mux_daa.o 
pcm-mod-objs-y   += snd_mux_daa.o 

pcm-mod-objs-$(CONFIG_RTK_VOIP_DRIVERS_SI3050)   += snd_proslic_daa.o snd_si3050.o 

obj-$(CONFIG_RTK_VOIP_DRIVERS_SILAB_SLIC_LINETEST) += proslic_mlt_api-v2.4.3/ 
obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226x)	+= proslic_api/ snd_pin_reset.o snd_pin_cs.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226x)	+= snd_si3226x.o snd_proslic_base.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226)	+= proslic_api/ snd_pin_reset.o snd_pin_cs.o 
obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226)	+= snd_si3226.o snd_proslic_base.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x)	+= proslic_api/ snd_pin_reset.o snd_pin_cs.o 
obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x)	+= snd_si3217x.o snd_proslic_base.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK) += zarlink/ snd_pin_reset.o snd_pin_cs.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221) += snd_le88221.o snd_zarlink_fxs.o snd_zarlink_common.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88111) += snd_le88111.o snd_zarlink_fxs.o snd_zarlink_common.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116) += snd_le89116.o snd_zarlink_fxs.o snd_zarlink_common.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316) += snd_le89316.o snd_zarlink_fxs.o snd_zarlink_daa.o snd_zarlink_common.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9662) += snd_le9662.o snd_zarlink_fxs.o snd_zarlink_common.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661) += snd_le9661.o snd_zarlink_fxs.o snd_zarlink_common.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9641) += snd_le9641.o snd_zarlink_fxs.o snd_zarlink_common.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601) += snd_le88601.o snd_zarlink_fxs.o snd_zarlink_common.o

obj-$(CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621) += snd_alc5621.o 

obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI) += microsemi/  snd_pin_reset.o snd_pin_cs.o snd_microsemi.o 
obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI) +=  snd_microsemi.o

obj-$(CONFIG_RTK_VOIP_DRIVERS_MIRROR_SLIC) += snd_mirror_slic.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_MIRROR_DAA) += snd_mirror_daa.o

obj-$(CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD) += snd_shadow_slic.o snd_shadow_daa.o

obj-y += snd_interface.o

# dsp
ifneq "$(CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST)" "y"
pcm-mod-objs-$(CONFIG_REALTEK_VOIP) += dsp_rtk_main.o dsp_rtk_handler.o dsp_rtk_isr.o dsp_rtk_mux.o 
pcm-mod-objs-$(CONFIG_REALTEK_VOIP) += dsp_rtk_caller.o
pcm-mod-objs-$(CONFIG_AUDIOCODES_VOIP)  += dsp_ac_main.o  dsp_ac_handler.o  dsp_ac_isr.o  dsp_ac_mux.o 
pcm-mod-objs-$(CONFIG_AUDIOCODES_VOIP)  += radiax_save.o
pcm-mod-objs-y += dsp_define.o
pcm-mod-objs-$(CONFIG_RTK_VOIP_T38) += t38_handler.o
endif
pcm-mod-objs-$(CONFIG_REALTEK_VOIP) += dsp_rtk_global.o
pcm-mod-objs-$(CONFIG_AUDIOCODES_VOIP) += dsp_ac_global.o

pcm-mod-objs-$(CONFIG_RTK_VOIP_T38) += t38_handler_data.o

# ioc 
obj-y += ioc_core.o
obj-$(CONFIG_RTK_VOIP_IOC_LED) += ioc_led.o 
obj-$(CONFIG_RTK_VOIP_IOC_SLIC_RELAY) += ioc_slic_relay.o 
obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116_IO) += snd_zarlink_ioc_op.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221_IO) += snd_zarlink_ioc_op.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9662_IO) += snd_zarlink_ioc_op.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_IO) += snd_zarlink_ioc_op.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9641_IO) += snd_zarlink_ioc_op.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_IO) += snd_zarlink_ioc_op.o

# common driver 
pcm-mod-objs-$(CONFIG_RTK_VOIP_LED)	+= led.o
obj-$(CONFIG_SERIAL_SC16IS7X0)               += i2c.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_CODEC_WM8510)  += i2c.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621) += i2c.o
obj-$(CONFIG_RTK_VOIP_DRIVERS)  += gpio/
obj-$(CONFIG_RTK_VOIP_DRIVERS_ATA_SLIC) += spi/
obj-$(CONFIG_RTK_VOIP_DECT_SPI_SUPPORT) += spi/
obj-$(CONFIG_RTK_VOIP_DRIVERS_IP_PHONE)	+= iphone/

# timer 
obj-y += os_timer_core.o os_timer_linux.o 

#g168 debut test input
#pcm-mod-objs-y   += ec128_testdatas.o

EXTRA_CFLAGS += -DREALTEK_PATCH_FOR_MICROSEMI
EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -Iarch/rlx/bsp
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/common/basic_op
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/common/util
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/cp3
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/v152
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/spi

ifeq "$(CONFIG_AUDIOCODES_VOIP)" "y"
EXTRA_CFLAGS += -Iacmw_lx/ac_drv
EXTRA_CFLAGS += -DAUDIOCODES_VOIP
EXTRA_CFLAGS += -DACMW_LX
EXTRA_CFLAGS += -DACL_BYPASS
EXTRA_CFLAGS += -DACL_RELEASE
EXTRA_CFLAGS += -DAUDIOCODES_VOIP_SIP
endif

ifeq ($(CONFIG_RTK_VOIP_T38),y)
ifeq ($(CONFIG_VOIP_SDK),y)
EXTRA_CFLAGS +=  -I$(DIR_VOIP)/voip_dsp/dsp_r1/include
else
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/t38/rly_src
endif
endif

ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI)" "y"
MICROSEMI_LIB = microsemi/microsemi_voicepath_api/api_lib
export MICROSEMI_LIB
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/common/
#EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/vp580_api/
#EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/vp790_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/vp880_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/vp886_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/vp890_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/microsemi/microsemi_voicepath_api/arch/rtl89xxb/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/microsemi/microsemi_voicepath_api/profiles/
endif

ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LINETEST)" "y"
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/microsemi/microsemi_verivoice_api/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/microsemi/microsemi_verivoice_api/vp880_lt/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/microsemi/microsemi_verivoice_api/vp886_lt/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/microsemi/microsemi_verivoice_api/vp890_lt/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/includes/linetest/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/$(MICROSEMI_LIB)/vp886_api/linetest/
endif


ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK)" "y"
#ZARLINK_LIB   	 = api_lib-2.13.1
#ZARLINK_LIB   	 = api_lib-2.16.1
#ZARLINK_LIB   	 = api_lib-2.22.0
#ZARLINK_LIB   	 = api_lib-2.24.0
ZARLINK_LIB   	 = api_lib-2.25.0
#ZARLINK_PROFILE   = profiles_HG250
ZARLINK_PROFILE  = profiles
export ZARLINK_LIB
export ZARLINK_PROFILE
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/includes/
ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_LINETEST)" "y"
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/linetest/include/
ifeq "$(CONFIG_RTK_VOIP_SLIC_ZARLINK_886_SERIES)" "y"
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/linetest/vp886_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/linetest/vp886_lt/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp886_api/
endif
endif
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/$(ZARLINK_PROFILE)/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/arch/rtl89xxb/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/common/
ifeq "$(CONFIG_RTK_VOIP_SLIC_ZARLINK_880_SERIES)" "y"
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/ve880/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp880_api/
endif
ifeq "$(CONFIG_RTK_VOIP_SLIC_ZARLINK_886_SERIES)" "y"
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/ve886/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp886_api/
endif
ifeq "$(CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES)" "y"
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/ve890/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp890_api/
endif
endif

ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_SILAB)" "y"
EXTRA_CFLAGS += -DREALTEK_PATCH_FOR_SILAB
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/proslic_api/config_inc/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/proslic_api/inc/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/proslic_api/custom/
ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SILAB_SLIC_LINETEST)" "y"
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/proslic_mlt_api-v2.4.3/inc
endif
endif

ifeq "$(CONFIG_VOIP_COP3_PROFILE)" "y"
EXTRA_CFLAGS += -DFEATURE_COP3_PROFILE
EXTRA_CFLAGS += -DFEATURE_COP3_PCMISR
EXTRA_CFLAGS += -DFEATURE_COP3_PCM_RX
EXTRA_CFLAGS += -DFEATURE_COP3_LEC
#EXTRA_CFLAGS += -DFEATURE_COP3_PCM_HANDLER
#EXTRA_CFLAGS += -DFEATURE_COP3_DTMFDEC
endif

ifeq ($(CONFIG_RTK_VOIP_PACKAGE_865X),y)
ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8651)$(CONFIG_RTK_VOIP_DRIVERS_PCM865xC),y)
EXTRA_CFLAGS += -I../../drivers/net/re865x/rtl865x
endif
endif


ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

# rock: add module name for PRINT
EXTRA_CFLAGS += -DMODULE_NAME=\"DRV\" 

voip_drivers-objs :=  $(pcm-mod-objs-y)

include $(DIR_VOIP)/rules.mak



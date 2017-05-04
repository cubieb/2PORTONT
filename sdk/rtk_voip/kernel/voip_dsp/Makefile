#
# Makefile for the DSP and PCM driver
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...



ifeq ($(CONFIG_RTK_VOIP_DSP), y)
obj-y := v152/ 
export-objs =
dsp-mod-objs :=
#CONFIG_VOIP_SDK=y

ifeq ($(CONFIG_VOIP_SDK),y)
#obj-y += dsp_main.obj
#obj-y += pcm_handler.obj
#obj-y += lec_g168.obj
#obj-y += dtmf.obj
#obj-y += fax.obj
#obj-y += ced_det.obj
#obj-y += fsk_gen.obj
#obj-y += voice_gain.obj
#obj-y += codec_descriptor.obj
obj-y += codec_descriptor.o
obj-y += dsp_init_var.o
#obj-y += cpu_setup.obj
#obj-y += energy.obj 

#ifeq ($(CONFIG_RTK_VOIP_DRIVERS_IP_PHONE),y)
#obj-$(CONFIG_RTK_VOIP_DRIVERS_IIS) += resampler.obj
#obj-y += aec_g168.obj
#obj-y += nr.obj
#else
#obj-y += aec_g168.obj
#endif

#ifeq ($(CONFIG_RTK_VOIP_DRIVERS_DAA_SUPPORT),y)
#obj-y += tone_detection.obj
#obj-y += fsk_det.obj
#endif

#obj-$(CONFIG_RTK_VOIP_WIDEBAND_SUPPORT) += resampler.obj

#obj-y += dtmf_cid.obj 
#obj-y += silence_det.obj 

else #CONFIG_VOIP_SDK
obj-y += dsp_main.o
obj-y += pcm_handler.o
obj-y += codec_descriptor.o
obj-y += dsp_init_var.o
obj-y += cpu_setup.o 
#obj-y += agc.o
obj-y += dtmf_dec/
obj-y += fax_v21/
obj-y += ced/
obj-y += fsk_gen/
obj-y += lec/
obj-y += aec/
obj-y += energy/

ifeq ("y",$(subst "yy","y","$(SUPPORT_RESAMPLER_G722)$(CONFIG_RTK_VOIP_DRIVERS_IIS)$(CONFIG_RTK_VOIP_WIDEBAND_SUPPORT)"))
obj-y += resampler/
else
subdir-y += resampler/
endif

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_IP_PHONE),y)
obj-y += nr/
else
subdir-y += nr/
endif

obj-y += voice_gain/

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_DAA_SUPPORT),y)
obj-y += tone_det/
obj-y += fsk_det/
ifeq ($(CONFIG_RTK_VOIP_DRIVERS_SI3050),y)
obj-y += fxo_tune/
endif
else
subdir-y += tone_det/
subdir-y += fsk_det/
ifeq ($(CONFIG_RTK_VOIP_DRIVERS_SI3050),y)
subdir-y += fxo_tune/
endif
endif

obj-y += dtmf_cid/
obj-y += silence_det.o

endif #CONFIG_VOIP_SDK

obj-y += ivr/
obj-y += rtp/
obj-y += dsp_r1/
obj-y += dsp_r0/
obj-y += cp3/
ifeq ($(CONFIG_RTK_VOIP_SOC_8686_CPU0),y)
ifeq ($(CONFIG_RTL9602C_SERIES), y)
obj-$(CONFIG_VOIP_SDK) += rtk_modules_5281_9602C.obj
else
obj-$(CONFIG_VOIP_SDK) += rtk_modules_5281.obj
endif
else
ifeq ($(CONFIG_RTK_VOIP_SOC_8686_CPU1),y)
obj-$(CONFIG_VOIP_SDK) += rtk_modules_5181.obj
else
ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8676),y)
obj-$(CONFIG_VOIP_SDK) += rtk_modules_5281.obj
else
$(error check config)
endif
endif
endif
#obj-$(CONFIG_VOIP_SDK) += rtk_modules.obj

obj-y$(CONFIG_USE_ROMCODE) += dsplib.obj # lib have to be the last 

EXTRA_CFLAGS += -DGNU
EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/cp3
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/energy/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/common/basic_op/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/common/util

ifeq ($(CONFIG_RTK_VOIP_PACKAGE_865X),y)
ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8651)$(CONFIG_RTK_VOIP_DRIVERS_PCM865xC),y)
EXTRA_CFLAGS += -Idrivers/net/re865x/rtl865x
endif
endif

ifeq "$(CONFIG_VOIP_COP3_PROFILE)" "y"
EXTRA_CFLAGS += -DFEATURE_COP3_PROFILE
#EXTRA_CFLAGS += -DFEATURE_COP3_DTMFDEC
#EXTRA_CFLAGS += -DFEATURE_COP3_LEC
#EXTRA_CFLAGS += -DFEATURE_COP3_DSPPROCESS
endif

#obj-m   := $(O_TARGET)

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

# rock: add module name for PRINT
EXTRA_CFLAGS += -DMODULE_NAME=\"DSP\"

include $(DIR_VOIP)/rules.mak
endif # CONFIG_RTK_VOIP_DSP

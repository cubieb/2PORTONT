# Makefile for the acmw_lx
#
#
#DIR_ACMW = $(shell /bin/pwd)
DIR_ACMW = $(src)
export DIR_ACMW

ifeq "$(CONFIG_AUDIOCODES_VOIP)" "y"
ifeq "$(CONFIG_DEFAULTS_KERNEL_2_6)" "y"

ifeq ($(CONFIG_VOIP_SDK_XXX),y)
#obj-y += pcm865x.obj
else
#obj-y += pcm865x.o
endif

obj-y += ac_drv/
obj-y += modules/

ifeq ($(CONFIG_RTK_VOIP_T38),y)
ifeq ($(CONFIG_VOIP_SDK),y)
EXTRA_CFLAGS +=  -I$(DIR_ACMW)/../rtk_voip/voip_dsp/dsp_r1/include
else
EXTRA_CFLAGS += -I$(DIR_ACMW)/../rtk_voip/voip_dsp/dsp_r1/t38/rly_src
endif
endif

EXTRA_CFLAGS += -I
EXTRA_CFLAGS += -I$(DIR_ACMW)/../rtk_voip/include
EXTRA_CFLAGS += -I$(DIR_ACMW)/include/
EXTRA_CFLAGS += -I$(DIR_ACMW)/ac_drv
EXTRA_CFLAGS += -DGNU
EXTRA_CFLAGS += -DAUDIOCODES_VOIP
EXTRA_CFLAGS += -DACMW_LX
EXTRA_CFLAGS += -DACL_BYPASS
EXTRA_CFLAGS += -DACL_RELEASE
EXTRA_CFLAGS += -DAUDIOCODES_VOIP_SIP
#EXTRA_CFLAGS += $(ACLFLAGS)

else

ifeq ($(CONFIG_AUDIOCODES_VOIP_RELEASE),y)
INCLUDEFLAGS= -I. -I$(ACLPRJDIR)/include \
				-I$(ACLPRJDIR)/ac_drv


else
INCLUDEFLAGS= -I. -I$(ACLPRJDIR)/include \
				-I$(ACLPRJDIR)/modules/cp3 \
				-I$(ACLPRJDIR)/modules/g711 \
				-I$(ACLPRJDIR)/modules/g729ab \
				-I$(ACLPRJDIR)/modules/include \
				-I$(ACLPRJDIR)/ac_drv \
				-I$(ACLPRJDIR)/modules/jb \
				-I$(ACLPRJDIR)/modules/vp/common \
				-I$(ACLPRJDIR)/modules/vp/include \
				-I$(ACLPRJDIR)/modules/ibsdet \
				-I$(ACLPRJDIR)/modules/ibsgen \
				-I$(ACLPRJDIR)/modules/lec3 \
				-I$(ACLPRJDIR)/modules/commonutils \
				-I$(ACLPRJDIR)/modules/ramlog \
				-I$(ACLPRJDIR)/modules/auxprob \
				-I$(ACLPRJDIR)/modules/shell \
				-I$(ACLPRJDIR)/modules/digitalgain \
				-I$(ACLPRJDIR)/modules/relay \
				-I$(ACLPRJDIR)/modules/ilbc \
				-I$(ACLPRJDIR)/modules/gsmfr
endif

EXTRA_CFLAGS += $(ACLFLAGS)

O_TARGET := aclxx.o
subdir-y :=
obj-y :=
export-objs =

ifeq ($(CONFIG_RTK_VOIP_T38),y)
ifeq ($(CONFIG_VOIP_SDK),y)
EXTRA_CFLAGS +=  -I../rtk_voip/voip_dsp/dsp_r1/include
else
EXTRA_CFLAGS += -I../rtk_voip/voip_dsp/dsp_r1/t38/rly_src
endif
endif

ifeq ($(CONFIG_VOIP_SDK),y)
obj-y += pcm865x.obj
else
obj-y += pcm865x.o
endif

obj-y += ac_drv/ac_drv.o
subdir-y += ac_drv

#obj-y += tst_vec/tst_vec_obj.o
#subdir-y += tst_vec

ifeq ($(CONFIG_AUDIOCODES_VOIP_RELEASE),y)

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8186VA_),y)
obj-y += modules/ac_modules.rtl8186va.obj
endif

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8671VA_),y)
ifeq ($(CONFIG_RTK_VOIP_DRIVERS_IP_PHONE),y)
obj-y += modules/ac_modules.rtl8982.aec.obj
else
obj-y += modules/ac_modules.rtl8671va.obj
endif
endif

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8651CVA_),y)
obj-y += modules/ac_modules.rtl8962.obj
endif

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8651BV_),y)
obj-y += modules/ac_modules.rtl8651bv.obj
endif

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8651CVA_),y)
obj-y += modules/ac_modules.rtl8962.obj
endif

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8981B_),y)
ifeq ($(CONFIG_RTK_VOIP_DRIVERS_IP_PHONE),y)
obj-y += modules/ac_modules.rtl8981B.aec.obj
else
obj-y += modules/ac_modules.rtl8981B.obj
endif
endif

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8982B_),y)
obj-y += modules/ac_modules.rtl8982B.obj
endif

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8984B_),y)
obj-y += modules/ac_modules.rtl8984B.obj
endif

else
obj-y += modules/ac_modules.o
subdir-y += modules
endif


EXTRA_CFLAGS += -DGNU
EXTRA_CFLAGS += -DAUDIOCODES_VOIP
EXTRA_CFLAGS += -DACMW_LX
EXTRA_CFLAGS += -DACL_BYPASS
EXTRA_CFLAGS += -DACL_RELEASE
EXTRA_CFLAGS += -DAUDIOCODES_VOIP_SIP
EXTRA_CFLAGS += $(INCLUDEFLAGS)


obj-m   := $(O_TARGET)
ifneq ($(CONFIG_RTL8672),y)
include $(TOPDIR)/Rules.make
endif
endif


else
obj-y := dummy.o 
endif


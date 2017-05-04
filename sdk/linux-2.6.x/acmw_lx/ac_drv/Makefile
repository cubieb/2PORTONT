ifeq "$(CONFIG_DEFAULTS_KERNEL_2_6)" "y"

ifeq ($(CONFIG_VOIP_SDK_XXX),y)
obj-y := Ntt_sRing_det.obj AC49xDrv_Api.obj AC49xDrv_UserDefinedFunctions.obj RTK_AC49xApi_Interface.obj 
else
obj-y := Ntt_sRing_det.o AC49xDrv_Api.o AC49xDrv_UserDefinedFunctions.o RTK_AC49xApi_Interface.o 
endif

ifeq ($(CONFIG_AUDIOCODES_VOIP_RELEASE),y)
EXTRA_CFLAGS += -I$(DIR_ACMW)/include
EXTRA_CFLAGS += -I$(DIR_ACMW)/ac_drv
EXTRA_CFLAGS += -I$(DIR_ACMW)/../rtk_voip/voip_drivers
EXTRA_CFLAGS += -I$(DIR_ACMW)/../rtk_voip/include
endif

else

EXTRA_CFLAGS += $(ACLFLAGS)


O_TARGET := ac_drv.o
subdir-y :=


ifeq ($(CONFIG_AUDIOCODES_VOIP_RELEASE),y)
INCLUDEFLAGS= -I. -I$(ACLPRJINCDIR) \
		  -I../../rtk_voip/voip_drivers \
		  -I../../rtk_voip/include

ifeq ($(CONFIG_VOIP_SDK),y)
obj-y := Ntt_sRing_det.obj AC49xDrv_Api.obj AC49xDrv_UserDefinedFunctions.obj RTK_AC49xApi_Interface.obj 
else
obj-y := Ntt_sRing_det.o AC49xDrv_Api.o AC49xDrv_UserDefinedFunctions.o RTK_AC49xApi_Interface.o 
endif

else 

ifeq ($(CONFIG_AUDIOCODES_VOIP_SIP),y)
INCLUDEFLAGS= -I. -I$(ACLPRJINCDIR) \
				-I../../rtk_voip/voip_drivers \
				-I../../rtk_voip/include

ifeq ($(CONFIG_VOIP_SDK),y)
obj-y := Ntt_sRing_det.obj AC49xDrv_Api.obj AC49xDrv_UserDefinedFunctions.obj RTK_AC49xApi_Interface.obj 
else
obj-y := Ntt_sRing_det.o AC49xDrv_Api.o AC49xDrv_UserDefinedFunctions.o RTK_AC49xApi_Interface.o 
endif

else

INCLUDEFLAGS= -I. -I../ac_app/ac49xclient/include  \
				-I$(ACLPRJINCDIR) \
				-I../../rtk_voip/voip_drivers \
				-I../../rtk_voip/include

ifeq ($(CONFIG_VOIP_SDK),y)
obj-y := dsp_drv.obj RTL8186_UserDefinedFunctions.obj AC49xDrv_Api.obj AC49xDrv_UserDefinedFunctions.obj
else
obj-y := dsp_drv.o RTL8186_UserDefinedFunctions.o AC49xDrv_Api.o AC49xDrv_UserDefinedFunctions.o  RTK_AC49xApi_dummy.o
endif

endif

endif

EXTRA_CFLAGS += $(INCLUDEFLAGS)

ifneq ($(CONFIG_RTL8672),y)
include $(TOPDIR)/Rules.make
endif

endif

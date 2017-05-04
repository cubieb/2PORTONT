obj-y	:= Ve_profile.o 

# for Narrow band High voltage 88221/241/266/286
obj-y 	+= Le880NB_HV.o 

# for Narrow band High voltage 89136/89336
obj-y 	+= Le890NB_HV.o 
obj-y 	+= Le890NB_LVHV.o 
obj-$(CONFIG_BOARD_LE89156) += Le89156_100V_IB_Profile.o
obj-y 	+= Le890WB_HV.o
obj-y += ZLR96622L_A0.o
obj-y += ZLR966121L.o
obj-y += Le89156_100V_IB_Profile.o

ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_USE_ZSI)" "y"
obj-y += Realtek_88601_zsi.o
else
obj-y += Realtek_88601.o
endif


EXTRA_CFLAGS += -I$(DIR_VOIP)/include/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/microsemi//microsemi_voicepath_api/api_lib/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/microsemi/microsemi_voicepath_api/arch/rtl89xxb/

include $(DIR_VOIP)/rules.mak


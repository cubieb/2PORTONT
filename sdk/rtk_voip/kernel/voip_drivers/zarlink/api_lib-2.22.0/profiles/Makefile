obj-y	:= Ve_profile.o 

# for Narrow band High voltage 88221/241/266/286
ifeq "$(CONFIG_RTK_VOIP_SLIC_ZARLINK_880_SERIES)" "y"
obj-y 	+= Le880NB_HV.o 
endif

# for Narrow band High voltage 89136/89336
ifeq "$(CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES)" "y"
#obj-y 	+= Le890NB_HV.o 
obj-y 	+= Le890NB_LVHV.o 
ifeq "$(CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES_WIDEBAND)" "y"
obj-$(CONFIG_BOARD_LE89156) += Le89156_100V_IB_Profile.o
obj-y 	+= Le890WB_HV.o
endif
endif

ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9662)" "y"
obj-y += ZLR96622L_A0.o
endif
ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661)" "y"
obj-y += ZLR966121L.o
endif
ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9641)" "y"
obj-y += 9661bb.o
endif

ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601)" "y"
ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_ZSI)" "y"
obj-y += Realtek_88601_zsi.o
else
obj-y += Realtek_88601.o
endif
endif


EXTRA_CFLAGS += -I$(DIR_VOIP)/include/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/arch/rtl89xxb/

include $(DIR_VOIP)/rules.mak


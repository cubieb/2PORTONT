#Makefile       proslic_mlt_dcfeed.c     proslic_mlt_math.c     si3217x_mlt.c
#proslic_mlt.c  proslic_mlt_diag_madc.c  proslic_mlt_version.c  si3226x_mlt.c

obj-y := proslic_mlt_dcfeed.o proslic_mlt_math.o proslic_mlt.o proslic_mlt_diag_madc.o proslic_mlt_version.o 
#obj-y += proslic_tstin.o

obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226x) += si3226x_mlt.o

obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x) += si3217x_mlt.o 

EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/proslic_api/inc
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/proslic_api/config_inc
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/proslic_api/config_src
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/proslic_mlt_api-v2.4.3/inc

EXTRA_CFLAGS += -DREALTEK_PATCH_FOR_SILAB
ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x)" "y"
EXTRA_CFLAGS += -DSI3217X
endif
ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226)" "y"
EXTRA_CFLAGS += -DSI322X
endif
ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226x)" "y"
EXTRA_CFLAGS += -DSI3226X
endif

include $(DIR_VOIP)/rules.mak

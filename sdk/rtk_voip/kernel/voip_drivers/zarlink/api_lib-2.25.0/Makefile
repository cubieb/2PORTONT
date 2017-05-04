obj-y += common/
obj-y += $(ZARLINK_PROFILE)/
obj-y += arch/rtl89xxb/

ifeq "$(CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES)" "y"
obj-y += vp890_api/
ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_LINETEST)" "y"
obj-y += linetest/common/
obj-y += linetest/vp890_api/
obj-y += linetest/vp890_lt/
endif
endif

ifeq "$(CONFIG_RTK_VOIP_SLIC_ZARLINK_880_SERIES)" "y"
obj-y += vp880_api/
endif

ifeq "$(CONFIG_RTK_VOIP_SLIC_ZARLINK_886_SERIES)" "y"
obj-y += vp886_api/
ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_LINETEST)" "y"
obj-y += linetest/common/
obj-y += linetest/vp886_api/
obj-y += linetest/vp886_lt/
endif
endif

include $(DIR_VOIP)/rules.mak


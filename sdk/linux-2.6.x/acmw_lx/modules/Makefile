ifeq "$(CONFIG_DEFAULTS_KERNEL_2_6)" "y"

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8981B_),y)
ifeq ($(CONFIG_RTK_VOIP_DRIVERS_IP_PHONE),y)
obj-y += ac_modules.rtl8981B.aec.obj
else
obj-y += ac_modules.rtl8981B.obj
endif
endif

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8982B_),y)
obj-y += ac_modules.rtl8982B.obj
endif

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8984B_),y)
obj-y += ac_modules.rtl8984B.obj
endif

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8672),y)
obj-y += ac_modules.rtl8672_fake.obj
endif

endif

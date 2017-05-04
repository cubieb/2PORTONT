# Realtek Semiconductor Corp.
#
# Tony Wu (tonywu@realtek.com)
# Oct. 18, 2008
#

#
# RTL8672
#
core-y   += arch/rlx/bsp_rtl8686/
cflags-y += -Iinclude/asm-rlx -Iarch/rlx/bsp_rtl8686/ -Idrivers/net/rtl86900/sdk/include/ -Idrivers/net/rtl86900/sdk/system/include 
load-y   += 0x80000000

extra-y  += vmlinux.lds

obj-y := prom.o setup.o irq.o timer.o serial.o promlib.o fwupdate.o
obj-y += gpio.o led-generic.o pushbutton.o rtl_gpio.o rtl_usb_phy.o rtk_proc.o netdev_reset.o
obj-$(CONFIG_APOLLO_MP_TEST) += mptest.o
obj-$(CONFIG_E8B) += mptest.o
obj-$(CONFIG_LUNA_CMU) += luna_cmu.o
obj-$(CONFIG_LUNA_WDT) += luna_wdt.o
obj-$(CONFIG_LUNA_WDT_KTHREAD) += watchdog_timer.o
ifeq ($(CONFIG_PRODUCT),luna) 
obj-y += board.o 
else
obj-$(CONFIG_RTL8686_BOARD_001) += board-001.o
obj-$(CONFIG_RTL8686_BOARD_002) += board-002.o
obj-$(CONFIG_RTL8686_BOARD_003) += board-003.o
obj-$(CONFIG_RTL8686_BOARD_004) += board-004.o
obj-$(CONFIG_RTL8686_BOARD_005) += board-005.o
endif
obj-$(CONFIG_GPIO_LED_TR068) += led-tr068.o
obj-$(CONFIG_WPS_LED) += led-wps.o
obj-y += watchdog.o
#obj-$(CONFIG_PCI) += pci.o
obj-$(CONFIG_PCI) += pci-rtl86xx.o ops-rtl86xx.o
obj-$(CONFIG_RTL867X_NETLOG) += netlog.o
# 2010-11-19 paula
ifeq ($(CONFIG_RTL8672_SW_USB_LED), y)
obj-y    += led-usb.o
endif
obj-$(CONFIG_USE_PRELOADER_PARAMETERS) += kernel_soc_parameters.o
obj-$(CONFIG_MTD_LUNA_NOR_SPI) += kernel_soc_parameters.o

obj-$(CONFIG_RTK_CP3_PERF) += cp3perf.o

ifeq ($(CONFIG_LUNA_USE_SRAM), y)
obj-y    += sram_mapping.o
endif
ifeq ($(CONFIG_LUNA_CPU_OPTION_SWITCH), y)
obj-y    += luna_cpuoptions_sw.o
endif
obj-$(CONFIG_RGMII_RESET_PROCESS) 	+= rgmii_rst.o
obj-$(CONFIG_HIGH_RES_TIMERS) += luna-csrc.o

EXTRA_AFLAGS := $(CFLAGS)
ifeq ($(CONFIG_MTD_NOR_SPIF_G3), y)
	EXTRA_CFLAGS += -Idrivers/mtd/chips/luna_norsf_g3/nor_spi
endif
#EXTRA_CFLAGS += -I$(DIR_LINUX)/drivers/net/rtl819x/AsicDriver
#EXTRA_CFLAGS += -I$(DIR_LINUX)/drivers/net/rtl819x/common

#krammer
ARCH_VMLINUX  := arch/$(ARCH)/bsp_rtl8686/vmlinux.lds


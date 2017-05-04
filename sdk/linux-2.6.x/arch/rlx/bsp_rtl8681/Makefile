# Realtek Semiconductor Corp.
#
# Tony Wu (tonywu@realtek.com)
# Oct. 18, 2008
#

#
# RTL8681
#
core-y   += arch/rlx/bsp_rtl8681/
cflags-y += -Iinclude/asm-rlx -Iarch/rlx/bsp_rtl8681/
load-y   += 0x80000000

extra-y  += vmlinux.lds

obj-y := prom.o setup.o irq.o timer.o serial.o promlib.o
obj-y += gpio.o led-generic.o pushbutton.o rtl_gpio.o

obj-y += adsl71.lib
obj-$(CONFIG_EOC) += netadsl.o
obj-$(CONFIG_RTL8681_BOARD_001) += board-001.o
obj-$(CONFIG_GPIO_LED_TR068) += led-tr068.o
obj-$(CONFIG_WPS_LED) += led-wps.o
obj-y += watchdog.o
obj-$(CONFIG_PCI) += pci.o
obj-$(CONFIG_RTL867X_NETLOG) += netlog.o
# 2010-11-19 paula
ifeq ($(CONFIG_RTL8672_SW_USB_LED), y)
obj-y    += led-usb.o
endif

EXTRA_AFLAGS := $(CFLAGS)
#EXTRA_CFLAGS += -I$(DIR_LINUX)/drivers/net/rtl819x/AsicDriver
#EXTRA_CFLAGS += -I$(DIR_LINUX)/drivers/net/rtl819x/common
ARCH_VMLINUX  := arch/$(ARCH)/bsp_rtl8681/vmlinux.lds

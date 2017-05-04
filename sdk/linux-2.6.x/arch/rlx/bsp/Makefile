# Realtek Semiconductor Corp.
#
# Tony Wu (tonywu@realtek.com)
# Oct. 18, 2008
#

#
# RTL8672
#
core-y   += arch/rlx/bsp/
cflags-y += -Iinclude/asm-rlx -Iarch/rlx/bsp/
load-y   += 0x80000000

extra-y  += vmlinux.lds

obj-y := prom.o setup.o irq.o timer.o serial.o promlib.o
obj-y += gpio.o led-generic.o pushbutton.o rtl_gpio.o rtl_usb_phy.o
obj-$(CONFIG_DEV_xDSL) += adsl71.lib
obj-$(CONFIG_EOC) += netadsl.o
obj-$(CONFIG_BOARD_001) += board-001.o
obj-$(CONFIG_BOARD_002) += board-002.o
obj-$(CONFIG_BOARD_003) += board-003.o
obj-$(CONFIG_BOARD_003v6) += board-003v6.o
obj-$(CONFIG_BOARD_004) += board-004.o
obj-$(CONFIG_BOARD_005) += board-005.o
obj-$(CONFIG_BOARD_006) += board-006.o
obj-$(CONFIG_BOARD_007) += board-007.o
obj-$(CONFIG_BOARD_008) += board-008.o
obj-$(CONFIG_BOARD_009) += board-009.o
obj-$(CONFIG_BOARD_010) += board-010.o
obj-$(CONFIG_BOARD_011) += board-011.o
obj-$(CONFIG_BOARD_012) += board-012.o
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

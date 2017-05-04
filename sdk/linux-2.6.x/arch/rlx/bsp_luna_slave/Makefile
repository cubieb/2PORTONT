# Realtek Semiconductor Corp.
#
# Tony Wu (tonywu@realtek.com)
# Oct. 18, 2008
#

BSP_PATH := arch/$(ARCH)/bsp_luna_slave/
core-y   += $(BSP_PATH)
cflags-y += -Iinclude/asm-rlx -I$(BSP_PATH)
load-y   += 0x80000000

extra-y  += vmlinux.lds

obj-y := prom.o setup.o irq.o timer.o serial.o promlib.o
obj-y += gpio.o led-generic.o pushbutton.o #rtl_usb_phy.o #rtl_gpio.o
obj-y += watchdog.o
#obj-$(CONFIG_GPIO_LED_TR068) += led-tr068.o
# 2010-11-19 paula
#obj-y += kernel_soc_parameters.o

ifeq ($(CONFIG_PRODUCT),luna)
obj-y += board.o
else
obj-$(CONFIG_RTL8686_BOARD_SLAVE) += board-slave.o
endif
obj-$(CONFIG_RTL8696_ICCLOG) += icclog.o
EXTRA_AFLAGS := $(CFLAGS)

#krammer
ARCH_VMLINUX  := $(BSP_PATH)vmlinux.lds

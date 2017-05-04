#
# Makefile for the Tulip ethernet driver
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...


obj-y := ../common/built-in.o
subdir-y := ../common

obj-y += ../AsicDriver/built-in.o
subdir-y += ../AsicDriver

ifdef CONFIG_RTL_LAYERED_DRIVER
	ifdef CONFIG_RTL_LAYERED_DRIVER_L2
               obj-y += ../l2Driver/built-in.o
               subdir-y += ../l2Driver
	else
		ifdef CONFIG_RTL_STP
			obj-y += ../l2Driver/built-in.o
			subdir-y += ../l2Driver
		endif
	endif
else
	obj-y += ../l2Driver/built-in.o
	subdir-y += ../l2Driver
endif

ifdef CONFIG_RTL_LAYERED_DRIVER
	ifdef CONFIG_RTL_LAYERED_DRIVER_L3
		obj-y += ../l3Driver/built-in.o
		subdir-y += ../l3Driver
	else
		ifdef CONFIG_RTL_HARDWARE_MULTICAST
			obj-y += ../l3Driver/built-in.o
			subdir-y += ../l3Driver
		endif
	endif
else
	obj-y += ../l3Driver/built-in.o
	subdir-y += ../l3Driver
endif

ifdef CONFIG_RTL_LAYERED_DRIVER
#	ifdef CONFIG_RTL_8198
	ifdef CONFIG_RTL_LAYERED_DRIVER_L4
		obj-y += ../l4Driver/built-in.o
		subdir-y += ../l4Driver
	endif
#	endif
else
	obj-y += ../l4Driver/built-in.o
	subdir-y += ../l4Driver
endif

#ifdef CONFIG_RTL_IGMP_SNOOPING
#obj-y += ../igmpsnooping/built-in.o
#subdir-y += ../igmpsnooping
#endif

ifdef CONFIG_8198_PORT5_RGMII
	obj-y += ../RTL8370_RTL8367_API/built-in.o
	subdir-y += ../RTL8370_RTL8367_API
endif

#ifdef CONFIG_RTL_MULTI_ETH_WAN
#	obj-y += ../smux/built-in.o
#	subdir-y += ../smux
#endif

	obj-y += ../RTL867x_hwnat_API/built-in.o
        subdir-y += ../RTL867x_hwnat_API

ifdef CONFIG_RTL_HARDWARE_NAT
	obj-y += ../testModel/built-in.o
	subdir-y += ../testModel
endif

ifdef CONFIG_RTL_HARDWARE_NAT
ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
	obj-y += ../acl_control/built-in.o
        subdir-y += ../acl_control
endif
endif

ifdef CONFIG_RTL_8367B
	obj-y += ../rtl8367b/built-in.o
	subdir-y += ../rtl8367b
endif

EXTRA_CFLAGS += -O1 -DRTL_TBLDRV -D__linux__  -mno-memcpy -DRTL865X_OVER_KERNEL -DRTL865X_OVER_LINUX -Werror

EXTRA_AFLAGS += $(EXTRA_CFLAGS)


#all:pcsc
#CC = gcc
#INCLUDE = .
#CFLAGS = -g -Wall

#ifdef 

EXTRA_CFLAGS += -Idrivers/net/rtl86900/sdk/system/include/
EXTRA_CFLAGS += -Idrivers/net/rtl86900/sdk/src/app/diag_rtk/include/
EXTRA_CFLAGS += -Idrivers/net/rtl86900/sdk/include/
obj-$(CONFIG_RTL_SMARTCARD) = pcsc.o

pcsc-objs := pcsc_if.o pcsc_atr.o pcsc_io.o pcsc_util.o pcsc_proc.o pcsc_timer.o pcsc_fs.o

clean:
	rm pcsc_atr.o pcsc_if.o pcsc_io.o pcsc_util.o pcsc_proc.o pcsc_timer.o pcsc_fs.o

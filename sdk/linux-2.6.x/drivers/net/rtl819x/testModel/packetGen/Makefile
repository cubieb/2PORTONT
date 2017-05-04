#
# Makefile for the model code
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...

obj-y := \
		crc32.o \
		pktDump.o \
		pktForm.o \
		pktGen.o \
		pktParse.o \
		protoForm.o \
		utility.o
			
EXTRA_CFLAGS += -Wall -ggdb
EXTRA_CFLAGS += -DRTL865X_MODEL_KERNEL -DRTL865X_OVER_KERNEL -DRTL865X_OVER_LINUX
EXTRA_CFLAGS += -I./$(obj) -I./$(obj)/../../


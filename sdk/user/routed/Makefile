#for test only
#CC = mips-uclibc-gcc
#CFLAGS = -O2  -fomit-frame-pointer   -Dlinux -D__linux__ -Dunix -D__uClinux__ -DEMBED -fpic -fPIC -embedded-relocs -mno-abicalls -I/home/casey/uClinux-dist/lib/uClibc-0.9.15/include -I/home/casey/uClinux-dist/lib/libm -I/home/casey/uClinux-dist/lib/libcrypt_old -I/home/casey/uClinux-dist -fno-builtin -nostartfiles -I/home/casey/uClinux-dist/linux-2.4.x/include
##

OBJS=	af.o if.o input.o main.o output.o startup.o \
	tables.o timer.o trace.o inet.o

CFLAGS += -I$(ROOTDIR)/linux/include $(INCNET) -I. -DRIPVERSION=2

all: routed

routed: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS) -L../boa/src/LINUX $(LIBMIB) $(LIBVOIP)

romfs:
	$(ROMFSINST) /bin/routed

clean:
	rm -f *.elf *.gdb *.o routed

$(OBJS): defs.h trace.h interface.h table.h af.h pathnames.h version.h


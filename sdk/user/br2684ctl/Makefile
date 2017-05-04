
EXEC = mpoad
EXEC1 = mpoactl
OBJS = mpoad.o text2atm.o ans.o misc.o text2qos.o
OBJS1 = mpoactl.o
CFLAGS += $(INCATM)
#CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/include/
#CFLAGS += -I$(ROOTDIR)/linux-2.6.x/include/linux/


ifdef CONFIG_USE_RSDK_WRAPPER
INC_OPT = $(ROOTDIR)/include/rtk/options.h
else #CONFIG_USE_RSDK_WRAPPER
INC_OPT = ../../lib/uClibc/include/rtk/options.h
endif #CONFIG_USE_RSDK_WRAPPER

all: $(EXEC) $(EXEC1)

$(EXEC): $(OBJS) $(INC_OPT)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBATM) $(LDLIBS$(LDLIBS-$(@))) $(LIBMIB) $(LIBVOIP)

$(EXEC1): $(OBJS1)
	$(CC) $(LDFLAGS) -o $@ $(OBJS1) $(LIBATM) $(LDLIBS$(LDLIBS-$(@)))

romfs:
	$(ROMFSINST) /bin/$(EXEC)
	$(ROMFSINST) /bin/$(EXEC1)

clean:
	-rm -f $(EXEC) $(EXEC1) *.elf *.gdb *.o


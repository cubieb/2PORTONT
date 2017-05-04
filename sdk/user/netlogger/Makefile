
EXEC = netlogger 
OBJS = netlogger.o

all: $(EXEC)


ifdef CONFIG_USE_RSDK_WRAPPER
INC_OPT = $(ROOTDIR)/include/rtk/options.h
else #CONFIG_USE_RSDK_WRAPPER
INC_OPT = ../../lib/uClibc/include/rtk/options.h
endif #CONFIG_USE_RSDK_WRAPPER


$(EXEC): $(OBJS) $(INC_OPT)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS) -L../boa/src/LINUX $(LIBMIB)


romfs:
	$(ROMFSINST) netlogger /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.o


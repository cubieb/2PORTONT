
EXEC = loopback
OBJS = loopback.o

ifdef CONFIG_LUNA
#FIXME: -DDO_NOT_USE_LOG10 use to avoid link -lm when other apps use libmib.so
CFLAGS += -I$(ROOTDIR)/linux-2.6.x/drivers/net/rtl86900/sdk/system/linux \
			-I$(ROOTDIR)/linux-2.6.x/drivers/net/rtl86900/sdk/include \
			-I$(ROOTDIR)/linux-2.6.x/drivers/net/rtl86900/sdk/system/include \
			-I$(ROOTDIR)/linux-2.6.x/drivers/net/rtl86900/romeDriver
endif

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS) $(LIBMIB) $(LIBVOIP)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.o


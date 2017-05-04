
EXEC = sarctl
OBJS = sarctl.o

ifneq ($(CONFIG_DEFAULTS_KERNEL_3_18),y)
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/include
endif
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.o


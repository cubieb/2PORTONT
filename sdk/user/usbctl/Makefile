
EXEC = usbctl
OBJS = usbctl.o

all: $(EXEC)

CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/include

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.o


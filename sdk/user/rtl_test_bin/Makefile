
EXEC = testbin
OBJS = testbin.o

CFLAGS += -I$(ROOTDIR)/linux-2.6.x/include/

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS) -L../boa/src/LINUX $(LIBMIB)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.o


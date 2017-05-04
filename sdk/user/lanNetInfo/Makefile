
EXEC = lanNetInfo
OBJS = lanNetInfo.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS) -L../boa_e8/src/LINUX $(LIBMIB) $(LIBVOIP)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.o


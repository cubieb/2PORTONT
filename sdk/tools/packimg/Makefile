
PROGS = packbin kfspad genhead
CFLAGS = 

all: $(PROGS)

packbin: packbin.o
	$(CC) $(LDFLAGS) -o $@ packbin.o -lz $(LDLIBS$(LDLIBS-$(@)))

kfspad: kfspad.o
	$(CC) $(LDFLAGS) -o $@ kfspad.o -lz $(LDLIBS$(LDLIBS-$(@)))

genhead: genhead.o
	$(CC) $(LDFLAGS) -o $@ genhead.o -lz $(LDLIBS$(LDLIBS-$(@)))

clean:
	-rm -f $(PROGS) *.elf *.gdb *.o


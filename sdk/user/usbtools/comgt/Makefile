EXEC = comgt 
OBJS = comgt.o

CFLAGS += -Os
CFLAGS += $(EXTRA_CFLAGS)


all: $(EXEC)

$(EXEC): $(OBJS) 
	$(CC) $(OBJS) -o $@

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.o


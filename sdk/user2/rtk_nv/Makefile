
EXEC = nv
OBJS = main.o libnv.o rtk_crc32.o
CFLAGS += -I../include -fPIC
# LDFLAGS += -L./

all: $(EXEC) 

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.o

rcX:


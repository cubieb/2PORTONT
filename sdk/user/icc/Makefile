ifeq ($(CONFIG_ARCH_LUNA_SLAVE),y)
EXEC = icc_rcv
OBJS = icc_rcv.o
else
EXEC = icc
OBJS = icc_snd.o
endif

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)
	

clean:
	-rm -f $(EXEC) *.elf *.o


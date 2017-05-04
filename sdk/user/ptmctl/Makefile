
EXEC = ptmctl
OBJS = ptmctl.o

all: $(EXEC)

$(EXEC): $(OBJS)
ifndef CONFIG_USER_BOA_SRC_BOA
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)
else  #CONFIG_USER_BOA_SRC_BOA
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS) $(LIBMIB) $(LIBVOIP)
endif #CONFIG_USER_BOA_SRC_BOA

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.o


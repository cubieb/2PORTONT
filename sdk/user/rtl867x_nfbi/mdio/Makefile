#uc makefile
UC_ROOT=$(shell pwd)


MDIOCTRL_EXEC = mdioctrl
MDIOCTRL_OBJS = mdioctrl.o


DEPH = 
CFLAGS += -Wall


PROG_ALL = $(MDIOCTRL_EXEC)


all: $(PROG_ALL)

%.o: %.c $(DEPH)
	$(CC) $(CFLAGS) -c -o $@ $<


$(MDIOCTRL_EXEC): $(MDIOCTRL_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(MDIOCTRL_OBJS)


romfs:
	$(ROMFSINST) /bin/$(MDIOCTRL_EXEC)

	
clean:
	-rm -f $(MDIOCTRL_EXEC) *.elf *.o

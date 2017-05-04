# Makefile for sh

EXEC = wget_manage
OBJS = wget_manage.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

$(OBJS): wget_manage.h

clean:
	rm -f $(EXEC) *.elf *.gdb *.o

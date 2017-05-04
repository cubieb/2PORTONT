# Makefile for sh

EXEC = ftp_manage
OBJS = ftp_manage.o debug.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

#$(OBJS): ftp_manage.h
$(OBJS):

clean:
	rm -f $(EXEC) *.elf *.gdb *.o

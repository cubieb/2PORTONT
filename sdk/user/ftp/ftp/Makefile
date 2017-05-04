
EXEC = ftp
OBJS = cmds.o cmdtab.o domacro.o ftp.o glob.o main.o ruserpass.o

ifeq ($(CONFIG_USER_TR143),y)
CFLAGS+= -DCONFIG_USER_TR143
endif

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS) $(LIBMIB) $(LIBVOIP)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.gdb *.elf *.o


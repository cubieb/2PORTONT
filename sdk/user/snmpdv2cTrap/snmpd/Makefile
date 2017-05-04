EXEC = snmpd
OBJS = snmpd.o trap.o getip.o

ifdef CONFIG_USER_REMOTE_MANAGEMENT
OBJS += xgetcwd.o xmalloc.o
endif

CFLAGS += -I../snmplib -I../modules
CFLAGS += -DHAVE_CONFIG_H=1 -I.

ifdef CONFIG_RTL8686_SWITCH
LDFLAGS += -lm
endif

# Need a bigger stack than the default
FLTFLAGS += -s 8000

USELIBS = ../modules/libmodules.a ../snmplib/libsnmplib.a

all: $(EXEC)

ifdef CONFIG_USER_REMOTE_MANAGEMENT
all: rmping

rmping: rmping.o
	$(CC) $(LDFLAGS) -o $@ rmping.o
endif

test:

$(EXEC): $(OBJS) $(USELIBS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(USELIBS) $(LDLIBS) -lpthread $(LIBMIB) $(LIBVOIP)


romfs:
	$(ROMFSINST) /bin/$(EXEC)
ifdef CONFIG_USER_REMOTE_MANAGEMENT
	$(ROMFSINST) /bin/rmping
endif

clean:
	-rm -f $(EXEC) rmping *.gdb *.elf *.o

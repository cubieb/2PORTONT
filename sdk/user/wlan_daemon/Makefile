ifeq ($(CONFIG_RTL8192CD),y)
EXEC = 11N_UDPserver
OBJS = 11N_UDPserver.o
else
ifeq ($(CONFIG_USB_RTL8192SU_SOFTAP),y)
EXEC = 11N_UDPserver
OBJS = 11N_UDPserver.o
else
EXEC = UDPserver
OBJS = UDPserver.o
endif
endif

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)
ifeq ($(CONFIG_USB_RTL8192SU_SOFTAP),y)
	$(ROMFSINST) /etc/orf
	$(ROMFSINST) /etc/irf
else
ifeq ($(CONFIG_RTL8192CD),y)
	$(ROMFSINST) /etc/orf
	$(ROMFSINST) /etc/irf
endif
endif

clean:
	-rm -f $(EXEC) *.elf *.o


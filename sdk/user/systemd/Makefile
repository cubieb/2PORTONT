
EXEC = systemd
OBJS = main.o

RC_LEVEL_SYSTEMD=/etc/init.d/rc18

all: $(EXEC)

ifdef CONFIG_LUNA
ifdef CONFIG_RTK_RG_INIT
CFLAGS += -I$(ROOTDIR)/linux-2.6.x/drivers/net/rtl86900/sdk/system/linux \
											-I$(ROOTDIR)/linux-2.6.x/drivers/net/rtl86900/sdk/include \
											-I$(ROOTDIR)/linux-2.6.x/drivers/net/rtl86900/sdk/system/include \
											-I$(ROOTDIR)/linux-2.6.x/drivers/net/rtl86900/romeDriver
endif
endif
$(EXEC): $(OBJS)
ifndef CONFIG_USER_BOA_SRC_BOA
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)
else  #CONFIG_USER_BOA_SRC_BOA
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS) $(LIBMIB) $(LIBVOIP)
endif #CONFIG_USER_BOA_SRC_BOA

.PHONY: rcX
rcX:
	$(ROMFSINST) -a "systemd&" $(RC_LEVEL_SYSTEMD)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.o


#nfbi makefile
EXEC = nfbi
OBJS = nfbi_api.o boot_param.o nfbi_frame.o nfbi_main.o
DEPH = nfbi_api.h boot_param.h nfbi_frame.h
DEPH += ../../../linux-2.6.x/drivers/char/rtl867x_nfbi/rtl867x_nfbi.h

CFLAGS += -Wall
ifdef CONFIG_USER_NFBI_BOOT_ONLY
CFLAGS += -DNFBI_BOOT_ONLY
endif #CONFIG_USER_NFBI_BOOT_ONLY

ifdef CONFIG_RTL8685
NFBI_FW = fw_rtl8685
else
ifdef CONFIG_RTL8685S
NFBI_FW = fw_rtl8685S
else
NFBI_FW = fw
endif
endif

all: $(EXEC)

%.o: %.c $(DEPH)
	$(CC) $(CFLAGS) -c -o $@ $<

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

romfs:
	#create rtl_nfbi node
	#mknod ../../../romfs/dev/rtl_nfbi c 14 0
	$(ROMFSINST) /bin/$(EXEC)
	mkdir -p ../../../romfs/etc/fw
	$(ROMFSINST) $(NFBI_FW)/boot.bin /etc/fw/boot.bin
	$(ROMFSINST) $(NFBI_FW)/linux.bin /etc/fw/linux.bin

clean:
	-rm -f $(EXEC) *.elf *.o
